
#include "private.h"

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#if defined(USE_WDT)
#include "esp_task_wdt.h"
//#include "rtc_wdt.h"
#endif
#include "esp_err.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "esp_timer.h"

#include <driver/gpio.h>
#include <driver/rtc_io.h>

#include "adc.h"
#include "bmx.h"
#include "button.h"
#include "context.h"
#include "vfs_fat_sdspi.h"
#ifdef CONFIG_USE_FATFS
#include "vfs_fat_spiflash.h"
#endif
#ifdef CONFIG_USE_SPIFFS
#include "vfs_spiffs.h"
#endif
#ifdef CONFIG_USE_LITTLEFS
#include "vfs_littlefs.h"
#endif
#include "gps_data.h"
#include "gps_log_file.h"
#include "http_rest_server.h"
#include "logger_config.h"
#include "ota.h"
#ifdef CONFIG_OTA_USE_AUTO_UPDATE
#include "https_ota.h"
#endif
#include "ubx.h"
#include "logger_wifi.h"
#include "driver_vendor.h"

#include "dstat_screens.h"
#include "lcd.h"

// events
#include "adc_events.h"
#include "button_events.h"
#include "config_events.h"
#include "gps_log_events.h"
#include "logger_events.h"
#include "ubx_events.h"
#include "ui_events.h"

ESP_EVENT_DEFINE_BASE(LOGGER_EVENT);

#if defined(CONFIG_IDF_TARGET_ESP32S3)
#define PIN_POWER_ON 15  // LCD and battery Power Enable
#define PIN_LCD_BL 38    // BackLight enable pin (see Dimming.txt)
#endif

#define PIN_BAT 35

#define CALIBRATION_BAT_V 1.7      // voor proto 1
#define uS_TO_S_FACTOR  1000000UL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  21600UL     /* Time ESP32 will go to sleep (no for 6h, 4/day) */
#define WDT_TIMEOUT 60             // 60 seconds WDT, opgelet zoeken naar ssid time-out<dan 10s !!!

static const char *TAG = "main";

extern struct context_s m_context;
extern struct context_rtc_s m_context_rtc;
extern struct m_wifi_context wifi_context;

static struct logger_config_s * m_config = 0;
// app_mode
// 0 - before config, still init running
// 1 - config, wifi and http running
// 2 - run, gps running
static app_mode_t app_mode = APP_MODE_UNKNOWN;
static uint8_t config_initialized = 0;
#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 2)
static const char * const app_mode_str[] = { APP_MODE_LIST(STRINGIFY) };
#endif

static TaskHandle_t gps_task_handle = 0;
#if defined(USE_WDT)
static int wdt_task0, wdt_task1;
#endif

static struct display_s display;
static esp_timer_handle_t button_timer = 0;
static esp_timer_handle_t sd_timer = 0;
static cur_screens_t cur_screen = CUR_SCREEN_NONE;
static cur_screens_t next_screen = CUR_SCREEN_NONE;
#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 2)
static const char * const cur_screen_str[] = { CUR_SCREEN_LIST(STRINGIFY) };
#endif
static uint8_t app_mode_wifi_on = 0;
static uint8_t app_mode_gps_on = 0;
static uint8_t stat_screen_count = 0;
static uint32_t low_bat_countdown = 0;
static uint8_t record_done = 255;
static const char * const wakeup_reasons[] = {
    0, 0,
    "ESP_SLEEP_WAKEUP_EXT0",
    "ESP_SLEEP_WAKEUP_EXT1",
    "ESP_SLEEP_WAKEUP_TIMER",
    "ESP_SLEEP_WAKEUP_TOUCHPAD",
    "ESP_SLEEP_WAKEUP_ULP",
    "ESP_SLEEP_WAKEUP_OTHER",
};
#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 2)
static uint32_t seconds_before_sleep = 0;
#endif
static uint32_t last_flush_time = 0;
static uint16_t ubx_fail_count = 0;
static uint8_t ubx_restart_requested = 0;

#define L_FW_UPDATE_FIELDS 3
#define L_CFG_GROUP_FIELDS 4

 // 200ms before exec cb
#define BUTTON_CB_WAIT_BEFORE 210000
static bool button_down = false;
static int button_press_mode = -1;
static uint8_t button_clicks = 0;
static uint8_t gps_cfg_item = 0;
static uint8_t stat_screen_cfg_item = 0;
static uint8_t screen_cfg_item = 0;
static uint8_t cfg_screen = 0;
static uint8_t fw_update_screen = 0;
static uint8_t fw_cfg_item = 0;

// static char msgbbb[BUFSIZ*3];

static void low_to_sleep(uint64_t sleep_time) {
    ILOG(TAG, "[%s]", __func__);
    display_uninit(&display);
    esp_timer_stop(button_timer);
    button_deinit();
    deinit_adc();
    events_uninit();
    gpio_set_direction((gpio_num_t)13, (gpio_mode_t)GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)13, 1);  // flash in deepsleep, CS stays HIGH!!
    gpio_deep_sleep_hold_en();
    esp_sleep_enable_timer_wakeup(uS_TO_S_FACTOR * sleep_time);
    ILOG(TAG, "[%s] getup logger to sleep for every %d seconds.", __func__, (int)sleep_time);
    ILOG(TAG, "[%s] going to sleep now.", __func__);
    esp_deep_sleep(uS_TO_S_FACTOR * sleep_time);
}

static void update_bat(uint8_t verbose) {
#ifdef USE_CUSTOM_CALIBRATION_VAL
    m_context_rtc.RTC_voltage_bat = volt_read(m_context_rtc.RTC_calibration_bat);
    if (verbose)
        DLOG(TAG, "[%s] computed:%.02f, required_min:%.02f, calibration:%.02f\n", __FUNCTION__, m_context_rtc.RTC_voltage_bat, MINIMUM_VOLTAGE, m_context_rtc.RTC_calibration_bat);
#else
    m_context_rtc.RTC_voltage_bat = volt_read();
    if (verbose)
        DLOG(TAG, "[%s] computed:%.02f, required_min:%.02f\n", __FUNCTION__, m_context_rtc.RTC_voltage_bat, MINIMUM_VOLTAGE);
#endif
    if(m_context_rtc.RTC_voltage_bat < MINIMUM_VOLTAGE) {
        if(!low_bat_countdown) {
            low_bat_countdown = get_millis() + 60000;  // 60 seconds
        }

    }
    else if(low_bat_countdown) {
        low_bat_countdown = 0;
        m_context.low_bat_count = 0;
    }
}

/*
Method to print the reason by which ESP32 has been awaken from sleep
*/

static int wakeup_init() {
    int ret = 0;
    DMEAS_START();
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    // First screen update call from wakeup
    if(wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) 
    // screen_cb(&display);

    if (m_context_rtc.RTC_voltage_bat < MINIMUM_VOLTAGE) {
        lowbat:
        app_mode = APP_MODE_SLEEP;
        esp_sleep_enable_ext0_wakeup(WAKE_UP_GPIO, 0);
        low_to_sleep(TIME_TO_SLEEP);
        goto done;
    }

    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0:
            ILOG(TAG, "%s", wakeup_reasons[wakeup_reason]);
            gpio_set_direction((gpio_num_t)WAKE_UP_GPIO, GPIO_MODE_INPUT);
            gpio_set_pull_mode((gpio_num_t)WAKE_UP_GPIO, GPIO_PULLUP_ONLY);
            rtc_gpio_deinit(WAKE_UP_GPIO);  // was 39
            m_context.reed = 1;
            esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
            /* ret += Boot_screen();
            delay_ms(ret); */
            break;
        case ESP_SLEEP_WAKEUP_EXT1:
            ILOG(TAG, "%s", wakeup_reasons[wakeup_reason]);
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            ILOG(TAG, "%s", wakeup_reasons[wakeup_reason]);
            // screen_cb(&display);
            goto lowbat;
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            ILOG(TAG, "%s", wakeup_reasons[wakeup_reason]);
            break;
        case ESP_SLEEP_WAKEUP_ULP:
            ILOG(TAG, "%s", wakeup_reasons[wakeup_reason]);
            break;
        default:
            ILOG(TAG, "%s int: %d", wakeup_reasons[7], wakeup_reason);
            /* ret += Boot_screen();
            delay_ms(ret); */
            break;
    }
    done:
    DMEAS_END(TAG, "[%s] took %llu us", __FUNCTION__);
    return ret;
}

static void wait_for_ui_task() {
    ILOG(TAG, "[%s]", __func__);
    uint8_t shutdown_counter_max = 10;
    cancel_lcd_ui_delay();
    while(!get_offscreen_counter() && shutdown_counter_max) {
#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 2)
        ILOG(TAG, "[%s] %hhu times 200 ms left to lcd ui task paused, done %lu ms", __func__, shutdown_counter_max, seconds_before_sleep);
#endif
        delay_ms(200);
#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 2)
        seconds_before_sleep += 200;
#endif
        --shutdown_counter_max;
    }
    lcd_ui_task_pause(); // no more screen updates from task
#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 2)
    ILOG(TAG, "[%s] wait for lcd ui task done, total %lu ms", __func__, seconds_before_sleep);
#endif
}

static void do_restart() {
    ILOG(TAG, "[%s]", __func__);
    esp_restart();
}

static void go_to_sleep(uint64_t sleep_time) {
    ILOG(TAG, "[%s]", __func__);
    if (wifi_status() > 0) {
        wifi_uninit();
    }
    wait_for_ui_task();
    write_rtc(&m_context_rtc);
    if (esp_timer_is_active(sd_timer)) {
            esp_timer_stop(sd_timer);
            esp_timer_delete(sd_timer);
    }
    if(sdcard_is_mounted()) {
        sdcard_umount();
    }
    sdcard_uninit();
#ifdef CONFIG_USE_FATFS
    fatfs_uninit();
#endif
#ifdef CONFIG_USE_SPIFFS
    spiffs_uninit();
#endif
#ifdef CONFIG_USE_LITTLEFS
    littlefs_deinit();
#endif
    if(sleep_time > 0) {
        low_to_sleep(sleep_time);
    } else {
        do_restart(0);
    }
}

