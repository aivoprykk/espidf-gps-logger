
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
#include "display.h"
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
#include "ubx.h"
#include "wifi.h"
#include "driver_vendor.h"
#include "esp_wifi.h"

#include "dstat_screens.h"
#include "display.h"

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

#define EEPROM_SIZE 1              // use 1 unsigned char in eeprom for saving type of ublox

#ifndef MINIMUM_VOLTAGE
#define MINIMUM_VOLTAGE 3.2  // if lower then minimum_voltage, back to sleep.....
#endif

static const char *TAG = "main";

struct logger_config_s * m_config = 0;
extern struct context_s m_context;
extern struct context_rtc_s m_context_rtc;
extern struct m_wifi_context wifi_context;

// app_mode
// 0 - before config, still init running
// 1 - config, wifi and http running
// 2 - run, gps running
app_mode_t app_mode = APP_MODE_UNKNOWN;
uint8_t prev_stage = 0;

static TaskHandle_t t1;
static TaskHandle_t t2;
static TaskHandle_t t3;
int wdt_task0, wdt_task1;
float analog_mean = 2000;
unsigned char ota_notrunning = 1;
bool g_master_log_level = ESP_LOG_INFO;

struct display_s display;
static esp_err_t events_uninit();
esp_timer_handle_t button_timer = 0;
esp_timer_handle_t sd_timer = 0;
static int screen_cb_loop = 0;
static uint8_t displ_trigger_done = 0;
static cur_screens_t cur_screen = CUR_SCREEN_NONE;
static cur_screens_t next_screen = CUR_SCREEN_NONE;
uint8_t app_mode_wifi_on = 0;
uint8_t app_mode_gps_on = 0;
static bool button_down = false;
static int button_time = -1;
static uint8_t stat_screen_count = 0;

static SemaphoreHandle_t lcd_refreshing_sem = NULL;

static void low_to_sleep(uint64_t sleep_time) {
    ILOG(TAG, "[%s]", __func__);
    gpio_set_direction((gpio_num_t)13, (gpio_mode_t)GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)13, 1);  // flash in deepsleep, CS stays HIGH!!
    gpio_deep_sleep_hold_en();
    esp_sleep_enable_timer_wakeup(uS_TO_S_FACTOR * sleep_time);
    events_uninit();
    delay_ms(2000);
    volatile uint8_t shutdown_progress = 1;
    if(cur_screen != CUR_SCREEN_OFF_SCREEN && cur_screen != CUR_SCREEN_SLEEP_SCREEN)
        displ_trigger_done = 1;
    while(1){
        DLOG(TAG, "** [%s] shutdown_progress %hhu, displ_trigger_done: %hhu", __FUNCTION__, shutdown_progress, displ_trigger_done);
        if(_lvgl_lock(1000)){
            _lvgl_unlock();
        }
        if(!displ_trigger_done){
            break;
        }
        if(shutdown_progress++>2)
            break; 
        delay_ms(500);
    }
    deinit_adc();
    esp_timer_stop(button_timer);
    button_deinit();
    display_uninit(&display);
    ILOG(TAG, "[%s] getup logger to sleep for every %d seconds.", __func__, (int)sleep_time);
    //delay_ms(250);
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
}

/*
Method to print the reason by which ESP32 has been awaken from sleep
*/