static int shut_down_gps(int no_sleep) {
    ILOG(TAG, "[%s]", __func__);
    struct gps_context_s *gps = &m_context.gps;
    struct ubx_config_s *ubx = gps->ublox_config;
    if ((!ubx || !ubx->uart_setup_ok) && no_sleep) 
        return 0;
    if(!ubx)
        goto end;
    int ret = 0;
    if(ubx->time_set)
            next_screen = CUR_SCREEN_SAVE_SESSION;
    if (ubx->ready) {
        ubx->signal_ok = false;
        gps->GPS_delay = 0;
    }
    if (ubx->time_set) {  // Only safe to RTC memory if new GPS data is available !!
        m_context_rtc.RTC_distance = gps->Ublox.total_distance / 1000000;
        m_context_rtc.RTC_alp = avail_fields[26].value.num();
        m_context_rtc.RTC_500m = avail_fields[9].value.num();
        m_context_rtc.RTC_1h = avail_fields[38].value.num();
        m_context_rtc.RTC_mile = avail_fields[22].value.num(); // null...
        m_context_rtc.RTC_max_2s = avail_fields[5].value.num();
        m_context_rtc.RTC_avg_10s = avail_fields[2].value.num();
        m_context_rtc.RTC_R1_10s = avail_fields[16].value.num();
        m_context_rtc.RTC_R2_10s = avail_fields[17].value.num();
        m_context_rtc.RTC_R3_10s = avail_fields[18].value.num();
        m_context_rtc.RTC_R4_10s = avail_fields[19].value.num();
        m_context_rtc.RTC_R5_10s = avail_fields[20].value.num();
        if (gps->files_opened) {
            if (m_context.config->gps.log_txt) {
                session_info(gps, &gps->Ublox);
                session_results_s(gps, &gps->S2, m_context_rtc.RTC_calibration_speed);
                session_results_s(gps, &gps->S10, m_context_rtc.RTC_calibration_speed);
                session_results_s(gps, &gps->S1800, m_context_rtc.RTC_calibration_speed);
                session_results_s(gps, &gps->S3600, m_context_rtc.RTC_calibration_speed);
                session_results_m(gps, &gps->M100, m_context_rtc.RTC_calibration_speed);
                session_results_m(gps, &gps->M500, m_context_rtc.RTC_calibration_speed);
                session_results_m(gps, &gps->M1852, m_context_rtc.RTC_calibration_speed);
                session_results_alfa(gps, &gps->A250, &gps->M250, m_context_rtc.RTC_calibration_speed);
                session_results_alfa(gps, &gps->A500, &gps->M500, m_context_rtc.RTC_calibration_speed);
            }
            ret += 100;
            // flush_files(&m_context);
            close_files(gps);
        }
        struct tm tms;
        getLocalTime(&tms, 0);
        m_context_rtc.RTC_year = ((tms.tm_year) + 1900);  // local time is corrected with timezone in close_files() !!
        m_context_rtc.RTC_month = ((tms.tm_mon) + 1);
        m_context_rtc.RTC_day = (tms.tm_mday);
        m_context_rtc.RTC_hour = (tms.tm_hour);
        m_context_rtc.RTC_min = (tms.tm_min);    
    }
    uint32_t timeout = get_millis() + 5000; // wait for 3 seconds
    if(gps_task_handle) {
        while(gps_task_handle!=0 && timeout > get_millis()){
            delay_ms(100);
        }
    }
    timeout = get_millis() + 1000;
    while(ubx->config_progress && timeout > get_millis()) {
        delay_ms(100);
    }
    ubx_off(ubx);
    ubx->time_set = 0;
#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 2 || defined(DEBUG))
    task_memory_info(__func__);
#endif
    end:
    if (!no_sleep) {
        go_to_sleep(3);  // got to sleep after 5 s, this to prevent booting when
        // GPIO39 is still low !
    }
    if(next_screen == CUR_SCREEN_SAVE_SESSION) {
        next_screen = CUR_SCREEN_NONE;
    }
    return ret;
}
/*
#define CONFIG_FREERTOS_ENABLE_BACKWARD_COMPATIBILITY;
static void esp_dump_per_task_heap_info(void) {
    heap_task_stat_t tstat = {};
    bool begin = true;
    printf("Task Heap Utilisation Stats:\n");
    printf("||\tTask\t\t|\tPeak DRAM\t|\tPeak IRAM\t|| \n");
    while (1) {
        size_t ret = heap_caps_get_per_task_stat(&tstat, begin);
        if (ret == 0) {
            printf("\n");
            break;
        }
        const char *task_name = tstat.task ? pcTaskGetTaskName(tstat.task) : "Pre-Scheduler allocs";
        if (!strcmp(task_name, "wifi") || !strcmp(task_name, "tiT") || !strcmp(task_name, "aws_iot_task")) {
            printf("||\t%-12s\t|\t%-5d\t\t|\t%-5d\t\t|| \n",
                   task_name, tstat.peak[0], tstat.peak[1]);
        }
        begin = false;
    }
} */


#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 1)
#define GPS_TIMER_STATS 1
#endif

#if defined(GPS_TIMER_STATS)
static uint32_t push_failed_count = 0, push_ok_count = 0, prev_push_failed_count = 0, prev_push_ok_count = 0;
static uint32_t local_nav_dop_count = 0, prev_local_nav_dop_count = 0;
static uint32_t prev_local_nav_pvt_count = 0;
static uint32_t prev_local_nav_sat_count = 0;
static uint32_t prev_millis = 0, prev_msg_count = 0, prev_err_count = 0;
static esp_timer_handle_t gps_periodic_timer = 0;

static void s(void* arg) {
    ubx_config_t *ubx = (ubx_config_t *)arg;
    uint32_t millis = get_millis();
    uint32_t period = millis - prev_millis;
    uint16_t period_err_count = ubx->ubx_msg.count_err-prev_err_count;
    uint16_t period_msg_count = ubx->ubx_msg.count_msg - prev_msg_count;
    uint16_t period_saved_count = period_msg_count - period_err_count;
    uint16_t period_push_failed_count = push_failed_count - prev_push_failed_count;
    uint16_t period_push_ok_count = push_ok_count - prev_push_ok_count;
    uint16_t period_push_count = period_push_failed_count + period_push_ok_count;
    uint16_t period_local_nav_dop_count = local_nav_dop_count - prev_local_nav_dop_count;
    uint16_t period_local_nav_pvt_count = ubx->ubx_msg.count_nav_pvt - prev_local_nav_pvt_count;
    uint16_t period_local_nav_sat_count = ubx->ubx_msg.count_nav_sat - prev_local_nav_sat_count;
    prev_millis = millis;
    prev_msg_count = ubx->ubx_msg.count_msg;
    prev_err_count = ubx->ubx_msg.count_err;
    prev_push_failed_count = push_failed_count;
    prev_push_ok_count = push_ok_count;
    prev_local_nav_dop_count = local_nav_dop_count;
    prev_local_nav_pvt_count = ubx->ubx_msg.count_nav_pvt;
    prev_local_nav_sat_count = ubx->ubx_msg.count_nav_sat;
    printf("\n[%s] * p:%"PRIu32"ms, r:%"PRIu8"Hz\n", __FUNCTION__, period, ubx->rtc_conf->output_rate);
    printf("[%s] >>>> period  msgcount: %6"PRIu16"  ok: %6"PRIu16" fail: %6"PRIu16" >>>>\n", __FUNCTION__, period_msg_count, period_saved_count, period_err_count);
    printf("[%s] >>>> total   msgcount: %6"PRIu32", ok: %6"PRIu32" fail: %6"PRIu32" >>>>\n", __FUNCTION__, ubx->ubx_msg.count_msg, ubx->ubx_msg.count_ok, ubx->ubx_msg.count_err);
    printf("[%s] >>>> period pushcount: %6"PRIu16", ok: %6"PRIu16" fail: %6"PRIu16" >>>>\n", __FUNCTION__, period_push_count, period_push_ok_count, period_push_failed_count);
    printf("[%s] >>>> total  pushcount: %6"PRIu32", ok: %6"PRIu32" fail: %6"PRIu32" >>>>\n\n", __FUNCTION__, push_ok_count+push_failed_count, push_ok_count, push_failed_count);
    printf("[%s] >>>> period nav_dop_count: %6"PRIu16" >>>>\n", __FUNCTION__, period_local_nav_dop_count);
    printf("[%s] >>>> total  nav_dop_count: %6"PRIu32" >>>>\n\n", __FUNCTION__, local_nav_dop_count);
    printf("[%s] >>>> period nav_pvt_count: %6"PRIu16" >>>>\n", __FUNCTION__, period_local_nav_pvt_count);
    printf("[%s] >>>> total  nav_pvt_count: %6"PRIu32" >>>>\n\n", __FUNCTION__, ubx->ubx_msg.count_nav_pvt);
    printf("[%s] >>>> period nav_sat_count: %6"PRIu16" >>>>\n", __FUNCTION__, period_local_nav_sat_count);
    printf("[%s] >>>> total  nav_sat_count: %6"PRIu32" >>>>\n\n", __FUNCTION__, ubx->ubx_msg.count_nav_sat);
}
#endif

#define MIN_numSV_FIRST_FIX 5      // before start logging, changed from 4 to 5 7.1/2023
#define MAX_Sacc_FIRST_FIX 2       // before start logging
#define MIN_numSV_GPS_SPEED_OK  4  // minimum number of satellites for calculating speed, otherwise
#define MAX_Sacc_GPS_SPEED_OK  1   // max Sacc value for calculating speed, otherwise 0
#define MAX_GPS_SPEED_OK  50       // max speed in m/s for calculating speed, otherwise 0

#define MIN_SPEED_START_LOGGING 2000  // was 2000 min speed in mm/s over 2 s before start logging naar SD 

#define TIME_DELAY_FIRST_FIX 10       // 10 navpvt messages before start logging

esp_err_t ubx_msg_do(ubx_msg_byte_ctx_t *mctx) {
    struct gps_context_s *gps = &m_context.gps;
    struct ubx_config_s *ubx = gps->ublox_config;
    struct gps_data_s *gps_data = &gps->Ublox;
    struct ubx_msg_s * ubxMessage = &ubx->ubx_msg;
    const struct nav_pvt_s * nav_pvt = &ubxMessage->navPvt;
    const uint32_t now = get_millis();
    esp_err_t ret = ESP_OK;
    switch(mctx->ubx_msg_type) {
            case MT_NAV_DOP:
                if (nav_pvt->iTOW > 0) {  // Logging after NAV_DOP, ublox sends first NAV_PVT, and then NAV_DOP.
                #if defined(GPS_TIMER_STATS)
                    local_nav_dop_count++;
                #endif
                    if ((nav_pvt->numSV >= MIN_numSV_FIRST_FIX) && ((nav_pvt->sAcc / 1000.0f) < MAX_Sacc_FIRST_FIX) && (nav_pvt->valid >= 7) && (ubx->signal_ok == false)) {
                        ubx->signal_ok = true;
                        ubx->first_fix = (now - ubx->ready_time) / 1000;
                        ESP_LOGI(TAG, "[%s] First GPS Fix after %"PRIu16" sec.", __FUNCTION__, ubx->first_fix);
                        if(!m_context_rtc.RTC_screen_auto_refresh) {
                            lcd_ui_task_resume_for_times(1, -1, -1, true);
                        }
                    }
                    if (ubx->signal_ok && gps->GPS_delay < UINT8_MAX) {
                        gps->GPS_delay++; // delay max is 255 ubx messages for now
                    }

                    ubx_set_time(ubx, m_context.config->timezone);
                    if (!gps->files_opened && ubx->signal_ok && (gps->GPS_delay > (TIME_DELAY_FIRST_FIX * ubx->rtc_conf->output_rate))) {  // vertraging Gps_time_set is nu 10 s!!
                        int32_t avg_speed = 0;
                        avg_speed = (avg_speed + nav_pvt->gSpeed * 19) / 20;  // FIR filter average speed of last 20 measurements in mm/s
                        //printf("[%s] GPS avg gSpeed: %"PRId32", start logging when min is %d\n", __FUNCTION__, avg_speed, MIN_SPEED_START_LOGGING);
                        if (avg_speed > 1000) { // 1 m/s == 3.6 km/h
                            if (ubx->time_set) {
                                m_context.Shut_down_Save_session = true;
                                gps->start_logging_millis = now;
                                open_files(gps);  // only start logging if GPS signal is OK
                            } else {
                                ESP_LOGE(TAG, "[%s] gps time not set!", __FUNCTION__);
                            }
                        }  //    Alleen speed>0 indien snelheid groter is dan 1m/s + sACC<1 + sat<5 + speed>35 m/s !!!
                    }
                    //printf("need to be for speed: %x %x %"PRIu32" %"PRIu32"\n", m_context.sdOK, ubx->time_set, ubxMessage->count_nav_pvt, ubxMessage->count_nav_pvt_prev);
                    if ((m_context.sdOK) && (ubx->time_set) && (ubxMessage->count_nav_pvt > 10) && (ubxMessage->count_nav_pvt != ubxMessage->count_nav_pvt_prev)) {
                        ubxMessage->count_nav_pvt_prev = ubxMessage->count_nav_pvt;
                        // float sAcc=nav_pvt->sAcc/1000;
                        gps->gps_speed = nav_pvt->gSpeed;  // hier alles naar mm/s !!
                        //printf("[%s] GPS nav_pvt gSpeed: %"PRId32"\n", __FUNCTION__, gps->gps_speed);
                        
                        // gps msg interval used by sd card logging and run detection and show trouble screen when no gps signal
                        gps->interval_gps_msg = nav_pvt->iTOW - gps->old_nav_pvt_itow;
                        gps->old_nav_pvt_itow = nav_pvt->iTOW;
        
                        if ((now - last_flush_time) > 60000) {
                            flush_files(gps);
                            last_flush_time = now;
                        }
                        // init run if speed is 0 or sAcc > 1 or speed > 35 m/s
                        if ((nav_pvt->numSV <= MIN_numSV_GPS_SPEED_OK) || ((nav_pvt->sAcc / 1000.0f) > MAX_Sacc_GPS_SPEED_OK) || (nav_pvt->gSpeed / 1000.0f > MAX_GPS_SPEED_OK)) {
                            //printf("[%s] GPS speed reset.\n", __FUNCTION__);
                            gps->gps_speed = 0;
                            gps_data->run_start_time = 0;
                        }
                        //saved_count++;
                        if (gps->gps_speed > 1000) { // log only when speed is above 1 m/s == 3.6 km/h
                            log_to_file(gps);  // here it is also printed to serial !!
                            if(!m_context_rtc.RTC_screen_auto_refresh && lcd_ui_task_is_paused()) {
                                lcd_ui_task_resume();
                            }
                        }
                        else if(!m_context_rtc.RTC_screen_auto_refresh) {
                            if(!lcd_ui_task_is_paused()) {
                                lcd_ui_task_pause();
                                goto showscr;
                            }
                            if(record_done < 240) {
                                showscr:
                                lcd_ui_task_resume_for_times(1, -1, -1, true);
                            }
                        }
                        ret = push_gps_data(gps, gps_data, nav_pvt->lat / 10000000.0f, nav_pvt->lon / 10000000.0f, gps->gps_speed);
                        if(ret){
                        #if defined(GPS_TIMER_STATS)
                            ++push_failed_count;
                        #endif
                        #if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 3)
                            ESP_LOGE(TAG, "[%s] push msg failed, timer: %lu, msg_count: %lu ...\n", __func__, now, ubxMessage->count_nav_pvt);
                        #endif
                            goto end;
                        }
                        #if defined(GPS_TIMER_STATS)
                        else
                            ++push_ok_count;
                        #endif
                        gps->run_count = new_run_detection(gps, nav_pvt->heading / 100000.0f, gps->S2.avg_s);
                        gps->alfa_window = alfa_indicator(gps, nav_pvt->heading / 100000.0f);
                        // new run detected, reset run distance
                        if (gps->run_count != gps->old_run_count) {
                            gps_data->run_distance = 0;
                            if (gps->gps_speed / 1000.0f > MAX_GPS_SPEED_OK){
                                gps_data->run_start_time = now;
                                gps->record = 0;
                                cancel_lcd_ui_delay();
                            }
                        }
                        gps->old_run_count = gps->run_count;
                        update_distance(gps, &gps->M100);
                        update_distance(gps, &gps->M250);
                        update_distance(gps, &gps->M500);
                        update_distance(gps, &gps->M1852);
                        update_speed(gps, &gps->S2);
                        update_speed(gps, &gps->s2);
                        update_speed(gps, &gps->S10);
                        update_speed(gps, &gps->s10);
                        update_speed(gps, &gps->S1800);
                        update_speed(gps, &gps->S3600);
                        update_alfa_speed(gps, &gps->A250, &gps->M250);
                        update_alfa_speed(gps, &gps->A500, &gps->M500);
                        update_alfa_speed(gps, &gps->a500, &gps->M500);
                    }
                }
                break;
            case MT_NAV_PVT:
                if (nav_pvt->iTOW > 0) {
                    if (ubx->time_set == 1) {
                        ubxMessage->count_nav_pvt++;
                    }
                }
                break;
            case MT_NAV_SAT:
                ubxMessage->count_nav_sat++;
                // ubxMessage->nav_sat.iTOW=ubxMessage->nav_sat.iTOW-18*1000; //to match 18s diff UTC nav pvt & GPS nav sat !!!
                push_gps_sat_info(&gps->Ublox_Sat, &ubxMessage->nav_sat);
                break;
            default:
                break;
        }
        end:
        return ret;
}

//uint32_t saved_count=0, prev_saved_count=0;
static void gpsTask(void *parameter) {
    uint32_t now = 0, loops = 0, mt = 0;
    ubx_config_t *ubx = m_context.gps.ublox_config;
    ubx_msg_byte_ctx_t mctx = UBX_MSG_BYTE_CTX_DEFAULT();
    mctx.msg_match_to_pos = false;
    mctx.msg_ready_handler = ubx_msg_checksum_handler;
    ubx_msg_t * ubxMessage = &ubx->ubx_msg;
    struct nav_pvt_s * nav_pvt = &ubxMessage->navPvt;
    uint8_t try_setup_times = 5;
    ILOG(TAG, "[%s]", __func__);
#if defined(GPS_TIMER_STATS)
    if(gps_periodic_timer)
        ESP_ERROR_CHECK(esp_timer_start_periodic(gps_periodic_timer, 1000000));
#endif
    while (app_mode == APP_MODE_GPS) {
#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 2 || defined(DEBUG))
        if (loops++ > 100) {
            task_memory_info(__func__);
            loops = 0;
        }
#endif
        now = get_millis();
        if (!ubx->ready || !ubxMessage->mon_ver.hwVersion[0]|| ubx_restart_requested) {
            mt = now - (ubx->ready ? ubx->ready_time : 5000);
            // ILOG(TAG, "[%s] Gps init ... (%lums)", __FUNCTION__, mt);
            if (mt > 10000) { // 5 seconds
                if(ubx->ready){
                    ubx_off(ubx);
                    delay_ms(100);
                    ubx_fail_count++;
                }
                if(app_mode == APP_MODE_GPS)
                    ubx_setup(ubx);
            }
            if(low_bat_countdown) {
                delay_ms(500);
                goto loop_tail;
            }
            if(ubx_fail_count>50) {
                if(!ubxMessage->mon_ver.hwVersion[0]) { // only when no hwVersion is received
                    m_context.request_restart = true;
                    ILOG(TAG, "[%s] Gps init failed, restart requested.", __FUNCTION__);
                }
                else
                    ubx_fail_count = 0;
            }
            ubx_restart_requested = 0;
            goto loop_tail;
        }
        esp_err_t ret = ubx_msg_handler(&mctx);
        if(!ret) { // only decoding if no Wifi connection}
            ubx_msg_do(&mctx);
            ubx_fail_count = 0;
        } else {
            if((ubx->ready && ubxMessage->mon_ver.hwVersion[0] && (ret == ESP_ERR_TIMEOUT && ubx_fail_count>3)) || ubx_fail_count>20) {
                if(ubx->ready) {
                    ubx->ready = false;
                    ubxMessage->mon_ver.hwVersion[0] = 0;
                }
                ubx_fail_count++;
                goto loop_tail;
            }
            ubx_fail_count+=5;
        }
        
    loop_tail:
        delay_ms(10);
    }
#if defined(GPS_TIMER_STATS)
    if(gps_periodic_timer)
        ESP_ERROR_CHECK(esp_timer_stop(gps_periodic_timer));
#endif
    gps_task_handle = 0;
    vTaskDelete(NULL);
}