static char * wakeup_reasons[] = {
    0, 0,
    "ESP_SLEEP_WAKEUP_EXT0",
    "ESP_SLEEP_WAKEUP_EXT1",
    "ESP_SLEEP_WAKEUP_TIMER",
    "ESP_SLEEP_WAKEUP_TOUCHPAD",
    "ESP_SLEEP_WAKEUP_ULP",
    "ESP_SLEEP_WAKEUP_OTHER",
};
static bool screen_from_wakeup = false;
static int wakeup_init() {
    int ret = 0;
    DMEAS_START();
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    // First screen update call from wakeup
    if(wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) 
    // screen_cb(&display);

    if (m_context_rtc.RTC_voltage_bat < MINIMUM_VOLTAGE) {
        lowbat:
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
            app_mode = APP_MODE_SLEEP;
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

static void go_to_restart() {
    ILOG(TAG, "[%s]", __func__);
        // m_context.request_restart = 0;
    delay_ms(2000);
    esp_restart();
}

static void go_to_sleep(uint64_t sleep_time) {
    ILOG(TAG, "[%s]", __func__);
    m_context.deep_sleep = true;
    if (wifi_status() > 0) {
        wifi_uninit();
        // esp_wifi_disconnect();
        // esp_wifi_stop();
    }
    write_rtc(&m_context_rtc);
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
    low_to_sleep(sleep_time);
}

static int shut_down_gps(int no_sleep) {
    if (!m_context.gps.ublox_config->ready && no_sleep) 
        return 0;
    ILOG(TAG, "[%s]", __func__); 
    int ret = 0;
    if(m_context.gps.ublox_config->time_set)
            next_screen = CUR_SCREEN_SAVE_SESSION;
    if (m_context.gps.ublox_config->ready) {
        m_context.gps.ublox_config->signal_ok = false;
        m_context.gps.GPS_delay = 0;
        //vTaskDelete(t1);
        uint32_t timeout = get_millis() + 3000; // wait for 3 seconds
        while(t1!=0 && timeout > get_millis()){
            delay_ms(100);
        }
    }
    if (m_context.gps.ublox_config->time_set) {  // Only safe to RTC memory if new GPS data is available !!
        m_context_rtc.RTC_distance = m_context.gps.Ublox.total_distance / 1000000;
        m_context_rtc.RTC_alp = m_context.gps.A500.display_max_speed * m_context_rtc.RTC_calibration_speed;
        m_context_rtc.RTC_500m = m_context.gps.M500.avg_speed[9] * m_context_rtc.RTC_calibration_speed;
        m_context_rtc.RTC_1h = m_context.gps.S3600.display_max_speed * m_context_rtc.RTC_calibration_speed;
        m_context_rtc.RTC_mile = m_context.gps.M1852.display_max_speed * m_context_rtc.RTC_calibration_speed;
        m_context_rtc.RTC_max_2s = m_context.gps.S2.avg_speed[9] * m_context_rtc.RTC_calibration_speed;
        m_context_rtc.RTC_avg_10s = m_context.gps.S10.avg_5runs * m_context_rtc.RTC_calibration_speed;
        m_context_rtc.RTC_R1_10s = m_context.gps.S10.avg_speed[9] * m_context_rtc.RTC_calibration_speed;
        m_context_rtc.RTC_R2_10s = m_context.gps.S10.avg_speed[8] * m_context_rtc.RTC_calibration_speed;
        m_context_rtc.RTC_R3_10s = m_context.gps.S10.avg_speed[7] * m_context_rtc.RTC_calibration_speed;
        m_context_rtc.RTC_R4_10s = m_context.gps.S10.avg_speed[6] * m_context_rtc.RTC_calibration_speed;
        m_context_rtc.RTC_R5_10s = m_context.gps.S10.avg_speed[5] * m_context_rtc.RTC_calibration_speed;
        if (m_context.gps.files_opened) {
            if (m_context.config->log_txt) {
                session_info(&m_context.gps, &m_context.gps.Ublox);
                session_results_s(&m_context.gps, &m_context.gps.S2, m_context_rtc.RTC_calibration_speed);
                session_results_s(&m_context.gps, &m_context.gps.S10, m_context_rtc.RTC_calibration_speed);
                session_results_s(&m_context.gps, &m_context.gps.S1800, m_context_rtc.RTC_calibration_speed);
                session_results_s(&m_context.gps, &m_context.gps.S3600, m_context_rtc.RTC_calibration_speed);
                session_results_m(&m_context.gps, &m_context.gps.M100, m_context_rtc.RTC_calibration_speed);
                session_results_m(&m_context.gps, &m_context.gps.M500, m_context_rtc.RTC_calibration_speed);
                session_results_m(&m_context.gps, &m_context.gps.M1852, m_context_rtc.RTC_calibration_speed);
                session_results_alfa(&m_context.gps, &m_context.gps.A250, &m_context.gps.M250, m_context_rtc.RTC_calibration_speed);
                session_results_alfa(&m_context.gps, &m_context.gps.A500, &m_context.gps.M500, m_context_rtc.RTC_calibration_speed);
            }
            ret += 100;
            // flush_files(&m_context);
            close_files(&m_context.gps);
        }
        struct tm tms;
        getLocalTime(&tms, 0);
        m_context_rtc.RTC_year = ((tms.tm_year) + 1900);  // local time is corrected with timezone in close_files() !!
        m_context_rtc.RTC_month = ((tms.tm_mon) + 1);
        m_context_rtc.RTC_day = (tms.tm_mday);
        m_context_rtc.RTC_hour = (tms.tm_hour);
        m_context_rtc.RTC_min = (tms.tm_min);
        m_context.gps.ublox_config->time_set = 0;
    }
    ubx_off(m_context.gps.ublox_config);
    task_memory_info("before_sleep");
    if (!no_sleep) {
        go_to_sleep(5);  // got to sleep after 5 s, this to prevent booting when
        // GPIO39 is still low !
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


#ifdef GPS_TIMER_STATS
uint32_t prev_millis = 0, prev_msg_count = 0, prev_err_count = 0;
esp_timer_handle_t gps_periodic_timer = 0;

static void gps_periodic_timer_callback(void* arg) {
    ubx_config_t *ubx = (ubx_config_t *)arg;
    uint32_t millis = get_millis();
    uint32_t period = millis - prev_millis;
    uint16_t period_err_count = ubx->ubx_msg.count_err-prev_err_count;
    uint16_t period_msg_count = ubx->ubx_msg.count_msg - prev_msg_count;
    uint16_t period_saved_count = period_msg_count - period_err_count;
    prev_millis = millis;
    prev_msg_count = ubx->ubx_msg.count_msg;
    prev_err_count = ubx->ubx_msg.count_err;
    printf("[%s] >>>> ts:%"PRId32" period:%"PRIu32", msgcount;%"PRIu16" failcount:%"PRIu16" passcount:%"PRIu16" >>>>\n", __FUNCTION__, millis, period, period_msg_count, period_err_count, period_saved_count);
    printf("[%s] >>>> ubx total msg count:%"PRIu32", failed: %"PRIu32" ok:%"PRIu32" >>>>\n", __FUNCTION__, ubx->ubx_msg.count_msg, ubx->ubx_msg.count_err, ubx->ubx_msg.count_ok);
}
#endif

#define MIN_numSV_FIRST_FIX 5      // before start logging, changed from 4 to 5 7.1/2023
#define MAX_Sacc_FIRST_FIX 2       // before start logging
#define MIN_numSV_GPS_SPEED_OK  4  // minimum number of satellites for calculating speed, otherwise
#define MAX_Sacc_GPS_SPEED_OK  1   // max Sacc value for calculating speed, otherwise 0
#define MAX_GPS_SPEED_OK  50       // max speed in m/s for calculating speed, otherwise 0

#define MIN_SPEED_START_LOGGING 2000  // was 2000 min speed in mm/s over 2 s before start logging naar SD 

#define TIME_DELAY_FIRST_FIX 10       // 10 navpvt messages before start logging

static uint32_t last_flush_time = 0;
esp_err_t ubx_msg_do(ubx_msg_byte_ctx_t *mctx) {
    ubx_config_t *ubx = m_context.gps.ublox_config;
    ubx_msg_t * ubxMessage = &ubx->ubx_msg;
    struct nav_pvt_s * nav_pvt = &ubxMessage->navPvt;
    gps_context_t *gps = &m_context.gps;
    uint32_t now = get_millis();
    switch(mctx->ubx_msg_type) {
            case MT_NAV_DOP:
                if (nav_pvt->iTOW > 0) {  // Logging after NAV_DOP, ublox sends first NAV_PVT, and then NAV_DOP.
                    if ((nav_pvt->numSV >= MIN_numSV_FIRST_FIX) && ((nav_pvt->sAcc / 1000.0f) < MAX_Sacc_FIRST_FIX) && (nav_pvt->valid >= 7) && (ubx->signal_ok == false)) {
                        ubx->signal_ok = true;
                        ubx->first_fix = (now - ubx->ready_time) / 1000;
                        ESP_LOGI(TAG, "[%s] First GPS Fix after %"PRIu16" sec.", __FUNCTION__, ubx->first_fix);
                    }
                    if (ubx->signal_ok && gps->GPS_delay < UINT8_MAX) {
                        gps->GPS_delay++; // delay max is 255 ubx messages for now
                    }

                    ubx_set_time(ubx, m_context.config->timezone);
                    if (!gps->files_opened && ubx->signal_ok && (gps->GPS_delay > (TIME_DELAY_FIRST_FIX * ubx->rtc_conf->output_rate))) {  // vertraging Gps_time_set is nu 10 s!!
                        int32_t avg_speed = 0;
                        avg_speed = (avg_speed + nav_pvt->gSpeed * 19) / 20;  // FIR filter gem. snelheid laatste 20 metingen in mm/s
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
                            gps->Ublox.run_start_time = 0;
                        }
                        //saved_count++;
                        if (gps->gps_speed > 1000) // log only when speed is above 1 m/s == 3.6 km/h
                            log_to_file(gps);  // here it is also printed to serial !!
                        push_gps_data(gps, &gps->Ublox, nav_pvt->lat / 10000000.0f, nav_pvt->lon / 10000000.0f, gps->gps_speed);
                        gps->run_count = new_run_detection(gps, nav_pvt->heading / 100000.0f, gps->S2.avg_s);
                        gps->alfa_window = alfa_indicator(gps, nav_pvt->heading / 100000.0f);
                        // new run detected, reset run distance
                        if (gps->run_count != gps->old_run_count) {
                            gps->Ublox.run_distance = 0;
                            if (gps->gps_speed / 1000.0f > MAX_GPS_SPEED_OK)
                                gps->Ublox.run_start_time = now;
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
                ubxMessage->nav_sat.iTOW=ubxMessage->nav_sat.iTOW-18*1000; //to match 18s diff UTC nav pvt & GPS nav sat !!!
                push_gps_sat_info(&m_context.gps.Ublox_Sat, &ubxMessage->nav_sat);
                break;
            default:
                break;
        }
        return ESP_OK;
}

uint8_t ubx_fail_count = 0;

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
    task_memory_info("gpsTask");
#ifdef GPS_TIMER_STATS
    if(gps_periodic_timer)
        ESP_ERROR_CHECK(esp_timer_start_periodic(gps_periodic_timer, 1000000));
#endif
    while (app_mode == APP_MODE_GPS) {
#ifdef DEBUG
        if (loops++ > 100) {
            task_memory_info("gpsTask");
            loops = 0;
        }
#endif
        now = get_millis();
        if (!m_context.gps.ublox_config->ready || !ubxMessage->mon_ver.hwVersion[0]) {
            mt = now - (ubx->ready ? ubx->ready_time : 5000);
            // ILOG(TAG, "[%s] Gps init ... (%lums)", __FUNCTION__, mt);
            if (mt > 10000) { // 5 seconds
                if(ubx->ready){
                    ubx_off(ubx);
                    delay_ms(100);
                    ubx_fail_count++;
                }
                ubx_setup(m_context.gps.ublox_config);
            }
            if(ubx_fail_count>50) {
                if(!ubxMessage->mon_ver.hwVersion[0]) // only when no hwVersion is received
                    app_mode = APP_MODE_WIFI;
                else
                    ubx_fail_count = 0;
            }
            goto loop_tail;
        }

        ubx_msg_handler(&mctx); // only decoding if no Wifi connection}
        ubx_msg_do(&mctx);
        
    loop_tail:
        delay_ms(10);
    }
#ifdef GPS_TIMER_STATS
    if(gps_periodic_timer)
        ESP_ERROR_CHECK(esp_timer_stop(gps_periodic_timer));
#endif
    vTaskDelete(NULL);
    t1 = 0;
    //ESP_LOGI(TAG, "[%s] ended", __FUNCTION__);
}

 // 200ms before exec cb
#define BUTTON_CB_WAIT_BEFORE 210000

static void button_timer_cb(void *arg) {
    ILOG(TAG, "[%s]", __func__);
    if(cur_screen == CUR_SCREEN_GPS_SPEED) {
        ESP_LOGI(TAG, "gps info next screen requested, cur: %hhu", m_context.config->speed_field);
        m_context.config->speed_field++;
        if (m_context.config->speed_field >= m_context.config->speed_field_count)
            m_context.config->speed_field = 0;
        m_context.Field_choice = 1;
    }
    else if(cur_screen==CUR_SCREEN_GPS_STATS) {
        ESP_LOGI(TAG, "next screen requested, cur: %hhu", m_context.stat_screen_cur);
        m_context.stat_screen_cur++;
        stat_screen_count = get_stat_screens_count();
    }
}

static void button_cb(int num, l_button_ev_t ev, uint64_t time) {
    uint32_t tm = time/1000;
    l_button_t *btn = 0;
    ILOG(TAG, "[%s]", __func__);
    //ESP_LOGI(TAG, "Button %d event: %d, time: %ld ms", num, ev, tm);
    switch (ev) {
    case L_BUTTON_UP:
        button_down = false;
        button_time = -1;
        if(num==0) {
            if (tm >= CONFIG_BTN_GPIO_INPUT_LONG_LONG_PRESS_TIME_MS) {
                if (app_mode == APP_MODE_GPS && app_mode_wifi_on == 0) {
                    app_mode = APP_MODE_WIFI;
                } else if (app_mode == APP_MODE_WIFI && app_mode_wifi_on == 1) {
                    // app_mode = APP_MODE_GPS;
                    m_context.request_restart = true;
                }
            }
            else if (tm >= CONFIG_BTN_GPIO_INPUT_LONG_PRESS_TIME_MS) {
                app_mode = APP_MODE_SHUT_DOWN;
            }
            else {
                esp_timer_start_once(button_timer, BUTTON_CB_WAIT_BEFORE);
            }
#ifdef CONFIG_BTN_GPIO_INPUT_1_ACTIVE
        } else if(num==1) {
            if(tm >= CONFIG_BTN_GPIO_INPUT_LONG_PRESS_TIME_MS) {
                if (m_context.gps.ublox_config->ready && m_context.gps.ublox_config->signal_ok) {
                    reset_time_stats(&m_context.gps.s10);
                    reset_time_stats(&m_context.gps.s2);
                    reset_alfa_stats(&m_context.gps.a500);
                }
            } else {
#ifdef CONFIG_DISPLAY_DRIVER_ST7789
                m_context.display_bl_level_set = m_context.display_bl_level_set >= 100 ? 20 : m_context.display_bl_level_set + 20;
#endif
                /* if (m_context.gps.ublox_config->ready && m_context.gps.ublox_config->signal_ok) {
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
            ESP_LOGI(TAG,"cancel timer 2");
            esp_timer_stop(button_timer);
        }
        button_down = true;
        button_time = 0;
        break;
    case L_BUTTON_LONG_PRESS_START:
        button_time = 1;
        break;
    case L_BUTTON_LONG_LONG_PRESS_START:
        if(num==0 && tm >= 9700) {
            ESP_LOGI(TAG, "Button %d 3xlong press detected, time: %lld, restart requested.", num, time);
            app_mode = APP_MODE_RESTART;
        }
        else
            button_time = 2;
        break;
    case L_BUTTON_DOUBLE_CLICK:
        ESP_LOGI(TAG, "Button %d double click requested.", num);
        if(esp_timer_is_active(button_timer)){
            ESP_LOGI(TAG,"cancel timer 3");
            esp_timer_stop(button_timer);
            if (app_mode == APP_MODE_GPS) {
                if(next_screen==CUR_SCREEN_NONE) {
                    next_screen = CUR_SCREEN_GPS_INFO;
                }
                else if(next_screen==CUR_SCREEN_GPS_INFO) {
                    next_screen = CUR_SCREEN_GPS_STATS;
                }
                else
                    next_screen = CUR_SCREEN_NONE;
            }
        }
        break;
    default:
        break;
    }
}

uint32_t screen_cb(void* arg) {
    DLOG(TAG, "[%s]", __func__);
    if(xSemaphoreTake(lcd_refreshing_sem, 0) != pdTRUE)
        return 0;
    struct display_s *dspl = &display;
    uint32_t delay=0;
    if(!dspl || !dspl->op) {
        goto end;
    }
    cur_screen = CUR_SCREEN_NONE;
    stat_screen_count = m_context.stat_screen_count;
    if (stat_screen_count > get_stat_screens_count())
        stat_screen_count = get_stat_screens_count();
    display_op_t *op = dspl->op;
    int32_t now, emillis, elapsed;
    if(button_down) {
        if(button_time > 0 && btns[0].button_down) {
            delay = op->update_screen(dspl, SCREEN_MODE_PUSH, (void*)button_time);
            goto end;
        }
    }

    update_bat(0);

    if(m_context_rtc.RTC_voltage_bat < MINIMUM_VOLTAGE && app_mode == APP_MODE_BOOT)
        m_context.low_bat_count = 10;
    if (m_context_rtc.RTC_voltage_bat < MINIMUM_VOLTAGE) {
        m_context.low_bat_count++;
    } else {
        m_context.low_bat_count = 0;
    }
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
        cur_screen = CUR_SCREEN_SLEEP_SCREEN;
    }
    if (app_mode == APP_MODE_SHUT_DOWN || app_mode == APP_MODE_RESTART) {
        op->off_screen(dspl, m_context_rtc.RTC_OFF_screen);
        cur_screen = CUR_SCREEN_OFF_SCREEN;
        delay=1000;
    } else if (m_context.boot_screen_stage || app_mode <= APP_MODE_BOOT) {
        op->boot_screen(dspl);
        m_context.boot_screen_stage = m_context.boot_screen_stage > 4 ? 0 : m_context.boot_screen_stage + 1;
    } else if (m_context.low_bat_count > 6) { // 6 * 10sec = 1 min
        op->off_screen(dspl, m_context_rtc.RTC_OFF_screen);
        cur_screen = CUR_SCREEN_OFF_SCREEN;
        delay=1000;
        app_mode = APP_MODE_SHUT_DOWN;
    } else if (app_mode == APP_MODE_WIFI) {
        delay=1000;
        int wifistatus = wifi_status();
        if(!m_context.sdOK) {
            op->update_screen(dspl, SCREEN_MODE_SD_TROUBLE, 0);
        }
        else if(next_screen==CUR_SCREEN_SAVE_SESSION){
            op->off_screen(dspl, m_context_rtc.RTC_OFF_screen);
            cur_screen = CUR_SCREEN_SAVE_SESSION;
        }
        else if (wifistatus < 1) {
            op->update_screen(dspl, SCREEN_MODE_WIFI_START, 0);
        } else if (wifistatus == 1) {
            op->update_screen(dspl, SCREEN_MODE_WIFI_STATION, 0);
        } else {
            op->update_screen(dspl, SCREEN_MODE_WIFI_AP, 0);
        }
    } else if (app_mode == APP_MODE_GPS) {
        bool run_is_active = (m_context.gps.gps_speed / 1000.0f >= m_context.config->stat_speed);
        if (run_is_active && next_screen != CUR_SCREEN_NONE)
            next_screen = CUR_SCREEN_NONE;

        if(!m_context.sdOK && next_screen == CUR_SCREEN_NONE) {
            // sd card trouble!!!
            op->update_screen(dspl, SCREEN_MODE_SD_TROUBLE, 0);
        }
        else if (m_context.gps.ublox_config->signal_ok && (m_context.gps.ublox_config->ubx_msg.navPvt.iTOW - m_context.gps.old_nav_pvt_itow) > (m_context.gps.time_out_gps_msg * 5) && next_screen == CUR_SCREEN_NONE) {
            // gps signal lost!!!
            op->update_screen(dspl, SCREEN_MODE_GPS_TROUBLE, 0);  // gps signal lost !!!
            cur_screen = CUR_SCREEN_GPS_TROUBLE;
        } else if (!m_context.gps.ublox_config->ready || (!run_is_active  && next_screen == CUR_SCREEN_GPS_INFO)) {
            // gps not ready jet!!!
            op->update_screen(dspl, SCREEN_MODE_GPS_INIT, 0);
            cur_screen = CUR_SCREEN_GPS_INFO;
            delay=1000;
        }
/* #if defined(GPIO12_ACTIF)
        // else if(Short_push12.long_pulse)
        else if (m_context.Field_choice2) {  //! io12_short_pulse
            op->update_screen(dspl, m_context.gpio12_screen[m_context.gpio12_screen_cur], 0);
        }  // heeft voorrang, na drukken GPIO_pin 12, 10 STAT4 scherm !!!
#endif */       
        else if (!run_is_active && (m_context.gps.S2.display_max_speed  > 1000 || next_screen == CUR_SCREEN_GPS_STATS)) { 
            // lower than 5 km/h
            if (m_context.stat_screen_cur >= stat_screen_count) {
                m_context.stat_screen_cur = 0;  // screen_count = 2
            }
            const uint8_t sc = (m_context.stat_screen_cur >= m_context.stat_screen_count) ? m_context.stat_screen_cur+1 : m_context.stat_screen[m_context.stat_screen_cur];
            op->update_screen(dspl, sc, 0);
            cur_screen = CUR_SCREEN_GPS_STATS;
            delay=500;
        } else {
            if (m_context.config->speed_large_font == 2) {
                    op->update_screen(dspl, SCREEN_MODE_SPEED_2, 0);
            } else {
                if(op)
                    op->update_screen(dspl, SCREEN_MODE_SPEED_1, 0);
            }
            m_context.stat_screen_cur = 0;
            stat_screen_count = m_context.stat_screen_count;
            cur_screen = CUR_SCREEN_GPS_SPEED;
        }
    }
    end:
#if defined(DEBUG)
    task_memory_info("screencb_task");
#endif
    xSemaphoreGive(lcd_refreshing_sem);
    return delay;
}

// uint32_t time_to_next_sec = 0;
// static void screenTask(void *parameter) {
//     //ILOG(TAG, "[%s]", __func__);
//     int wifistatus;
//     bool screen_active = true;
//     int32_t now, emillis, elapsed;
//     uint32_t loops = 0, delay = 0, next_screen_time = 0;
//     m_context.boot_screen_stage = 1;
//     task_memory_info("screenTask");
//     while (screen_active) {
//         now = get_millis();
//         elapsed = 0;
//         delay = 0;
        
//         if (next_screen_time <= now) {
            
//             // esp_event_post(LOGGER_EVENT, LOGGER_EVENT_SCREEN_UPDATE_END, &elapsed, sizeof(int32_t), portMAX_DELAY);
//         }
//         emillis = get_millis();
//         elapsed = emillis - now;
//         if (button_down) {
//                 next_screen_time = emillis + 50;
//         } else {
//             next_screen_time = emillis + ((delay > 99) ? 100 : delay) - elapsed;
//         }
//         if (elapsed < 51)
//             delay_ms(50 - elapsed);
//     }
//     vTaskDelete(NULL);
//     t2 = 0;
//     ESP_LOGI(TAG, "[%s] ended", __FUNCTION__);
// }

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
#ifdef CONFIG_BTN_GPIO_INPUT_1_ACTIVE
    btns[1].cb = button_cb;
#endif
}

char msgbbb[BUFSIZ*3];

void wifiTask() {
    uint16_t delay = 100;
    int16_t ap_trigger = 4;
    uint32_t loops = 0;
    task_memory_info("wifiTask");
    while (app_mode == APP_MODE_WIFI) {
        
#ifdef DEBUG
        if (loops++ > 100) {
            task_memory_info("wifiTask");
            loops = 0;
        }
#endif          
        if (m_context.request_restart) {
            app_mode = APP_MODE_RESTART;
        }
        int wifi = wifi_status();
        if (wifi < 1) {
            if (--ap_trigger == 0) {
                wifi_ap_start();
            }
            delay = 1000;
        } else {
            wifi_context.Wifi_on = true;
            delay = 200;
        }

        delay_ms(delay);
    }
    vTaskDelete(NULL);
    t3 = 0;
    app_mode_wifi_on = 0;
    m_context.NTP_time_set = 0;
}

void app_mode_wifi_handler(int verbose) {
    if (t3 || wifi_context.Wifi_on || app_mode_wifi_on)
        return;
    app_mode = APP_MODE_WIFI;
    app_mode_wifi_on = 1;
    if (!wifi_context.Wifi_on) {
        ILOG(TAG, "[%s] first turn wifi on", __FUNCTION__);
        wifi_init();
        wifi_context.offset = m_context.config->timezone;
        wifi_sta_set_config(m_context.config->ssid, m_context.config->password);
        wifi_sta_connect_scan();  // try station mode first
        // wifi_sta_connect(0);
        m_context.wifi_ap_timeout = 10;
        ILOG(TAG, "[%s] wifi sta started.", __FUNCTION__);
    }
    task_memory_info("wifiStarter-main");
    xTaskCreate(wifiTask,   /* Task function. */
                "wifiTask", /* String with name of task. */
                CONFIG_WIFI_TASK_STACK_SIZE,  /* Stack size in bytes. */
                NULL,      /* Parameter passed as input of the task */
                5,         /* Priority of the task. */
                &t3);      /* Task handle. */
}

void app_mode_gps_handler(int verbose) {
    if (t1)
        return;
    DMEAS_START();
    app_mode = APP_MODE_GPS;
    if (wifi_status() > 0 || wifi_context.Wifi_on == true) {
        wifi_context.Wifi_on = 0;
        if (!ota_notrunning) {
            ILOG(TAG, "[%s] wifi turn off ap", __FUNCTION__);
            // wifi_disconnect();
            // esp_wifi_stop();
            wifi_uninit();
            // set_time_zone(100);
            //http_rest_uninit();
            ota_notrunning = 1;
        }
        m_context.wifi_ap_timeout = 0;
    }
    if(!m_context.sdOK)
        goto end;
    
    task_memory_info("gpsStarter-main");
    xTaskCreatePinnedToCore(gpsTask,   /* Task function. */
                "gpsTask", /* String with name of task. */
                CONFIG_GPS_LOG_STACK_SIZE,  /* Stack size in bytes. */
                NULL,      /* Parameter passed as input of the task */
                19,         /* Priority of the task. */
                &t1, 1);      /* Task handle. */
    end:
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
                go_to_restart();
            }
            break;
        case APP_MODE_WIFI:
            app_mode_gps_on = 0;
                next_screen = CUR_SCREEN_NONE;
            shut_down_gps(1);
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
    
    if(base == LOGGER_EVENT) {
        switch(id) {
            case LOGGER_EVENT_SDCARD_MOUNTED:
                ILOG(TAG, "[%s] %s", __FUNCTION__, logger_event_strings[id]);
                //m_context.sdOK = true;
                //m_context.freeSpace = sdcard_space();
                break;
            case LOGGER_EVENT_SDCARD_MOUNT_FAILED:
                ILOG(TAG, "[%s] %s", __FUNCTION__, logger_event_strings[id]);
                m_context.sdOK = false;
                m_context.freeSpace = 0;
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
                break;
            case UBX_EVENT_UART_INIT_FAIL:
                ILOG(TAG, "[%s] %s", __FUNCTION__, ubx_event_strings[id]);
                break;
            case UBX_EVENT_UART_DEINIT_DONE:
                ILOG(TAG, "[%s] %s", __FUNCTION__, ubx_event_strings[id]);
                break;
            case UBX_EVENT_SETUP_DONE:
                ILOG(TAG, "[%s] %s", __FUNCTION__, ubx_event_strings[id]);
                break;
            case UBX_EVENT_SETUP_FAIL:
                ILOG(TAG, "[%s] %s", __FUNCTION__, ubx_event_strings[id]);
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
                http_start_webserver();
                break;
            case WIFI_EVENT_AP_STOP:
                ILOG(TAG, "[%s] %s", __FUNCTION__, wifi_event_strings[id]);
                http_stop_webserver();
                break;
            default:
                // ILOG(TAG, "[%s] %s:%" PRId32, __FUNCTION__, base, id);
                break;
        }
    }
    else if(base == IP_EVENT) {
        switch(id) {
            case IP_EVENT_STA_GOT_IP:
                ILOG(TAG, "[%s] IP_EVENT_STA_GOT_IP", __FUNCTION__);
                http_start_webserver();
                break;
            case IP_EVENT_STA_LOST_IP:
                ILOG(TAG, "[%s] IP_EVENT_STA_LOST_IP", __FUNCTION__);
                http_stop_webserver();
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
                // screen_cb(&display);
                if(displ_trigger_done)
                    displ_trigger_done = 0;
                break;
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

void sd_mount_cb(void* arg) {
    if(!sdcard_is_mounted()) {
        if(sdcard_mount()==ESP_OK) {
            m_context.sdOK = true;
            m_context.freeSpace = sdcard_space();
            ESP_LOGI(TAG, "[%s] sdcard mounted.", __FUNCTION__);

            config_load_json(m_config, m_context.filename, m_context.filename_backup);

            g_context_rtc_add_config(&m_context_rtc, m_config);
            g_context_add_config(&m_context, m_config);
            config_fix_values(m_config);
            g_context_ubx_add_config(&m_context, 0);
            log_config_add_config(m_context.gps.log_config, m_config);
            /* strbf_t sbc;
            strbf_init(&sbc);
            config_encode_json(&sbc);
            printf("conf:%s, size:%d", strbf_finish(&sbc), sbc.cur-sbc.start);
            strbf_reset(&sbc);
            config_get_json(&sbc, 0);
            printf("conf:%s, size:%d", strbf_finish(&sbc), sbc.cur-sbc.start);
            strbf_free(&sbc);
            */
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

#if defined(USE_WDT)
    init_watchdog();
#endif

    lcd_refreshing_sem = xSemaphoreCreateBinary();
    xSemaphoreGive(lcd_refreshing_sem);
    display_init(&display);

    // const esp_timer_create_args_t screen_periodic_timer_args = {
    //     .callback = &screen_cb,
    //     .name = "scr_tmr",
    //     .arg = &display
    // };
    // ESP_ERROR_CHECK(esp_timer_create(&screen_periodic_timer_args, &screen_periodic_timer));
    // ESP_LOGI(TAG, "[%s] start screen timer.", __FUNCTION__);
    // ESP_ERROR_CHECK(esp_timer_start_periodic(screen_periodic_timer, 1000000)); // 1000ms

    wakeup_init();  // Print the wakeup reason for ESP32, go back to sleep is timer is wake-up source !

    init_button();
    const esp_timer_create_args_t button_timer_args = {
        .callback = &button_timer_cb,
        .name = "btn_tmr",
        .arg = 0
    };
    ESP_ERROR_CHECK(esp_timer_create(&button_timer_args, &button_timer));

#ifdef CONFIG_USE_FATFS
    fatfs_init();
#endif
#ifdef CONFIG_USE_SPIFFS
    spiffs_init();
#endif
#ifdef CONFIG_USE_LITTLEFS
    littlefs_init();
#endif
    m_config = config_new();
    g_context_defaults(&m_context);
    m_context.boot_screen_stage = 2;
    m_context.config = m_config;
    if (!m_context.gps.Gps_fields_OK)
        init_gps_context_fields(&m_context.gps);
    ESP_LOGI(TAG, "[%s] init sdcard", __FUNCTION__);
    if(!sdcard_init())
        sd_mount_cb(NULL);

    // appstage 1, structures initialized, start gps.
    delay_ms(50);
    ret += 50;

    http_rest_init(CONFIG_WEB_APP_PATH);

#if defined(CONFIG_BMX_ENABLE)
    init_bmx();
#endif

#if defined(DEBUG)
    ESP_LOGI(TAG, "[%s] verbosity mode from common module %d.", __FUNCTION__, CONFIG_LOGGER_COMMON_LOG_LEVEL);
#elif defined(NDEBUG)
    ESP_LOGI(TAG, "[%s] silent mode.", __FUNCTION__);
#else
    ESP_LOGW(TAG, "[%s] build debug mode not set.", __FUNCTION__);
#endif
    task_memory_info("screenStarter-main");

#ifdef GPS_TIMER_STATS
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
        if (loops++ > 100) {
#ifdef DEBUG
            task_top();
            memory_info_large("main");
            task_memory_info("main");
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
#ifdef GPS_TIMER_STATS
    if(gps_periodic_timer)
        ESP_ERROR_CHECK(esp_timer_delete(gps_periodic_timer));
#endif
    vSemaphoreDelete(lcd_refreshing_sem);
    lcd_refreshing_sem = NULL;
    // esp_timer_stop(screen_periodic_timer);
    esp_timer_stop(button_timer);
    display_uninit(&display);
    config_delete(m_config);
    ubx_config_delete(m_context.gps.ublox_config);
    events_uninit();
}