static void button_timer_cb(void *arg) {
    ILOG(TAG, "[%s]", __func__);
    if(button_clicks == 1) {
        ILOG(TAG, "[%s] Button single click arrived, %s.", __func__, button_press_mode == 3 ? "lllong" : button_press_mode == 2 ? "llong" : button_press_mode == 1 ? "long" : "short");
        uint8_t flush_times = 1;
        int8_t fast_refr_time = -1;
        if(button_press_mode==3) { // long long long press
            if(next_screen == CUR_SCREEN_SETTINGS||next_screen == CUR_SCREEN_FW_UPDATE) next_screen = CUR_SCREEN_NONE;
            m_context.request_restart = true;
            goto done; // not requesting refresh here
        }
        else if(button_press_mode==2) { // long long press
            if (app_mode == APP_MODE_GPS && app_mode_wifi_on == 0) {
                app_mode = APP_MODE_WIFI;
                if(next_screen == CUR_SCREEN_SETTINGS) next_screen = CUR_SCREEN_NONE;
                // lcd_ui_request_full_refresh(0);
            } else if (app_mode == APP_MODE_WIFI && app_mode_wifi_on == 1) {
                // app_mode = APP_MODE_GPS;
                m_context.request_restart = true;
                goto done;
            }
        }
        else if (button_press_mode==1) { // long press
            if (next_screen == CUR_SCREEN_SETTINGS){
                ILOG(TAG, "[%s] settings new screen requested %d", __func__, 1);
                    if(cfg_screen >= L_CFG_GROUP_FIELDS-1)
                        cfg_screen = 0;
                    else
                        ++cfg_screen;
            } else if (cur_screen == CUR_SCREEN_FW_UPDATE){
                ILOG(TAG, "[%s] fw update choice saved %d", __func__, 1);
                if(fw_update_screen == 0) {
                    m_context.fw_update_is_allowed = 1;
                } else if(fw_update_screen == 1) {
                    m_context.fw_update_postponed = get_millis() + 86400000; // 24 hours
                } else {
                    m_context.fw_update_postponed = get_millis() + 3600000; // 1 hours
                }
            } else {
                m_context.request_shutdown = true;
                goto done; // not requesting refresh here
            }
        }
        else if (button_press_mode==0) { // just click
            if (cur_screen == CUR_SCREEN_FW_UPDATE){
                ILOG(TAG, "[%s] fw update next choice %d", __func__, 1);
                    if(fw_update_screen >= L_FW_UPDATE_FIELDS-1)
                        fw_update_screen = 0;
                    else
                        ++fw_update_screen;
                goto refresh;
            }
            else if(cur_screen == CUR_SCREEN_SETTINGS) {
                ILOG(TAG, "[%s] settings next requested %d", __func__, 1);
                if(cfg_screen == CFG_GROUP_GPS) {
                    if(++gps_cfg_item >= L_CONFIG_GPS_FIELDS)
                        gps_cfg_item = 0;
                }
                else if(cfg_screen == CFG_GROUP_STAT_SCREENS) {
                    if(++stat_screen_cfg_item >= L_CONFIG_STAT_FIELDS)
                        stat_screen_cfg_item = 0;
                }
                else if(cfg_screen == CFG_GROUP_SCREEN) {
                    if(++screen_cfg_item >= L_CONFIG_SCREEN_FIELDS)
                        screen_cfg_item = 0;
                }
                else if(cfg_screen == CFG_GROUP_FW) {
                    if(++fw_cfg_item >= 2)
                        fw_cfg_item = 0;
                }
                goto refresh;
            }
            if (app_mode == APP_MODE_GPS){
                if(cur_screen == CUR_SCREEN_GPS_SPEED) {
                    ILOG(TAG, "[%s] gps info next screen requested, cur: %hhu", __func__, m_context.config->screen.speed_field);
                    m_context.config->screen.speed_field++;
                    if (m_context.config->screen.speed_field > L_CONFIG_SPEED_FIELDS)
                        m_context.config->screen.speed_field = 1;
                    m_context.Field_choice = 1;
                }
                else if(cur_screen==CUR_SCREEN_GPS_STATS) {
                    if(++m_context.stat_screen_cur >= get_stat_screens_count()) m_context.stat_screen_cur = 0;
                    ILOG(TAG, "[%s] next screen requested, cur: %hhu", __func__, m_context.stat_screen_cur);
                    stat_screen_count = get_stat_screens_count();
                }
            }
        }
        refresh:
        if(!m_context_rtc.RTC_screen_auto_refresh && lcd_ui_task_is_paused()) {
            lcd_ui_task_resume_for_times(flush_times, fast_refr_time, -1, false); // one partial refresh
        }
    }
    else if(button_clicks==2) {
        ILOG(TAG, "[%s] Button double click arrived, %s", __func__, button_press_mode == 3 ? "lllong" : button_press_mode == 2 ? "llong" : button_press_mode == 1 ? "long" : "short");
        if (app_mode == APP_MODE_GPS) {
            if(next_screen == CUR_SCREEN_GPS_INFO || cur_screen == CUR_SCREEN_GPS_INFO) {
                ILOG(TAG, "[%s] setting screen requested", __func__);
                next_screen = CUR_SCREEN_SETTINGS;
            }
            else if(next_screen==CUR_SCREEN_NONE) {
                ILOG(TAG, "[%s] gps info screen requested", __func__);
                next_screen = CUR_SCREEN_GPS_INFO;
            }
            else if(next_screen==CUR_SCREEN_SETTINGS) {
                ILOG(TAG, "[%s] gps_stats screen requested", __func__);
                next_screen = CUR_SCREEN_GPS_STATS;
            }
            else {
                ILOG(TAG, "[%s] default screen requested", __func__);
                next_screen = CUR_SCREEN_NONE;
            }
        }
        else if(app_mode == APP_MODE_WIFI) {
            wifi_sta_conf_sync();
            if(wifi_context.s_ap_connection && wifi_context.s_sta_connection) {
                ILOG(TAG, "[%s] wifi ap mode requested", __func__);
                wifi_mode(0, 1); // wifi set station mode
            }
            else if(wifi_context.s_ap_connection && !wifi_context.s_sta_connection) {
                ILOG(TAG, "[%s] wifi sta mode requested", __func__);
                wifi_mode(1, 0); // wifi set station mode
            }
            else {
                ILOG(TAG, "[%s] wifi sta + ap mode requested", __func__);
                wifi_mode(1, 1); // wifi set ap mode
            }
        }
        if(!m_context_rtc.RTC_screen_auto_refresh){
            lcd_ui_task_resume_for_times(1, -1, -1, false);
        }
        else
            lcd_ui_request_full_refresh(0);
    }
    else {
        ILOG(TAG, "[%s] Button triple click arrived, %s", __func__, button_press_mode == 3 ? "lllong" : button_press_mode == 2 ? "llong" : button_press_mode == 1 ? "long" : "short");
        ubx_config_t *ubx = m_context.gps.ublox_config;
        if(!(app_mode == APP_MODE_GPS && next_screen == CUR_SCREEN_SETTINGS)) {
            ILOG(TAG, "[%s] screen rotation change requested", __func__);
            if(set_screen_cfg_item(m_config, 6, ubx->rtc_conf->hw_type)) {
                g_context_rtc_add_config(&m_context_rtc, m_context.config);
                display_set_rotation(m_context_rtc.RTC_screen_rotation);
            }
            goto refresh;
        }
        if (app_mode == APP_MODE_GPS) {
            if(next_screen==CUR_SCREEN_SETTINGS) {
                const ubx_hw_t hw_type = ubx->rtc_conf->hw_type;
                ILOG(TAG, "[%s] settings screen change requested", __func__);
                if(cfg_screen == CFG_GROUP_GPS) {
                    if(set_gps_cfg_item(m_config, gps_cfg_item, hw_type)) {
                        ILOG(TAG, "[%s] settings screen change requested", __func__);
                        g_context_ubx_add_config(&m_context, ubx);
                        g_context_rtc_add_config(&m_context_rtc, m_context.config);
                        ubx_restart_requested = 1;
                    }
                }
                else if(cfg_screen == CFG_GROUP_STAT_SCREENS) {
                    if(set_stat_screen_cfg_item(m_config, stat_screen_cfg_item, hw_type)) {
                        ILOG(TAG, "[%s] settings screen change requested", __func__);
                        g_context_add_config(&m_context, m_context.config);
                    }
                }
                else if(cfg_screen == CFG_GROUP_SCREEN) {
                    if(set_screen_cfg_item(m_config, screen_cfg_item, hw_type)) {
                        ILOG(TAG, "[%s] settings screen change requested", __func__);
                        g_context_rtc_add_config(&m_context_rtc, m_context.config);
                        display_set_rotation(m_context_rtc.RTC_screen_rotation);
                    }
                }
                else if(cfg_screen == CFG_GROUP_FW) {
                    if(set_fw_update_cfg_item(m_config, fw_cfg_item, hw_type)) {
                        ILOG(TAG, "[%s] settings fw change requested", __func__);
                        // g_context_add_config(&m_context, m_context.config);
                    }
                }
                if(!m_context_rtc.RTC_screen_auto_refresh){
                    lcd_ui_task_resume_for_times(1, -1, -1, false);
                }
                // lcd_ui_request_fast_refresh(0);
            }
        }

    }
    done:
    cancel_lcd_ui_delay();
    button_clicks = 0;
    button_press_mode = -1;
}

static void button_cb(int num, l_button_ev_t ev, uint64_t time) {
    uint32_t tm = time/1000;
    l_button_t *btn = 0;
    ILOG(TAG, "[%s] num: %d event: %s", __func__, ev, l_button_ev_list[ev]);
    //ESP_LOGI(TAG, "Button %d event: %d, time: %ld ms", num, ev, tm);
    switch (ev) {
    case L_BUTTON_UP:
        button_down = false;
        if(num==0) {
            esp_timer_start_once(button_timer, BUTTON_CB_WAIT_BEFORE);
#if defined(CONFIG_LOGGER_BUTTON_GPIO_1)
        } else if(num==1) {
            struct gps_context_s *gps = &m_context.gps;
            const struct ubx_config_s *ubx = gps->ublox_config;
            if(tm >= CONFIG_LOGGER_BUTTON_LONG_PRESS_TIME_MS) {
                if (ubx->ready && ubx->signal_ok) {
                    reset_time_stats(&gps->s10);
                    reset_time_stats(&gps->s2);
                    reset_alfa_stats(&gps->a500);
                }
            } else {
#ifdef CONFIG_DISPLAY_DRIVER_ST7789
                m_context.display_bl_level_set = m_context.display_bl_level_set >= 100 ? 20 : m_context.display_bl_level_set + 20;
#endif
                /* if (ubx->ready && ubx->signal_ok) {
                    m_context.gpio12_screen_cur++;
                    if (m_context.gpio12_screen_cur >= m_context.gpio12_screen_count)
                        m_context.gpio12_screen_cur = 0;
                    m_context.Field_choice2 = 1;
                } */
            }
#endif
        }
        break;
    case L_BUTTON_DOWN:
        if(esp_timer_is_active(button_timer)){
            ILOG(TAG,"[%s] cancel timer, num: %d", __FUNCTION__, num);
            esp_timer_stop(button_timer);
        }
        button_down = true;
        button_press_mode = 0;
        button_clicks++;
        break;
    case L_BUTTON_LONG_PRESS_START:
        button_press_mode = 1;
        if(!m_context_rtc.RTC_screen_auto_refresh){
            lcd_ui_task_resume_for_times(1, -1, -1, false);
        }
        break;
    case L_BUTTON_LONG_LONG_PRESS_START:
        if(num==0 && tm >= 9700) {
            button_press_mode = 3;
            ILOG(TAG, "[%s] Button num: %d lllong press detected, time: %lld, restart requested.", __func__, num, time);
            m_context.request_restart = true;
            break;
        }
        else{
            button_press_mode = 2;
            if(!m_context_rtc.RTC_screen_auto_refresh){
                lcd_ui_task_resume_for_times(1, -1, -1, false);
            }
        }
        break;
    case L_BUTTON_DOUBLE_CLICK:
        break;
    case L_BUTTON_TRIPLE_CLICK:
        break;
    default:
        break;
    }
}

#define LOW_BAT_TRIGGER 7
uint32_t screen_cb(void* arg) {
    const uint32_t lcd_count = get_lcd_ui_count();
    ILOG(TAG, "[%s] %ld app_mode: %s, cur_screen: %s, next_screen: %s, count_failed: %hu", __func__, lcd_count, app_mode_str[app_mode], cur_screen_str[cur_screen], cur_screen_str[next_screen], ubx_fail_count);
    struct display_s *dspl = &display;
    uint32_t delay=0;
    if(!dspl || !dspl->op) {
        goto end;
    }
    bool run_is_active = false;
    struct gps_context_s *gps = &m_context.gps;
    const struct ubx_config_s *ubx = gps->ublox_config;
    if(app_mode == APP_MODE_GPS) {
        run_is_active = (m_config && ubx && ubx->signal_ok && gps->gps_speed / 1000.0f >= m_context.config->screen.stat_speed);
        if (run_is_active && next_screen != CUR_SCREEN_NONE){
            next_screen = CUR_SCREEN_NONE;
        }
        stat_screen_count = m_context.stat_screen_count;
        if (stat_screen_count > get_stat_screens_count())
            stat_screen_count = get_stat_screens_count();
    }
    
    display_op_t *op = dspl->op;
    int32_t now, emillis, elapsed;
    if(button_down) {
        if(button_press_mode > 0 && btns[0].button_down) {
            const char *s = 
            (button_press_mode==3) ? "Reboot" : 
            (button_press_mode==2) ? "Mode change" : 
            ((next_screen == CUR_SCREEN_SETTINGS) ? "Next group" : next_screen == CUR_SCREEN_FW_UPDATE ? "Do update" : "Shut down");
            delay = op->update_screen(dspl, SCREEN_MODE_PUSH, (void*)(&((struct push_forwarder_s){ button_press_mode, s })));
            goto end;
        }
    }

    if(m_context.firmware_update_started>=1 && next_screen == CUR_SCREEN_FW_UPDATE) {
        logger_config_item_t item = {.name = "Confirm"};
        if(m_context.fw_update_is_allowed || m_context.firmware_update_started == 1) {
            item.name = "Updating...";
            item.desc = "Please wait";
            item.value = 1;
            item.pos = 0;
        }
        else if(fw_update_screen == 0) {
            item.desc = "Update";
            item.value = 0;
            item.pos = 0;
        } else if(fw_update_screen == 1){
            item.desc = "Postpone for 24h";
            item.value = 1;
            item.pos = 1;
        }
        else {
            item.desc = "Postpone for 1h";
            item.value = 2;
            item.pos = 2;
        }
        const v_settings_t s = { CFG_GROUP_GPS, "Firmware Update", &item };
        delay=op->update_screen(dspl, SCREEN_MODE_FW_UPDATE, (void*)&s);
        cur_screen = CUR_SCREEN_FW_UPDATE;
        goto end;
    }

    // if(low_bat_countdown) {
    //     if(low_bat_countdown - get_millis() < 10000) {
    //         m_context.low_bat_count = 10;
    //     }
    // }
#if defined(CONFIG_DISPLAY_DRIVER_ST7789)
            if(m_context.display_bl_level!=m_context.display_bl_level_set){
                m_context.display_bl_level_set = m_context.display_bl_level_set > 100 ? 100 : m_context.display_bl_level_set;
                m_context.display_bl_level = m_context.display_bl_level_set;
                driver_st7789_bl_set(m_context.display_bl_level);
                delay_ms(2);
            }
#endif  
    if(app_mode == APP_MODE_SLEEP){
        op->sleep_screen(dspl, 0);
        goto end;
    }
    if (app_mode == APP_MODE_SHUT_DOWN || app_mode == APP_MODE_RESTART || m_context.request_shutdown || m_context.request_restart) {
        delay = op->update_screen(dspl, SCREEN_MODE_SHUT_DOWN, 0);
        goto end;
    }
    if(lcd_count > 1){
        if(m_context.low_bat_count>=LOW_BAT_TRIGGER) {
            delay=op->update_screen(dspl, SCREEN_MODE_LOW_BAT, 0);
            goto end;
        }
        if(!m_context.sdOK) {
            delay=op->update_screen(dspl, SCREEN_MODE_SD_TROUBLE, 0);
            goto end;
        }
    }
    if (lcd_count < 2 || app_mode == APP_MODE_BOOT) {
        delay = op->update_screen(dspl, SCREEN_MODE_BOOT, 0);
        cur_screen = CUR_SCREEN_BOOT;
        goto end;
    }
    
    if (next_screen == CUR_SCREEN_SETTINGS) {
        logger_config_item_t item = {0};
        if(cfg_screen == CFG_GROUP_GPS) {
            get_gps_cfg_item(m_config, gps_cfg_item, &item);
            const v_settings_t s = { CFG_GROUP_GPS, "GPS", &item };
            delay = op->update_screen(dspl, SCREEN_MODE_SETTINGS, (void*)&s);
        } else if(cfg_screen == CFG_GROUP_STAT_SCREENS) {
            get_stat_screen_cfg_item(m_config, stat_screen_cfg_item, &item);
            const v_settings_t s = { CFG_GROUP_STAT_SCREENS, "Stat Screens", &item };
            delay = op->update_screen(dspl, SCREEN_MODE_SETTINGS, (void*)&s);
        } else if(cfg_screen == CFG_GROUP_SCREEN) {
            get_screen_cfg_item(m_config, screen_cfg_item, &item);
            const v_settings_t s = { CFG_GROUP_SCREEN, "Display", &item };
            delay = op->update_screen(dspl, SCREEN_MODE_SETTINGS, (void*)&s);
        } else if(cfg_screen == CFG_GROUP_FW) {
            get_fw_update_cfg_item(m_config, fw_cfg_item, &item);
            const v_settings_t s = { CFG_GROUP_FW, "FW Update", &item };
            delay = op->update_screen(dspl, SCREEN_MODE_SETTINGS, (void*)&s);
        }
        cur_screen = CUR_SCREEN_SETTINGS;
    }
    else if (app_mode == APP_MODE_WIFI) {
        int wifistatus = wifi_status();
        if(next_screen==CUR_SCREEN_SAVE_SESSION){
            delay=op->update_screen(dspl, SCREEN_MODE_SHUT_DOWN, 0);
            cur_screen = CUR_SCREEN_SAVE_SESSION;
        } else {
            if (wifistatus < 1) {
                delay=op->update_screen(dspl, SCREEN_MODE_WIFI_START, 0);
            } else if (wifistatus == 1) {
                delay=op->update_screen(dspl, SCREEN_MODE_WIFI_STATION, 0);
            } else {
                delay=op->update_screen(dspl, SCREEN_MODE_WIFI_AP, 0);
            }
            cur_screen = CUR_SCREEN_WIFI;
        }
    } else if (app_mode == APP_MODE_GPS) {
        if (ubx && ubx->time_set && (ubx->ubx_msg.navPvt.iTOW - gps->old_nav_pvt_itow) > (gps->time_out_gps_msg * 5) && next_screen == CUR_SCREEN_NONE) {
            gpstrblscr:
            delay=op->update_screen(dspl, SCREEN_MODE_GPS_TROUBLE, 0);  // gps signal lost !!!
            cur_screen = CUR_SCREEN_GPS_TROUBLE;
        } else if ((next_screen != CUR_SCREEN_GPS_STATS && (!ubx || !ubx->ready || (ubx->ready && !ubx->ubx_msg.mon_ver.hwVersion[0]))) || (!run_is_active && next_screen == CUR_SCREEN_GPS_INFO)) {
            // if(!ubx->ubx_msg.mon_ver.hwVersion[0]) goto bootscreen;
            delay=op->update_screen(dspl, SCREEN_MODE_GPS_INIT, 0);
            cur_screen = CUR_SCREEN_GPS_INFO;
        }
        else if (!run_is_active && (gps->S2.display_max_speed  > 1000 || next_screen == CUR_SCREEN_GPS_STATS)) {
            if (gps->record && record_done == 255) {
                if(gps->S2.display_max_speed > 10000) // when more than 32k/h show records
                    record_done=0;
                gps->record = 0;
            }
            if (gps->S10.record && record_done < 2) { // 10sec max record
                struct record_forwarder_s r = { &avail_fields[16], &avail_fields[17], record_done==0};
                delay=op->update_screen(dspl, SCREEN_MODE_RECORD, &r);
                ++record_done;
                goto end;
            }
            else if (gps->S2.record && record_done < 4) { // 2sec max record
                if(record_done<2) record_done = 2;
                struct record_forwarder_s r = { &avail_fields[47], &avail_fields[48] , record_done==2};
                delay=op->update_screen(dspl, SCREEN_MODE_RECORD, &r);
                ++record_done;
                goto end;
            }
            else if (gps->M250.record && record_done < 6) { // 500m max record
                if(record_done<4) record_done = 4;
                struct record_forwarder_s r = { &avail_fields[51], &avail_fields[52] , record_done==4};
                delay=op->update_screen(dspl, SCREEN_MODE_RECORD, &r);
                ++record_done;
                goto end;
            }
            else if (gps->M500.record && record_done < 8) { // 500m max record
                if(record_done<6) record_done = 6;
                struct record_forwarder_s r = { &avail_fields[53], &avail_fields[54] , record_done==6};
                delay=op->update_screen(dspl, SCREEN_MODE_RECORD, &r);
                ++record_done;
                goto end;
            }
            else if (gps->M1852.record && record_done < 10) { // 1852m max record
                if(record_done<8) record_done = 8;
                struct record_forwarder_s r = { &avail_fields[55], &avail_fields[56] , record_done==8};
                delay=op->update_screen(dspl, SCREEN_MODE_RECORD, &r);
                ++record_done;
                goto end;
            }
             else if (gps->A500.record && record_done < 12) { // 500m alfa max record
                if(record_done<10) record_done = 10;
                struct record_forwarder_s r = { &avail_fields[42], &avail_fields[43] , record_done==10};
                delay=op->update_screen(dspl, SCREEN_MODE_RECORD, &r);
                ++record_done;
                goto end;
            }
            else if(record_done < 240) {
                record_done = 240;
            }
            // lower than 5 km/h
            while(m_context.stat_screen[m_context.stat_screen_cur] == 0) {
                if(++m_context.stat_screen_cur >= get_stat_screens_count()) { // 16 fields used eq uint16_t
                    m_context.stat_screen_cur = 0;
                    break;
                }
            }
            delay=op->update_screen(dspl, m_context.stat_screen_cur+1, 0);
            cur_screen = CUR_SCREEN_GPS_STATS;
        } else {
            if(ubx && !ubx->ubx_msg.mon_ver.hwVersion[0] && ubx->ready) goto gpstrblscr;
            if (m_config && m_config->screen.speed_large_font == 2) {
                    delay=op->update_screen(dspl, SCREEN_MODE_SPEED_2, 0);
            } else {
                if(op)
                    delay=op->update_screen(dspl, SCREEN_MODE_SPEED_1, 0);
            }
            m_context.stat_screen_cur = 0;
            stat_screen_count = m_context.stat_screen_count;
            cur_screen = CUR_SCREEN_GPS_SPEED;
            record_done = 255;
            gps_cfg_item = 0;
        }
    }
    end:
#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 2 || defined(DEBUG))
    task_memory_info(__func__);
#endif
    return delay;
}

#if defined(USE_WDT)
#ifdef USE_OLD_WDT

// For RTOS, the watchdog has to be triggered

void feedTheDog_Task0() {
    TIMERG0.wdt_wprotect =
        TIMG_WDT_WKEY_VALUE;   // write enable
                               // TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
    TIMERG0.wdt_feed = 1;      // feed dog
    TIMERG0.wdt_wprotect = 0;  // write protect
}

void feedTheDog_Task1() {
    TIMERG1.wdt_wprotect = TIMG_WDT_WKEY_VALUE;  // write enable
    TIMERG1.wdt_feed = 1;                        // feed dog
    TIMERG1.wdt_wprotect = 0;                    // write protect
}
#else
static volatile bool run_wdt_loop;
static esp_task_wdt_user_handle_t func_a_twdt_user_hdl;
static esp_task_wdt_user_handle_t func_b_twdt_user_hdl;

static void feedTheDog_Task0(void) {
    esp_task_wdt_reset_user(func_a_twdt_user_hdl);
}

static void feedTheDog_Task1(void) {
    esp_task_wdt_reset_user(func_b_twdt_user_hdl);
}
#endif

static void wdt_user_task() {
    int32_t millis = get_millis();
    int wdt_task0_duration = millis - wdt_task0;
    int wdt_task1_duration = millis - wdt_task1;
    int task_timeout =
        (WDT_TIMEOUT - 1) * 1000;  // 1 second less then reboot timeout
    if ((wdt_task0_duration < task_timeout) &&
        (wdt_task1_duration < task_timeout)) {
        feedTheDog_Task0();
        feedTheDog_Task1();
    }
    if ((wdt_task0_duration > task_timeout) && (m_context.downloading_file)) {
        feedTheDog_Task0();
        wdt_task0 = millis;
        ESP_LOGW(TAG, "Extend watchdog_timeout due long download");
    }
    if ((wdt_task0_duration > task_timeout) && (!m_context.downloading_file))
        ESP_LOGW(TAG, "Watchdog task0 triggered");
    if (wdt_task1_duration > task_timeout)
        ESP_LOGW(TAG, "Watchdog task1 triggered");
}

#if !CONFIG_ESP_TASK_WDT_INIT
static void wdt_task(void *arg) {
    // Subscribe this task to TWDT, then check if it is subscribed
    esp_task_wdt_add(NULL);
    esp_task_wdt_status(NULL);

    // Subscribe func_a and func_b as users of the the TWDT
    esp_task_wdt_add_user("feedTheDog_Task0", &func_a_twdt_user_hdl);
    esp_task_wdt_add_user("feedTheDog_Task1", &func_b_twdt_user_hdl);

    ESP_LOGI(TAG, "Subscribed to TWDT");
    // int timeout = WDT_TIMEOUT;
    while (run_wdt_loop) {
        esp_task_wdt_reset();
        wdt_user_task();
        delay_ms(50);
    }

    // Unsubscribe this task, func_a, and func_b
    esp_task_wdt_delete_user(func_a_twdt_user_hdl);
    esp_task_wdt_delete_user(func_b_twdt_user_hdl);
    esp_task_wdt_delete(NULL);

    ESP_LOGI(TAG, "Unsubscribed from TWDT");

    // Notify main task of deletion
    xTaskNotifyGive((TaskHandle_t)arg);
    // vTaskDelete(NULL);
}
#endif

static void init_watchdog() {
#if !CONFIG_ESP_TASK_WDT_INIT
        esp_task_wdt_config_t twdt_config = {
            .timeout_ms = WDT_TIMEOUT * 1000,
            .idle_core_mask =
                (1 << portNUM_PROCESSORS) - 1,  // Bitmask of all cores
            .trigger_panic = false,
        };
    esp_task_wdt_init(&twdt_config);
    ESP_LOGI(TAG, "TWDT initialized");
    ESP_LOGI(TAG, "Create TWDT task");
    run_wdt_loop = true;
    xTaskCreatePinnedToCore(wdt_task, "wdt_task", 4 * 1024,
                            xTaskGetCurrentTaskHandle(), 10, NULL, 0);
#endif  // CONFIG_ESP_TASK_WDT_INIT
}

#endif  // USE_WDT

static void init_button() {
    ILOG(TAG, "[%s]", __func__);
    button_init();
    btns[0].cb = button_cb;
#if defined(CONFIG_LOGGER_BUTTON_GPIO_1)
    btns[1].cb = button_cb;
#endif
}

void wifi_sta_conf_sync() {
    ILOG(TAG, "[%s]", __func__);
    for(uint8_t i=0, j=5; i<j; ++i) {
        if(i>0 && !m_config->wifi_sta[i].ssid[0]) break;
        if (strcmp(wifi_context.stas[i].ssid, m_config->wifi_sta[i].ssid)) {
            strcpy(wifi_context.stas[i].ssid, m_config->wifi_sta[i].ssid);
            strcpy(wifi_context.stas[i].password, m_config->wifi_sta[i].password);
        }
    }
    wifi_context.offset = m_config->timezone;
}

void app_mode_wifi_handler(int verbose) {
    if (app_mode_wifi_on) return;
    ILOG(TAG, "[%s]", __func__);
    app_mode = APP_MODE_WIFI;
    app_mode_wifi_on = 1;
    shut_down_gps(1);
    if (!wifi_context.s_wifi_initialized) {
        ILOG(TAG, "[%s] first turn wifi on", __FUNCTION__);
        wifi_sta_conf_sync();
        wifi_init();
        wifi_mode(1, 1);
        ILOG(TAG, "[%s] wifi started.", __FUNCTION__);
    }
#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 2 || defined(DEBUG))
    task_memory_info(__func__);
#endif
}

void app_mode_gps_handler(int verbose) {
    if (gps_task_handle || app_mode_gps_on) return;
    DMEAS_START();
    app_mode = APP_MODE_GPS;
    if (wifi_context.s_wifi_initialized) {
        wifi_uninit();
        m_context.NTP_time_set = 0;
    }
    if(!config_initialized) {
        goto end;
    }
    app_mode_gps_on = 1;
    xTaskCreatePinnedToCore(gpsTask,   /* Task function. */
                "gpsTask", /* String with name of task. */
                CONFIG_GPS_LOG_STACK_SIZE,  /* Stack size in bytes. */
                NULL,      /* Parameter passed as input of the task */
                19,         /* Priority of the task. */
                &gps_task_handle, 1);      /* Task handle. */
    end:
#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 2 || defined(DEBUG))
    task_memory_info(__func__);
#endif
    DMEAS_END(TAG, "[%s] took %llu us", __FUNCTION__);
}

void task_app_mode_handler(int verbose) {
    switch (app_mode) {
        case APP_MODE_RESTART:
        case APP_MODE_SHUT_DOWN:
            app_mode_wifi_on = 0;
            app_mode_gps_on = 0;
            
            if (app_mode == APP_MODE_SHUT_DOWN) {
                shut_down_gps(0);  // save gps
            } else {
                shut_down_gps(1);  // save gps
                go_to_sleep(0);
            }
            break;
        case APP_MODE_WIFI:
            app_mode_gps_on = 0;
            // next_screen = CUR_SCREEN_NONE;
            app_mode_wifi_handler(verbose);
            break;
        case APP_MODE_GPS:
        case APP_MODE_BOOT:
            app_mode_wifi_on = 0;
            app_mode_gps_handler(verbose);
            break;
        default:
            break;
    };
}

#if defined(CONFIG_IDF_TARGET_ESP32S3)
void init_power() {
    gpio_set_direction((gpio_num_t)PIN_POWER_ON, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_POWER_ON, 1);
    // gpio_set_direction((gpio_num_t)PIN_LCD_BL, GPIO_MODE_OUTPUT);
    // gpio_set_level(PIN_LCD_BL, 1);
}
#endif    

static void all_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    uint8_t no_auto_refresh = m_config ? !m_context_rtc.RTC_screen_auto_refresh : 0;
    if(base == LOGGER_EVENT) {
        switch(id) {
            case LOGGER_EVENT_DATETIME_SET:
                ILOG(TAG, "[%s] %s", __FUNCTION__, logger_event_strings[id]);
                if(no_auto_refresh){
                    lcd_ui_task_resume_for_times(1, -1, -1, false);
                }
                break;
            case LOGGER_EVENT_SDCARD_MOUNTED:
                ILOG(TAG, "[%s] %s", __FUNCTION__, logger_event_strings[id]);
                //m_context.sdOK = true;
                //m_context.freeSpace = sdcard_space();
                break;
            case LOGGER_EVENT_SDCARD_MOUNT_FAILED:
                ILOG(TAG, "[%s] %s", __FUNCTION__, logger_event_strings[id]);
                m_context.sdOK = false;
                m_context.freeSpace = 0;
                if(no_auto_refresh){
                    lcd_ui_task_resume_for_times(1, -1, -1, false);
                }
                break;
            case LOGGER_EVENT_SDCARD_UNMOUNTED:
                ILOG(TAG, "[%s] %s", __FUNCTION__, logger_event_strings[id]);
                m_context.sdOK = false;
                m_context.freeSpace = 0;
                break;
            case LOGGER_EVENT_FAT_PARTITION_MOUNTED:
                ILOG(TAG, "[%s] %s", __FUNCTION__, logger_event_strings[id]);
                break;
            case LOGGER_EVENT_FAT_PARTITION_MOUNT_FAILED:
                ILOG(TAG, "[%s] %s", __FUNCTION__, logger_event_strings[id]);
                break;
            case LOGGER_EVENT_FAT_PARTITION_UNMOUNTED:
                break;
            case LOGGER_EVENT_OTA_AUTO_UPDATE_START:
                ILOG(TAG, "[%s] %s", __FUNCTION__, logger_event_strings[id]);
                next_screen = CUR_SCREEN_FW_UPDATE;
                if(no_auto_refresh){
                    lcd_ui_task_resume_for_times(1, -1, -1, false);
                }
                break;
            case LOGGER_EVENT_OTA_AUTO_UPDATE_FINISH:
                ILOG(TAG, "[%s] %s", __FUNCTION__, logger_event_strings[id]);
                if(!m_context.request_restart) {
                    next_screen = CUR_SCREEN_NONE;
                    if(no_auto_refresh){
                        lcd_ui_task_resume_for_times(1, -1, -1, false);
                    }
                }
                break;
            case LOGGER_EVENT_OTA_AUTO_UPDATE_FAILED:
                ILOG(TAG, "[%s] %s", __FUNCTION__, logger_event_strings[id]);
                if(no_auto_refresh){
                    lcd_ui_task_resume_for_times(1, -1, -1, false);
                }
                break;
            // case LOGGER_EVENT_SCREEN_UPDATE_BEGIN:
            //     ESP_LOGW(TAG, "--- LOGGER_EVENT_SCREEN_UPDATE task start ---");
            //     break;
            // case LOGGER_EVENT_SCREEN_UPDATE_END:
            //     ESP_LOGW(TAG, "--- LOGGER_EVENT_SCREEN_UPDATE task elapsed:%" PRId32 " ---", *(int32_t *)event_data);
            //     break;
            default:
                // ILOG(TAG, "[%s] %s:%" PRId32, __FUNCTION__, base, id);
                break;
        }
    }
    else if(base == UBX_EVENT) {
        switch(id) {
            case UBX_EVENT_DATETIME_SET:
                ILOG(TAG, "[%s] %s", __FUNCTION__, ubx_event_strings[id]);
                break;
            case UBX_EVENT_UART_INIT_DONE:
                ILOG(TAG, "[%s] %s", __FUNCTION__, ubx_event_strings[id]);
                if(no_auto_refresh && (cur_screen == CUR_SCREEN_GPS_INFO || cur_screen == CUR_SCREEN_NONE)){
                    lcd_ui_task_resume_for_times(1, -1, -1, false);
                }
                break;
            case UBX_EVENT_UART_INIT_FAIL:
                ILOG(TAG, "[%s] %s", __FUNCTION__, ubx_event_strings[id]);
                if(no_auto_refresh && (cur_screen == CUR_SCREEN_GPS_INFO || cur_screen == CUR_SCREEN_NONE)){
                    lcd_ui_task_resume_for_times(1, -1, -1, false);
                }
                break;
            case UBX_EVENT_UART_DEINIT_DONE:
                ILOG(TAG, "[%s] %s", __FUNCTION__, ubx_event_strings[id]);
                break;
            case UBX_EVENT_SETUP_DONE:
                ILOG(TAG, "[%s] %s", __FUNCTION__, ubx_event_strings[id]);
                if(no_auto_refresh){
                    lcd_ui_task_resume_for_times(1, -1, -1, false);
                }
                break;
            case UBX_EVENT_SETUP_FAIL:
                ILOG(TAG, "[%s] %s", __FUNCTION__, ubx_event_strings[id]);
                if(no_auto_refresh && (cur_screen == CUR_SCREEN_GPS_INFO || cur_screen == CUR_SCREEN_NONE)){
                    lcd_ui_task_resume_for_times(1, -1, -1, false);
                }
                break;
            default:
                // ILOG(TAG, "[%s] %s:%" PRId32, __FUNCTION__, base, id);
                break;
        }
    }
    else if(base==GPS_LOG_EVENT) {
        const char * c = 0;
        switch(id) {
            case GPS_LOG_EVENT_LOG_FILES_OPENED:
                ILOG(TAG, "[%s] %s", __FUNCTION__, gps_log_event_strings[id]);
                goto printfiles;
                break;
            case GPS_LOG_EVENT_LOG_FILES_OPEN_FAILED:
                ILOG(TAG, "[%s] %s", __FUNCTION__, gps_log_event_strings[id]);
                printfiles:
                for(uint8_t i=0; i<SD_FD_END; i++) {
                    printf("** [%s] ", i==SD_GPX ? "GPX" : i==SD_GPY ? "GPY" : i==SD_SBP ? "SBP" : i==SD_UBX ? "UBX" : i==SD_TXT ? "TXT" : "-");
                    if(GETBIT(m_context.gps.log_config->log_file_bits, i) && m_context.gps.log_config->filefds[i]<=0)
                    {
                        printf(" ERROR to");
                    }
                    if(c) {
                        printf(" open: %s ",  m_context.gps.log_config->filenames[i]);
                    }
                    printf("\n");
                }
                break;
            case GPS_LOG_EVENT_LOG_FILES_SAVED:
                ILOG(TAG, "[%s] %s", __FUNCTION__, gps_log_event_strings[id]);
                break;
            case GPS_LOG_EVENT_LOG_FILES_CLOSED:
                ILOG(TAG, "[%s] %s", __FUNCTION__, gps_log_event_strings[id]);
                break;
            default:
                // ILOG(TAG, "[%s] %s:%" PRId32, __FUNCTION__, base, id);
                break;
        }
    }
    else if(base == ADC_EVENT) {
        switch(id) {
            case ADC_EVENT_VOLTAGE_UPDATE:
                //ILOG(TAG, "[%s] ADC_EVENT_VOLTAGE_UPDATE", __FUNCTION__);
                break;
            case ADC_EVENT_BATTERY_LOW:
                ILOG(TAG, "[%s] %s", __FUNCTION__, adc_event_strings[id]);
                if(no_auto_refresh){
                    lcd_ui_task_resume_for_times(1, -1, -1, false);
                }
                break;
            case ADC_EVENT_BATTERY_OK:
                ILOG(TAG, "[%s] %s", __FUNCTION__, adc_event_strings[id]);
                break;
            default:
                // ILOG(TAG, "[%s] %s:%" PRId32, __FUNCTION__, base, id);
                break;
        }
    }
    else if(base == WIFI_EVENT) {
        switch(id) {
            case WIFI_EVENT_AP_START:
                ILOG(TAG, "[%s] %s", __FUNCTION__, wifi_event_strings[id]);
                if(no_auto_refresh){
                    lcd_ui_task_resume_for_times(1, -1, -1, false);
                }
                break;
            case WIFI_EVENT_AP_STOP:
                ILOG(TAG, "[%s] %s", __FUNCTION__, wifi_event_strings[id]);
                break;
            default:
                break;
        }
    }
    else if(base == IP_EVENT) {
        switch(id) {
            case IP_EVENT_STA_GOT_IP:
                ILOG(TAG, "[%s] IP_EVENT_STA_GOT_IP", __FUNCTION__);
                if(no_auto_refresh){
                    lcd_ui_task_resume_for_times(1, -1, -1, false);
                }
                break;
            case IP_EVENT_STA_LOST_IP:
                ILOG(TAG, "[%s] IP_EVENT_STA_LOST_IP", __FUNCTION__);
                if(no_auto_refresh){
                    lcd_ui_task_resume_for_times(1, -1, -1, false);
                }
                break;
            default:
                break;
        }
    }
    else if(base == UI_EVENT) {
        switch(id) {
            case UI_EVENT_FLUSH_START:
                ILOG(TAG, "[%s] %s", __FUNCTION__, ui_event_strings[id]);
                break;
            case UI_EVENT_FLUSH_DONE:
                ILOG(TAG, "[%s] %s", __FUNCTION__, ui_event_strings[id]);
#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 2 || defined(DEBUG))
                task_memory_info(__func__);
#endif
            default:
                break;
        }
    }
    else {
        // ILOG(TAG, "[%s] %s:%" PRId32, __FUNCTION__, base, id);
    }
}

/* static void wifi_event_handler(void *arg, esp_event_base_t base, int32_t id, void *event_data) {
    ESP_LOGI(TAG, "[%s] %s:%" PRId32, __FUNCTION__, base, id);
}

static void logger_event_handler(void *arg, esp_event_base_t base, int32_t id, void *event_data) {
    ESP_LOGI(TAG, "[%s] %s:%" PRId32, __FUNCTION__, base, id);
} */

static esp_err_t events_init() {
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_handler_instance_register(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, all_event_handler, NULL, NULL));
    // ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    // ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));
    // ESP_ERROR_CHECK(esp_event_handler_instance_register(LOGGER_EVENT, ESP_EVENT_ANY_ID, logger_event_handler, NULL, NULL));
    // ESP_ERROR_CHECK(esp_event_handler_instance_register(BUTTON_EVENT, ESP_EVENT_ANY_ID, logger_event_handler, NULL, NULL));
    // ESP_ERROR_CHECK(esp_event_handler_instance_register(UBX_EVENT, ESP_EVENT_ANY_ID, logger_event_handler, NULL, NULL));
    return ESP_OK;
}

static esp_err_t events_uninit() {
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, all_event_handler));
    // ESP_ERROR_CHECK(esp_event_handler_instance_unregister(LOGGER_EVENT, ESP_EVENT_ANY_ID, logger_event_handler));
    // ESP_ERROR_CHECK(esp_event_handler_instance_unregister(BUTTON_EVENT, ESP_EVENT_ANY_ID, logger_event_handler));
    // ESP_ERROR_CHECK(esp_event_handler_instance_unregister(UBX_EVENT, ESP_EVENT_ANY_ID, logger_event_handler));
    // ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler));
    // ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler));
    ESP_ERROR_CHECK(esp_event_loop_delete_default());
    return ESP_OK;
}

// observer like callback
static void config_changed_cb(const char *key) {
    if(strcmp(key, "screen_rotation")==0 || strcmp(key, "board_logo")==0||strcmp(key, "sail_logo")==0||strcmp(key, "speed_unit")==0 || strcmp(key, "sleep_info")==0) {
        g_context_rtc_add_config(&m_context_rtc, m_config);
    }
    if(strcmp(key, "screen_rotation")==0) {
        display_set_rotation(m_config->screen.screen_rotation);
        if(!m_context_rtc.RTC_screen_auto_refresh){
            lcd_ui_task_resume_for_times(1, -1, -1, false);
        }
    }
}

static void ctx_load_cb() {
    ILOG(TAG, "[%s]", __FUNCTION__);
    m_context.freeSpace = sdcard_space();
    ESP_LOGI(TAG, "[%s] sdcard mounted.", __FUNCTION__);
    m_config = config_new();
    m_config->config_changed_screen_cb = config_changed_cb;
    g_context_defaults(&m_context);
    m_context.config = m_config;
    if (!m_context.gps.Gps_fields_OK)
        init_gps_context_fields(&m_context.gps);
    delay_ms(50);

    config_load_json(m_config);
    if(m_config->screen.screen_rotation != m_context_rtc.RTC_screen_rotation) {
        ILOG(TAG, "[%s] screen rotation change (rtc) %d to (conf) %d", __FUNCTION__, m_context_rtc.RTC_screen_rotation, m_config->screen.screen_rotation);
    }
    g_context_rtc_add_config(&m_context_rtc, m_config);
    display_set_rotation(m_context_rtc.RTC_screen_rotation);
    g_context_add_config(&m_context, m_config);
    config_fix_values(m_config);
    g_context_ubx_add_config(&m_context, 0);
    log_config_add_config(m_context.gps.log_config, m_config);
    config_initialized = 1;
    if(!m_context_rtc.RTC_screen_auto_refresh){
       lcd_ui_task_resume_for_times(1, -1, -1, false);
    }
}

static void sd_mount_cb(void* arg) {
    if(!sdcard_is_mounted()) {
        if(sdcard_mount()==ESP_OK) {
            m_context.sdOK = true;
        }
        else if (!esp_timer_is_active(sd_timer)) {
            const esp_timer_create_args_t sd_timer_args = {
                .callback = &sd_mount_cb,
                .name = "sd_mount",
                .arg = 0
            };
            ESP_ERROR_CHECK(esp_timer_create(&sd_timer_args, &sd_timer));
            ESP_ERROR_CHECK(esp_timer_start_periodic(sd_timer, 1000000)); // 500ms
        }
    }
    else if (esp_timer_is_active(sd_timer)) {
            esp_timer_stop(sd_timer);
            esp_timer_delete(sd_timer);
    }
}

static void setup(void) {
    DMEAS_START();
    app_mode = APP_MODE_BOOT;
    int ret = 0;

#if defined(CONFIG_IDF_TARGET_ESP32S3)
    init_power();
#endif

    events_init();
    init_adc();
    delay_ms(50);
    update_bat(0);
    
#if defined(USE_WDT)
    init_watchdog();
#endif

    display_init(&display);
    display_set_rotation(m_context_rtc.RTC_screen_rotation);
    if(!m_context_rtc.RTC_screen_auto_refresh){
       lcd_ui_task_pause();
    }
    lcd_ui_start_task();
    delay_ms(50);
    // const esp_timer_create_args_t screen_periodic_timer_args = {
    //     .callback = &screen_cb,
    //     .name = "scr_tmr",
    //     .arg = &display
    // };
    // ESP_ERROR_CHECK(esp_timer_create(&screen_periodic_timer_args, &screen_periodic_timer));
    // ESP_LOGI(TAG, "[%s] start screen timer.", __FUNCTION__);
    // ESP_ERROR_CHECK(esp_timer_start_periodic(screen_periodic_timer, 1000000)); // 1000ms

    wakeup_init();  // Print the wakeup reason for ESP32, go back to sleep is timer is wake-up source !
     
    delay_ms(50);
    init_button();
    const esp_timer_create_args_t button_timer_args = {
        .callback = &button_timer_cb,
        .name = "btn_tmr",
        .arg = 0
    };
    ESP_ERROR_CHECK(esp_timer_create(&button_timer_args, &button_timer));
    delay_ms(50);

#ifdef CONFIG_USE_FATFS
    fatfs_init();
#endif
#ifdef CONFIG_USE_SPIFFS
    spiffs_init();
#endif
#ifdef CONFIG_USE_LITTLEFS
    littlefs_init();
#endif
    ESP_LOGI(TAG, "[%s] init sdcard", __FUNCTION__);
    if(!sdcard_init())
        sd_mount_cb(NULL);
    if(!m_context_rtc.RTC_screen_auto_refresh){
       lcd_ui_task_resume_for_times(3, 0, 1, true);
    }
    // appstage 1, structures initialized, start gps.
    delay_ms(50);
    ret += 50;

    http_rest_init(CONFIG_WEB_APP_PATH);

#if defined(CONFIG_BMX_ENABLE)
    init_bmx();
#endif

#if defined(DEBUG)
    ESP_LOGI(TAG, "[%s] verbosity mode module %d.", __FUNCTION__, CONFIG_LOGGER_COMMON_LOG_LEVEL);
#elif defined(NDEBUG)
    ESP_LOGI(TAG, "[%s] silent mode.", __FUNCTION__);
#else
    ESP_LOGW(TAG, "[%s] build debug mode not set.", __FUNCTION__);
#endif
    delay_ms(50);
#if defined(GPS_TIMER_STATS)
    const esp_timer_create_args_t gps_periodic_timer_args = {
        .callback = &gps_periodic_timer_callback,
        .name = "periodic",
        .arg = m_context.gps.ublox_config
    };
    ESP_ERROR_CHECK(esp_timer_create(&gps_periodic_timer_args, &gps_periodic_timer));
#endif

    DMEAS_END(TAG, "[%s] took %llu us", __FUNCTION__);
}

// static char rtbuf[BUFSIZ];
void app_main(void) {
    uint32_t loops = 0, millis = 0, bat_timeout = 0;
    // rtc_wdt_protect_off();
    setup();
    uint8_t verbose = 0;
    while (1) {
        if(bat_timeout) bat_timeout = 0;
        if(loops%10==0) { // ~1sec
            if(low_bat_countdown) {
                millis = get_millis();
                if(millis > low_bat_countdown) bat_timeout = 100;
                ESP_LOGW(TAG, "[%s] %lu low bat count:%d, seconds left: %lu", __FUNCTION__, loops, m_context.low_bat_count, (bat_timeout == 100 ? 0 : (low_bat_countdown-millis)));
                if(!bat_timeout && m_context.low_bat_count==LOW_BAT_TRIGGER) bat_timeout = 1;
                if(m_context.low_bat_count == LOW_BAT_TRIGGER+2) m_context.low_bat_count=0; // increase low bat count
                else m_context.low_bat_count++;
            }
            update_bat(0);
        }
        if (m_context.request_restart) {
            app_mode = APP_MODE_RESTART;
            m_context.request_restart = 1;
        }
        else if (m_context.request_shutdown || (low_bat_countdown && bat_timeout==100)) {
            app_mode = APP_MODE_SHUT_DOWN;
            m_context.request_shutdown = 1;
        }
        if(m_context.request_shutdown || m_context.request_restart || bat_timeout) {
            if(!m_context_rtc.RTC_screen_auto_refresh && lcd_ui_task_is_paused()) {
                lcd_ui_task_resume_for_times(1, -1, -1, false); // one partial refresh
            }
            m_context.request_shutdown = 0;
            m_context.request_restart = 0;
        }
        if(!m_config && m_context.sdOK) {
            ILOG(TAG, "[%s] config not loaded, do it as sdcard is initialized.", __FUNCTION__);
            ctx_load_cb();
        }
        if (loops++ >= 99) {
#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 2 || defined(DEBUG))
            memory_info_large(__func__);
            task_memory_info(__func__);
#endif
            loops=0;
            verbose = 1;
        } else {
            verbose = 0;
        }
        task_app_mode_handler(verbose);
        delay_ms(100);
    }
#if defined(USE_WDT)
    run_wdt_loop = false;
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
#if !CONFIG_ESP_TASK_WDT_INIT
    esp_task_wdt_deinit();
#endif
#endif
#if defined(CONFIG_BMX_ENABLE)
    deinit_bmx();
#endif
    sdcard_umount();
    sdcard_uninit();
#ifdef CONFIG_USE_FATFS
    fatfs_uninit();
#endif
#ifdef CONFIG_USE_SPIFFS
    spiffs_uninit();
#endif
#ifdef CONFIG_USE_LITTLEFS
    littlefs_deinit();
#endif
#if defined(GPS_TIMER_STATS)
    if(gps_periodic_timer)
        ESP_ERROR_CHECK(esp_timer_delete(gps_periodic_timer));
#endif
    // esp_timer_stop(screen_periodic_timer);
    esp_timer_stop(button_timer);
    display_uninit(&display);
    config_delete(m_config);
    ubx_config_delete(m_context.gps.ublox_config);
    events_uninit();
}
