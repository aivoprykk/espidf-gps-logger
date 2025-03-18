
#include "private.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#if defined(CONFIG_LOGGER_USE_WDT)
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

#ifdef CONFIG_LOGGER_ADC_ENABLED
#include "adc.h"
#include "adc_events.h"
#endif
#ifdef CONFIG_BMX_ENABLE
#include "bmx.h"
#endif
#if defined(CONFIG_LOGGER_BUTTON_ENABLED)
#include "button.h"
#include "button_events.h"
#endif
#ifdef CONFIG_LOGGER_VFS_ENABLED
#include "vfs_events.h"
#include "vfs.h"

#endif
#if defined(CONFIG_GPS_LOG_ENABLED)
#include "gps_log.h"
#include "gps_data.h"
#include "gps_log_file.h"
#include "gps_log_events.h"
#include "gps_user_cfg.h"
#endif
#ifdef CONFIG_LOGGER_HTTP_ENABLED
#include "http_rest_server.h"
#endif
#ifdef CONFIG_OTA_ENABLED
#include "ota.h"
#endif
#ifdef CONFIG_OTA_USE_AUTO_UPDATE
#include "https_ota.h"
#include "ota_events.h"
#endif
#ifdef CONFIG_UBLOX_ENABLED
#include "ubx.h"
#include "ubx_events.h"
#endif
#ifdef CONFIG_LOGGER_WIFI_ENABLED
#include "logger_wifi.h"
#endif
#ifdef CONFIG_DISPLAY_ENABLED
#include "driver_vendor.h"
#include "ui_events.h"
#endif
#include "context.h"
#include "logger_config.h"
#include "dstat_screens.h"
#include "lcd.h"

// events
#include "config_events.h"
#include "logger_events.h"

ESP_EVENT_DEFINE_BASE(LOGGER_EVENT);

#define PIN_BAT 35

#define CALIBRATION_BAT_V 1.7      // voor proto 1
#define uS_TO_S_FACTOR  1000000UL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  21600UL     /* Time ESP32 will go to sleep (no for 6h, 4/day) */
#define WDT_TIMEOUT 60             // 60 seconds WDT, opgelet zoeken naar ssid time-out<dan 10s !!!

static const char *TAG = "main";

extern struct context_s m_context;
extern struct context_rtc_s m_context_rtc;

#ifdef CONFIG_LOGGER_WIFI_ENABLED
extern struct m_wifi_context wifi_context;
#endif

struct main_ctx_s m_app_ctx = {
.config = 0,
.ctx = &m_context,
.wifi_ctx = &wifi_context,
// app_mode
// 0 - before config, still init running
// 1 - config, wifi and http running
// 2 - run, gps running
.app_mode = APP_MODE_UNKNOWN,
.cur_screen = CUR_SCREEN_NONE,
.next_screen = CUR_SCREEN_NONE,
.stat_screen_count = 0,
.low_bat_countdown = 0,
.record_done = 25,
.button_down = false,
.button_press_mode = -1,
.gps_cfg_item = CFG_GPS_ITEM_BASE,
.stat_screen_cfg_item = 0,
.screen_cfg_item = 0,
.cfg_screen = 0,
.fw_update_screen = 0,
.fw_cfg_item = 0,
.app_mode_wifi_on = 0,
.app_mode_gps_on = 0,
.config_initialized = 0,
.screen_auto_refresh = 0,
#ifdef CONFIG_USE_SD_CARD
.sd_space = {0},
#endif
#ifdef CONFIG_USE_FATFS
.fatfs_space = {0},
#endif
};

#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 2 || CONFIG_LOGGER_GLOBAL_LOG_LEVEL < 2)
const char * const app_mode_str[] = { APP_MODE_LIST(STRINGIFY) };
const char * const cur_screen_str[] = { CUR_SCREEN_LIST(STRINGIFY) };
#endif

#if (defined(GPSSS))
static TaskHandle_t gps_task_handle = 0;
#endif

#if defined(CONFIG_LOGGER_USE_WDT)
static int wdt_task0, wdt_task1;
#endif

static esp_timer_handle_t button_timer = 0;
static uint8_t button_clicks = 0;

static const char * const wakeup_reasons[] = {
    0, 0,
    "ESP_SLEEP_WAKEUP_EXT0",
    "ESP_SLEEP_WAKEUP_EXT1",
    "ESP_SLEEP_WAKEUP_TIMER",
    "ESP_SLEEP_WAKEUP_TOUCHPAD",
    "ESP_SLEEP_WAKEUP_ULP",
    "ESP_SLEEP_WAKEUP_OTHER",
};

// #define L_FW_UPDATE_FIELDS 3
#define L_CFG_GROUP_FIELDS 4

// 200ms before exec cb
#if (defined(CONFIG_DISPLAY_DRIVER_ST7789) || defined(CONFIG_DISPLAY_DRIVER_QEMU))
#define BUTTON_CB_WAIT_BEFORE 300000U
#else
#define BUTTON_CB_WAIT_BEFORE 210000U
#endif

// static char msgbbb[BUFSIZ*3];

static void low_to_sleep(uint64_t sleep_time) {
    ILOG(TAG, "[%s]", __func__);
#if defined(CONFIG_DISPLAY_ENABLED)
    lcd_uninit();
#endif
#if defined(CONFIG_LOGGER_BUTTON_ENABLED)
    esp_timer_stop(button_timer);
    button_deinit();
#endif
#if defined(CONFIG_LOGGER_ADC_ENABLED)
    deinit_adc();
#endif
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
#if defined(CONFIG_LOGGER_ADC_ENABLED)
    m_context_rtc.RTC_voltage_bat = volt_read();
#endif
    if (verbose)
        DLOG(TAG, "[%s] computed:%.02f, required_min:%.02f\n", __FUNCTION__, m_context_rtc.RTC_voltage_bat, MINIMUM_VOLTAGE);
#endif
    if(m_context_rtc.RTC_voltage_bat < MINIMUM_VOLTAGE) {
        if(!m_app_ctx.low_bat_countdown) {
            m_app_ctx.low_bat_countdown = get_millis() + 60000;  // 60 seconds
        }

    }
    else if(m_app_ctx.low_bat_countdown) {
        m_app_ctx.low_bat_countdown = 0;
        m_context.low_bat_count = 0;
    }
}

/*
Method to print the reason by which ESP32 has been awaken from sleep
*/

static int wakeup_init() {
    ILOG(TAG, "[%s]", __func__);
    int ret = 0;
    DMEAS_START();
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    // First screen update call from wakeup
    if(wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) 
    // screen_cb(&display);

    if (m_context_rtc.RTC_voltage_bat < MINIMUM_VOLTAGE) {
        lowbat:
        m_app_ctx.app_mode = APP_MODE_SLEEP;
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

static void do_restart() {
    ILOG(TAG, "[%s]", __func__);
    esp_restart();
}

static void go_to_sleep(uint64_t sleep_time) {
    ILOG(TAG, "[%s]", __func__);
#if defined(CONFIG_LOGGER_WIFI_ENABLED)
    if (wifi_status() > 0) {
        wifi_uninit();
    }
#endif
#if defined(CONFIG_DISPLAY_ENABLED)
    display_wait_for_task();
#endif
    // write_rtc(&m_context_rtc);
    vfs_uninit();
    if(sleep_time > 0) {
        low_to_sleep(sleep_time);
    } else {
        do_restart(0);
    }
}

static int shut_down_gps(int no_sleep) {
    ILOG(TAG, "[%s]", __func__);
    int ret = gps_shut_down();
#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 2 || defined(DEBUG))
    task_memory_info(__func__);
#endif
    if (!no_sleep) {
        go_to_sleep(3);  // got to sleep after 5 s, this to prevent booting when
        // GPIO39 is still low !
    }
    return ret;
}

#if defined(CONFIG_LOGGER_BUTTON_ENABLED)
static void button_timer_cb(void *arg) {
    ILOG(TAG, "[%s]", __func__);
#if (defined(CONFIG_UBLOX_ENABLED) && defined(CONFIG_GPS_LOG_ENABLED))
    ubx_config_t *ubx_dev = m_context.gps.ubx_device;
    const ubx_hw_t hw_type = ubx_dev->rtc_conf->hw_type;
#endif
    if(button_clicks == 1) {
        ILOG(TAG, "[%s] Button single click arrived, %s.", __func__, m_app_ctx.button_press_mode == 3 ? "lllong" : m_app_ctx.button_press_mode == 2 ? "llong" : m_app_ctx.button_press_mode == 1 ? "long" : "short");
        uint8_t flush_times = 1;
        int8_t fast_refr_time = -1;
        if(m_app_ctx.button_press_mode==3) { // long long long press
            if(m_app_ctx.next_screen == CUR_SCREEN_SETTINGS||m_app_ctx.next_screen == CUR_SCREEN_FW_UPDATE) m_app_ctx.next_screen = CUR_SCREEN_NONE;
            m_context.request_restart = true;
            goto done; // not requesting refresh here
        }
        else if(m_app_ctx.button_press_mode==2) { // long long press
#if defined(CONFIG_LOGGER_WIFI_ENABLED)
            if (m_app_ctx.app_mode == APP_MODE_GPS && m_app_ctx.app_mode_wifi_on == 0) {
                m_app_ctx.app_mode = APP_MODE_WIFI;
                if(m_app_ctx.next_screen == CUR_SCREEN_SETTINGS) m_app_ctx.next_screen = CUR_SCREEN_NONE;
                // lcd_ui_request_full_refresh(0);
            } else if (m_app_ctx.app_mode == APP_MODE_WIFI && m_app_ctx.app_mode_wifi_on == 1) {
                // m_app_ctx.app_mode = APP_MODE_GPS;
                m_context.request_restart = true;
                goto done;
            }
#else
            if(m_app_ctx.next_screen == CUR_SCREEN_SETTINGS) m_app_ctx.next_screen = CUR_SCREEN_NONE;
            m_context.request_restart = true;
            goto done;
#endif
        }
        else if (m_app_ctx.button_press_mode==1) { // long press
            if (m_app_ctx.next_screen == CUR_SCREEN_SETTINGS){
                ILOG(TAG, "[%s] settings new screen requested %d", __func__, 1);
                    if(m_app_ctx.cfg_screen >= L_CFG_GROUP_FIELDS-1)
                        m_app_ctx.cfg_screen = 0;
                    else
                        ++m_app_ctx.cfg_screen;
            } else if (m_app_ctx.cur_screen == CUR_SCREEN_FW_UPDATE){
                ILOG(TAG, "[%s] fw update choice saved %d", __func__, 1);
                if(m_app_ctx.fw_update_screen == 0) {
                    m_context.fw_update_is_allowed = 1;
                } else if(m_app_ctx.fw_update_screen == 1) {
                    m_context.fw_update_postponed = get_millis() + 86400000; // 24 hours
                } else {
                    m_context.fw_update_postponed = get_millis() + 3600000; // 1 hours
                }
            } else {
                m_context.request_shutdown = true;
                goto done; // not requesting refresh here
            }
        }
        else if (m_app_ctx.button_press_mode==0) { // just click
            if (m_app_ctx.cur_screen == CUR_SCREEN_FW_UPDATE){
                ILOG(TAG, "[%s] fw update next choice %d", __func__, 1);
                    if(m_app_ctx.fw_update_screen >= config_fw_update_item_count-1)
                        m_app_ctx.fw_update_screen = 0;
                    else
                        ++m_app_ctx.fw_update_screen;
                goto refresh;
            }
            else if(m_app_ctx.cur_screen == CUR_SCREEN_SETTINGS) {
                ILOG(TAG, "[%s] settings next requested %d", __func__, 1);
                if(m_app_ctx.cfg_screen == CFG_GROUP_GPS) {
                    if(m_app_ctx.gps_cfg_item < (CFG_GPS_ITEM_BASE))
                        m_app_ctx.gps_cfg_item = CFG_GPS_ITEM_BASE;
                    else if(++m_app_ctx.gps_cfg_item >= (CFG_GPS_ITEM_BASE + gps_user_cfg_item_count))
                        m_app_ctx.gps_cfg_item = CFG_GPS_ITEM_BASE;
                    ILOG(TAG, "[%s] gps settings next requested %hhu", __func__, m_app_ctx.gps_cfg_item);
                }
                else if(m_app_ctx.cfg_screen == CFG_GROUP_STAT_SCREENS) {
                    if(++m_app_ctx.stat_screen_cfg_item >= config_stat_screen_item_count)
                        m_app_ctx.stat_screen_cfg_item = 0;
                }
                else if(m_app_ctx.cfg_screen == CFG_GROUP_SCREEN) {
                    if(++m_app_ctx.screen_cfg_item >= config_screen_item_count)
                        m_app_ctx.screen_cfg_item = 0;
                }
                else if(m_app_ctx.cfg_screen == CFG_GROUP_FW) {
                    if(++m_app_ctx.fw_cfg_item >= 2)
                        m_app_ctx.fw_cfg_item = 0;
                }
                goto refresh;
            }
            if (m_app_ctx.app_mode == APP_MODE_GPS){
                if(m_app_ctx.cur_screen == CUR_SCREEN_GPS_SPEED) {
                    ILOG(TAG, "[%s] gps info next screen requested, cur: %hhu", __func__, m_context.config->screen.speed_field);
                    m_context.config->screen.speed_field++;
                    if (m_context.config->screen.speed_field >= config_speed_field_item_count)
                        m_context.config->screen.speed_field = 1;
                    m_context.Field_choice = 1;
                }
                else if(m_app_ctx.cur_screen==CUR_SCREEN_GPS_STATS) {
                    if(++m_context.stat_screen_cur >= get_stat_screens_count()) m_context.stat_screen_cur = 0;
                    ILOG(TAG, "[%s] next screen requested, cur: %hhu", __func__, m_context.stat_screen_cur);
                    m_app_ctx.stat_screen_count = get_stat_screens_count();
                }
            }
        }
        refresh:
#if defined(CONFIG_DISPLAY_ENABLED)
        if(!m_app_ctx.screen_auto_refresh && display_task_is_paused()) {
            display_task_resume_for_times(flush_times, fast_refr_time, -1, false); // one partial refresh
        }
#endif
    }
    else if(button_clicks==2) {
        ILOG(TAG, "[%s] Button double click arrived, %s", __func__,  m_app_ctx.button_press_mode == 3 ? "lllong" :  m_app_ctx.button_press_mode == 2 ? "llong" :  m_app_ctx.button_press_mode == 1 ? "long" : "short");
        if (m_app_ctx.app_mode == APP_MODE_GPS) {
            if(m_app_ctx.next_screen == CUR_SCREEN_GPS_INFO || m_app_ctx.cur_screen == CUR_SCREEN_GPS_INFO) {
                ILOG(TAG, "[%s] setting screen requested", __func__);
                m_app_ctx.next_screen = CUR_SCREEN_SETTINGS;
            }
            else if(m_app_ctx.next_screen==CUR_SCREEN_NONE) {
                ILOG(TAG, "[%s] gps info screen requested", __func__);
                m_app_ctx.next_screen = CUR_SCREEN_GPS_INFO;
            }
            else if(m_app_ctx.next_screen==CUR_SCREEN_SETTINGS) {
                ILOG(TAG, "[%s] gps_stats screen requested", __func__);
                m_app_ctx.next_screen = CUR_SCREEN_GPS_STATS;
            }
            else {
                ILOG(TAG, "[%s] default screen requested", __func__);
                m_app_ctx.next_screen = CUR_SCREEN_NONE;
            }
        }
#if defined (CONFIG_LOGGER_WIFI_ENABLED)
        else if(m_app_ctx.app_mode == APP_MODE_WIFI) {
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
#endif
#if defined(CONFIG_DISPLAY_ENABLED)
        if(!m_app_ctx.screen_auto_refresh){
            display_task_resume_for_times(1, -1, -1, false);
        }
        else
            display_request_full_refresh(0);
#endif
    }
    else if(button_clicks==3) {
        ILOG(TAG, "[%s] Button triple click arrived, %s", __func__,  m_app_ctx.button_press_mode == 3 ? "lllong" :  m_app_ctx.button_press_mode == 2 ? "llong" :  m_app_ctx.button_press_mode == 1 ? "long" : "short");
#if (defined(CONFIG_UBLOX_ENABLED) && defined(CONFIG_GPS_LOG_ENABLED))
        if(!(m_app_ctx.app_mode == APP_MODE_GPS && m_app_ctx.next_screen == CUR_SCREEN_SETTINGS)) {
            ILOG(TAG, "[%s] screen rotation change requested", __func__);
            if(set_screen_cfg_item(m_app_ctx.config, CGG_SCREEN_ITEM_ROTATION_POS)) {
                g_context_rtc_add_config(&m_context_rtc, m_context.config);
#if defined(CONFIG_DISPLAY_ENABLED)
                display_set_rotation(m_context_rtc.RTC_screen_rotation);
#endif
            }
            goto refresh;
        }
        if (m_app_ctx.app_mode == APP_MODE_GPS) {
            if(m_app_ctx.next_screen==CUR_SCREEN_SETTINGS) {
                ILOG(TAG, "[%s] settings screen change requested", __func__);
                if(m_app_ctx.cfg_screen == CFG_GROUP_GPS) {
                    if(set_gps_cfg_item(m_app_ctx.gps_cfg_item)) {
                        ILOG(TAG, "[%s] settings screen gps change saved", __func__);
                        // g_context_ubx_add_config(&m_context, ubx_dev);
                        // g_context_rtc_add_config(&m_context_rtc, m_context.config);
                        // m_app_ctx.ubx_restart_requested = 1;
                    }
                }
                else if(m_app_ctx.cfg_screen == CFG_GROUP_STAT_SCREENS) {
                    if(set_stat_screen_cfg_item(m_app_ctx.config, m_app_ctx.stat_screen_cfg_item)) {
                        ILOG(TAG, "[%s] settings screen change requested", __func__);
                        g_context_add_config(&m_context, m_context.config);
                    }
                }
                else if(m_app_ctx.cfg_screen == CFG_GROUP_SCREEN) {
                    int changed = 0;
                    if((changed = (set_screen_cfg_item(m_app_ctx.config, m_app_ctx.screen_cfg_item)))) {
                        ILOG(TAG, "[%s] settings screen change requested", __func__);
                        g_context_rtc_add_config(&m_context_rtc, m_context.config);
#if defined(CONFIG_DISPLAY_ENABLED)
                        if(changed == cfg_screen_rotation)
                            display_set_rotation(m_context_rtc.RTC_screen_rotation);
#if !defined(CONFIG_LCD_IS_EPD)
                        else
                       if(changed == cfg_screen_brightness)
                            display_drv_bl_set(m_context_rtc.RTC_screen_brightness);
#endif
#endif
                    }
                }
                else if(m_app_ctx.cfg_screen == CFG_GROUP_FW) {
                    if(set_fw_update_cfg_item(m_app_ctx.config, m_app_ctx.fw_cfg_item)) {
                        ILOG(TAG, "[%s] settings fw change requested", __func__);
                        // g_context_add_config(&m_context, m_context.config);
                    }
                }
#if defined(CONFIG_DISPLAY_ENABLED)
                if(!m_app_ctx.screen_auto_refresh){
                    display_task_resume_for_times(1, -1, -1, false);
                }
                // lcd_ui_request_fast_refresh(0);
#endif
            }
        } 
#endif
    } else if(button_clicks==4) {
            ILOG(TAG, "[%s] Button 4 click arrived", __func__);
#if !defined(CONFIG_LCD_IS_EPD)
            int changed = 0;
            if((changed = (set_screen_cfg_item(m_app_ctx.config, CGG_SCREEN_ITEM_BRIGHTNESS_POS)))) {
                g_context_rtc_add_config(&m_context_rtc, m_context.config);
#if defined(CONFIG_DISPLAY_ENABLED)
                if(changed == cfg_screen_brightness)
                    display_drv_bl_set(m_context_rtc.RTC_screen_brightness);
#endif
            }
#endif
    }
    done:
#if defined(CONFIG_DISPLAY_ENABLED)
    display_cancel_delay();
#endif
    button_clicks = 0;
    m_app_ctx.button_press_mode = -1;
}

static void button_cb(int num, l_button_ev_t ev, uint64_t time) {
    uint32_t tm = time/1000;
    l_button_t *btn = 0;
    ILOG(TAG, "[%s] num: %d event: %s", __func__, ev, l_button_ev_list[ev]);
    //ESP_LOGI(TAG, "Button %d event: %d, time: %ld ms", num, ev, tm);
    switch (ev) {
    case L_BUTTON_UP:
        m_app_ctx.button_down = false;
        if(num==0) {
            esp_timer_start_once(button_timer, BUTTON_CB_WAIT_BEFORE);
#if defined(CONFIG_LOGGER_BUTTON_GPIO_1)
        } else if(num==1) {
            struct gps_context_s *gps = &m_context.gps;
            const struct ubx_config_s *ubx_dev = gps->ubx_device;
            if(tm >= CONFIG_LOGGER_BUTTON_LONG_PRESS_TIME_MS) {
                if (ubx_dev->ready && gps->signal_ok) {
                    reset_time_stats(&gps->s10);
                    reset_time_stats(&gps->s2);
                    reset_alfa_stats(&gps->a500);
                }
            } else {
                /* if (ubx_dev->ready && ubx_dev->signal_ok) {
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
        m_app_ctx.button_down = true;
        m_app_ctx.button_press_mode = 0;
        button_clicks++;
        break;
    case L_BUTTON_LONG_PRESS_START:
        m_app_ctx.button_press_mode = 1;
#if defined(CONFIG_DISPLAY_ENABLED)
        if(!m_app_ctx.screen_auto_refresh){
            display_task_resume_for_times(1, -1, -1, false);
        }
#endif
        break;
    case L_BUTTON_LONG_LONG_PRESS_START:
        if(num==0 && tm >= 9700) {
            m_app_ctx.button_press_mode = 3;
            ILOG(TAG, "[%s] Button num: %d lllong press detected, time: %lld, restart requested.", __func__, num, time);
            m_context.request_restart = true;
            break;
        }
        else{
            m_app_ctx.button_press_mode = 2;
#if defined(CONFIG_DISPLAY_ENABLED)
            if(!m_app_ctx.screen_auto_refresh){
                display_task_resume_for_times(1, -1, -1, false);
            }
#endif
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

#endif


#if defined(CONFIG_LOGGER_USE_WDT)
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

#if defined(CONFIG_LOGGER_BUTTON_ENABLED)
static void init_button() {
    ILOG(TAG, "[%s]", __func__);
    button_init();
    btns[0].cb = button_cb;
#if defined(CONFIG_LOGGER_BUTTON_GPIO_1)
    btns[1].cb = button_cb;
#endif
}
#endif

#if defined(CONFIG_LOGGER_WIFI_ENABLED)
void wifi_sta_conf_sync() {
    ILOG(TAG, "[%s]", __func__);
    for(uint8_t i=0, j=5; i<j; ++i) {
        if(i>0 && !m_app_ctx.config->wifi_sta[i].ssid[0]) break;
        if (strcmp(wifi_context.stas[i].ssid, m_app_ctx.config->wifi_sta[i].ssid)) {
            strcpy(wifi_context.stas[i].ssid, m_app_ctx.config->wifi_sta[i].ssid);
            strcpy(wifi_context.stas[i].password, m_app_ctx.config->wifi_sta[i].password);
        }
    }
    wifi_context.offset = c_gps_cfg.timezone;
}

void app_mode_wifi_handler(int verbose) {
    if (m_app_ctx.app_mode_wifi_on) return;
    ILOG(TAG, "[%s]", __func__);
    m_app_ctx.app_mode = APP_MODE_WIFI;
    m_app_ctx.app_mode_wifi_on = 1;
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
#endif

void app_mode_gps_handler(int verbose) {
    if (m_app_ctx.app_mode_gps_on) return;
    DMEAS_START();
    m_app_ctx.app_mode = APP_MODE_GPS;
#if defined(CONFIG_LOGGER_WIFI_ENABLED)
    if (wifi_context.s_wifi_initialized) {
        wifi_uninit();
        m_context.NTP_time_set = 0;
    }
#endif
    if(!m_app_ctx.config_initialized) {
        goto end;
    }
    m_app_ctx.app_mode_gps_on = 1;
#if (defined(CONFIG_UBLOX_ENABLED) && defined(CONFIG_GPS_LOG_ENABLED))
    gps_task_start();
#endif
    end:
#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 2 || defined(DEBUG))
    task_memory_info(__func__);
#endif
    DMEAS_END(TAG, "[%s] took %llu us", __FUNCTION__);
}

void task_app_mode_handler(int verbose) {
    switch (m_app_ctx.app_mode) {
        case APP_MODE_RESTART:
        case APP_MODE_SHUT_DOWN:
            m_app_ctx.app_mode_wifi_on = 0;
            m_app_ctx.app_mode_gps_on = 0;
            
            if (m_app_ctx.app_mode == APP_MODE_SHUT_DOWN) {
                shut_down_gps(0);  // save gps
            } else {
                shut_down_gps(1);  // save gps
                go_to_sleep(0);
            }
            break;
        case APP_MODE_WIFI:
#if defined(CONFIG_LOGGER_WIFI_ENABLED)
            m_app_ctx.app_mode_gps_on = 0;
            // next_screen = CUR_SCREEN_NONE;
            app_mode_wifi_handler(verbose);
            break;
#endif
        case APP_MODE_GPS:
        case APP_MODE_BOOT:
            m_app_ctx.app_mode_wifi_on = 0;
            app_mode_gps_handler(verbose);
            break;
        default:
            break;
    };
}

#if defined(CONFIG_DISPLAY_PWR)
void init_power() {
    gpio_set_direction((gpio_num_t)CONFIG_DISPLAY_PWR, GPIO_MODE_OUTPUT);
    gpio_set_level(CONFIG_DISPLAY_PWR, 1);
}
#endif    

static void gps_save_rtc() {
    m_context_rtc.RTC_distance = m_context.gps.Ublox.total_distance / 1000000;
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
    
    struct tm tms;
    getLocalTime(&tms, 0);
    m_context_rtc.RTC_year = ((tms.tm_year) + 1900);  // local time is corrected with timezone in close_files() !!
    m_context_rtc.RTC_month = ((tms.tm_mon) + 1);
    m_context_rtc.RTC_day = (tms.tm_mday);
    m_context_rtc.RTC_hour = (tms.tm_hour);
    m_context_rtc.RTC_min = (tms.tm_min);
}

static void all_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    uint8_t no_auto_refresh = m_app_ctx.config ? !m_app_ctx.screen_auto_refresh : 0;
#if defined(CONFIG_LOGGER_VFS_ENABLED)
    if(base == VFS_EVENT) {
        switch(id) {
            case VFS_EVENT_SDCARD_MOUNTED:
                ILOG(TAG, "[%s] %s", __FUNCTION__, vfs_event_strings[id]);
                m_context.sdOK = true;
                //m_context.freeSpace = sdcard_space();
                break;
            case VFS_EVENT_SDCARD_MOUNT_FAILED:
                ILOG(TAG, "[%s] %s", __FUNCTION__, vfs_event_strings[id]);
                m_context.sdOK = false;
#if defined(CONFIG_DISPLAY_ENABLED)
                if(no_auto_refresh){
                    display_task_resume_for_times(1, -1, -1, false);
                }
#endif
                break;
            case VFS_EVENT_SDCARD_UNMOUNTED:
                ILOG(TAG, "[%s] %s", __FUNCTION__, vfs_event_strings[id]);
                m_context.sdOK = false;
                break;
            case VFS_EVENT_FAT_PARTITION_MOUNTED:
                ILOG(TAG, "[%s] %s", __FUNCTION__, vfs_event_strings[id]);
                break;
            case VFS_EVENT_FAT_PARTITION_MOUNT_FAILED:
                ILOG(TAG, "[%s] %s", __FUNCTION__, vfs_event_strings[id]);
                break;
            case VFS_EVENT_FAT_PARTITION_UNMOUNTED:
                break;
            default:
                // ILOG(TAG, "[%s] %s:%" PRId32, __FUNCTION__, base, id);
                break;
        }
    } 
    else
#endif
#if defined(CONFIG_OTA_USE_AUTO_UPDATE)
    if(base == OTA_AUTO_EVENT) {
        const char * c =  "OTA_UNKNOWN_EVENT";
        switch(id) {
            case OTA_AUTO_EVENT_UPDATE_FINISH:
                ILOG(TAG, "[%s] ota %s", __FUNCTION__, id < 4 ? ota_auto_event_strings[id] : c);
                if(!m_context.request_restart) {
                    m_app_ctx.next_screen = CUR_SCREEN_NONE;
#if defined(CONFIG_DISPLAY_ENABLED)
                    if(no_auto_refresh){
                        display_task_resume_for_times(1, -1, -1, false);
                    }
#endif
                }
                break;
            case OTA_AUTO_EVENT_UPDATE_FAILED:
                ILOG(TAG, "[%s] ota %s", __FUNCTION__, id < 4 ? ota_auto_event_strings[id] : c);
#if defined(CONFIG_DISPLAY_ENABLED)
                if(no_auto_refresh){
                    display_task_resume_for_times(1, -1, -1, false);
                }
#endif
                break;
            case OTA_AUTO_EVENT_UPDATE_START:
                ILOG(TAG, "[%s] ota %s", __FUNCTION__, id < 4 ? ota_auto_event_strings[id] : c);
                m_app_ctx.next_screen = CUR_SCREEN_FW_UPDATE;
#if defined(CONFIG_DISPLAY_ENABLED)
                if(no_auto_refresh){
                    display_task_resume_for_times(1, -1, -1, false);
                }
#endif
                break;

            default:
                // ILOG(TAG, "[%s] %s:%" PRId32, __FUNCTION__, base, id);
                break;
        }
    } 
    else
#endif
    if(base == LOGGER_EVENT) {
        switch(id) {
            case LOGGER_EVENT_DATETIME_SET:
                ILOG(TAG, "[%s] l %s", __FUNCTION__, logger_event_strings(id));
#if defined(CONFIG_DISPLAY_ENABLED)
                if(no_auto_refresh){
                    display_task_resume_for_times(1, -1, -1, false);
                }
#endif
                break;
            default:
                // ILOG(TAG, "[%s] %s:%" PRId32, __FUNCTION__, base, id);
                break;
        }
    }
    else if(base == LOGGER_CONFIG_EVENT) {
        switch(id) {
            case LOGGER_CONFIG_EVENT_CFG_CHANGED:
                ILOG(TAG, "[%s] g %s d %hhu", __FUNCTION__, logger_config_event_strings[id], *((uint8_t*)event_data));
                break;
            case LOGGER_CONFIG_EVENT_CFG_SET:
                ILOG(TAG, "[%s] g %s d %hhu", __FUNCTION__, logger_config_event_strings[id], *((uint8_t*)event_data));
                config_save_json(m_app_ctx.config);
                break;
            case LOGGER_CONFIG_EVENT_CFG_GET:
                ILOG(TAG, "[%s] c %s", __FUNCTION__, logger_config_event_strings[id]);
                break;
            case LOGGER_CONFIG_EVENT_INIT_DONE:
                ILOG(TAG, "[%s] c %s", __FUNCTION__, logger_config_event_strings[id]);
                break;
            case LOGGER_CONFIG_EVENT_SAVE_DONE:
                ILOG(TAG, "[%s] c %s", __FUNCTION__, logger_config_event_strings[id]);
                break;
            case LOGGER_CONFIG_EVENT_SAVE_FAIL:
                ILOG(TAG, "[%s] c %s", __FUNCTION__, logger_config_event_strings[id]);
                break;
        }

    }
#if defined(CONFIG_UBLOX_ENABLED)
    else if(base == UBX_EVENT) {
        switch(id) {
            case UBX_EVENT_DATETIME_SET:
                ILOG(TAG, "[%s] u %s", __FUNCTION__, ubx_event_strings(id));
                break;
            case UBX_EVENT_UART_INIT_DONE:
                ILOG(TAG, "[%s] u %s", __FUNCTION__, ubx_event_strings(id));
#if defined(CONFIG_DISPLAY_ENABLED)
                if(no_auto_refresh && (m_app_ctx.cur_screen == CUR_SCREEN_GPS_INFO || m_app_ctx.cur_screen == CUR_SCREEN_NONE)){
                    display_task_resume_for_times(1, -1, -1, false);
                }
#endif
                break;
            case UBX_EVENT_UART_INIT_FAIL:
                ILOG(TAG, "[%s] u %s", __FUNCTION__, ubx_event_strings(id));
#if defined(CONFIG_DISPLAY_ENABLED)
                if(no_auto_refresh && (m_app_ctx.cur_screen == CUR_SCREEN_GPS_INFO || m_app_ctx.cur_screen == CUR_SCREEN_NONE)){
                    display_task_resume_for_times(1, -1, -1, false);
                }
#endif
                break;
            case UBX_EVENT_UART_DEINIT_DONE:
                ILOG(TAG, "[%s] u %s", __FUNCTION__, ubx_event_strings(id));
                break;
            case UBX_EVENT_SETUP_DONE:
                ILOG(TAG, "[%s] u %s", __FUNCTION__, ubx_event_strings(id));
#if defined(CONFIG_DISPLAY_ENABLED)
                if(no_auto_refresh){
                    display_task_resume_for_times(1, -1, -1, false);
                }
#endif
                break;
            case UBX_EVENT_SETUP_FAIL:
                ILOG(TAG, "[%s] u %s", __FUNCTION__, ubx_event_strings(id));
#if defined(CONFIG_DISPLAY_ENABLED)
                if(no_auto_refresh && (m_app_ctx.cur_screen == CUR_SCREEN_GPS_INFO || m_app_ctx.cur_screen == CUR_SCREEN_NONE)){
                    display_task_resume_for_times(1, -1, -1, false);
                }
#endif
                break;
            default:
                // ILOG(TAG, "[%s] %s:%" PRId32, __FUNCTION__, base, id);
                break;
        }
    }
#endif
#if defined(CONFIG_GPS_LOG_ENABLED)
    else if(base==GPS_LOG_EVENT) {
        const char * c = 0;
        switch(id) {
            case GPS_LOG_EVENT_LOG_FILES_OPENED:
                ILOG(TAG, "[%s] g %s", __FUNCTION__, gps_log_event_strings(id));
                m_context.Shut_down_Save_session = true;
                goto printfiles;
                break;
            case GPS_LOG_EVENT_LOG_FILES_OPEN_FAILED:
                ILOG(TAG, "[%s] g %s", __FUNCTION__, gps_log_event_strings(id));
                printfiles:
                for(uint8_t i=0; i<SD_FD_END; i++) {
                    printf("** [%s] ", i==SD_GPX ? "GPX" : 
#ifdef GPS_LOG_ENABLE_GPY
                    i==SD_GPY ? "GPY" : 
#endif
                    i==SD_SBP ? "SBP" : i==SD_UBX ? "UBX" : i==SD_TXT ? "TXT" : "-");
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
                ILOG(TAG, "[%s] g %s", __FUNCTION__, gps_log_event_strings(id));
                break;
            case GPS_LOG_EVENT_LOG_FILES_CLOSED:
                ILOG(TAG, "[%s] g %s", __FUNCTION__, gps_log_event_strings(id));
                break;
            case GPS_LOG_EVENT_GPS_SAVE_FILES:
                ILOG(TAG, "[%s] g %s", __FUNCTION__, gps_log_event_strings(id));
                m_app_ctx.next_screen = CUR_SCREEN_SAVE_SESSION;
                gps_save_rtc();
                break;
            case GPS_LOG_EVENT_GPS_SHUT_DOWN_DONE:
                ILOG(TAG, "[%s] g %s", __FUNCTION__, gps_log_event_strings(id));
                if(m_app_ctx.next_screen == CUR_SCREEN_SAVE_SESSION) {
                    m_app_ctx.next_screen = CUR_SCREEN_NONE;
                }
                break;
            case GPS_LOG_EVENT_GPS_REQUEST_RESTART:
                ILOG(TAG, "[%s] g %s", __FUNCTION__, gps_log_event_strings(id));
                m_context.request_restart = 1;
                break;
            case GPS_LOG_EVENT_GPS_IS_MOVING:
                ILOG(TAG, "[%s] g %s", __FUNCTION__, gps_log_event_strings(id));
#if defined(CONFIG_DISPLAY_ENABLED)
                if(!m_app_ctx.screen_auto_refresh && display_task_is_paused()) {
                    display_task_resume();
                }
#endif
                break;
            case GPS_LOG_EVENT_GPS_IS_STOPPING:
                ILOG(TAG, "[%s] g %s", __FUNCTION__, gps_log_event_strings(id));
#if defined(CONFIG_DISPLAY_ENABLED)
                if(!m_app_ctx.screen_auto_refresh) {
                    if(!display_task_is_paused()) {
                        display_task_pause();
                        goto showscr;
                    }
                    if(m_app_ctx.record_done < 240) {
                        showscr:
                        display_task_resume_for_times(2, -1, -1, true);
                    }
                }
#endif
                break;
            case GPS_LOG_EVENT_GPS_FIRST_FIX:
                ILOG(TAG, "[%s] g %s", __FUNCTION__, gps_log_event_strings(id));
#if defined(CONFIG_DISPLAY_ENABLED)
                if(!m_app_ctx.screen_auto_refresh) {
                    display_task_resume_for_times(1, -1, -1, true);
                }
#endif
                break;
            case GPS_LOG_EVENT_GPS_NEW_RUN:
                ILOG(TAG, "[%s] g %s", __FUNCTION__, gps_log_event_strings(id));
#if defined(CONFIG_DISPLAY_ENABLED)
                display_cancel_delay();
#endif
                break;
            case GPS_LOG_EVENT_CFG_CHANGED:
                ILOG(TAG, "[%s] g %s d %hhu", __FUNCTION__, gps_log_event_strings(id), *((uint8_t*)event_data));
                break;
            case GPS_LOG_EVENT_CFG_SET:
                ILOG(TAG, "[%s] g %s d %hhu", __FUNCTION__, gps_log_event_strings(id), *((uint8_t*)event_data));
                config_save_json(m_app_ctx.config);
                break;
            default:
                // ILOG(TAG, "[%s] %s:%" PRId32, __FUNCTION__, base, id);
                break;
        }
    }
#endif
#if defined CONFIG_LOGGER_ADC_ENABLED
    else if(base == ADC_EVENT) {
        switch(id) {
            case ADC_EVENT_VOLTAGE_UPDATE:
                //ILOG(TAG, "[%s] ADC_EVENT_VOLTAGE_UPDATE", __FUNCTION__);
                break;
            case ADC_EVENT_BATTERY_LOW:
                ILOG(TAG, "[%s] a %s", __FUNCTION__, adc_event_strings(id));
#if defined(CONFIG_DISPLAY_ENABLED)
                if(no_auto_refresh){
                    display_task_resume_for_times(1, -1, -1, false);
                }
#endif
                break;
            case ADC_EVENT_BATTERY_OK:
                ILOG(TAG, "[%s] a %s", __FUNCTION__, adc_event_strings(id));
                break;
            default:
                // ILOG(TAG, "[%s] %s:%" PRId32, __FUNCTION__, base, id);
                break;
        }
    }
#endif
#if defined CONFIG_LOGGER_WIFI_ENABLED
    else if(base == WIFI_EVENT) {
        switch(id) {
            case WIFI_EVENT_AP_START:
                ILOG(TAG, "[%s] w %s", __FUNCTION__, wifi_event_strings(id));
#if defined(CONFIG_DISPLAY_ENABLED)
                if(no_auto_refresh){
                    display_task_resume_for_times(1, -1, -1, false);
                }
#endif
                break;
            case WIFI_EVENT_AP_STOP:
                ILOG(TAG, "[%s] w %s", __FUNCTION__, wifi_event_strings(id));
                break;
            default:
                break;
        }
    }
    else if(base == IP_EVENT) {
        switch(id) {
            case IP_EVENT_STA_GOT_IP:
                ILOG(TAG, "[%s] i IP_EVENT_STA_GOT_IP", __FUNCTION__);
#if defined(CONFIG_DISPLAY_ENABLED)
                if(no_auto_refresh){
                    display_task_resume_for_times(1, -1, -1, false);
                }
#endif
                break;
            case IP_EVENT_STA_LOST_IP:
                ILOG(TAG, "[%s] i IP_EVENT_STA_LOST_IP", __FUNCTION__);
#if defined(CONFIG_DISPLAY_ENABLED)
                if(no_auto_refresh){
                    display_task_resume_for_times(1, -1, -1, false);
                }
#endif
                break;
            default:
                break;
        }
    }
#endif
#if defined(CONFIG_DISPLAY_ENABLED)
    else if(base == UI_EVENT) {
        switch(id) {
            case UI_EVENT_FLUSH_START:
                ILOG(TAG, "[%s] d %s", __FUNCTION__, ui_event_strings(id));
                break;
            case UI_EVENT_FLUSH_DONE:
                ILOG(TAG, "[%s] d %s", __FUNCTION__, ui_event_strings(id));
                if(!m_app_ctx.screen_auto_refresh) {
                    if(m_app_ctx.app_mode == APP_MODE_GPS && display_task_is_paused() && m_app_ctx.record_done < 240 && m_app_ctx.record_done != 25) {
                        display_task_resume_for_times(1, -1, -1, true);
                    }
                }
#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 2 || defined(DEBUG))
                task_memory_info(__func__);
#endif
            default:
                break;
        }
    }
#endif
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
    ILOG(TAG, "[%s] %s", __FUNCTION__, key);
    if(strcmp(key, "screen_rotation")==0 || strcmp(key, "board_logo")==0||strcmp(key, "sail_logo")==0||strcmp(key, "speed_unit")==0 || strcmp(key, "sleep_info")==0) {
        g_context_rtc_add_config(&m_context_rtc, m_app_ctx.config);
    }
#if defined(CONFIG_DISPLAY_ENABLED)
    if(strcmp(key, "screen_rotation")==0) {
        display_set_rotation(m_app_ctx.config->screen.screen_rotation);
        if(!m_app_ctx.screen_auto_refresh){
            display_task_resume_for_times(1, -1, -1, false);
        }
    }
#if !defined(CONFIG_LCD_IS_EPD)
    if(strcmp(key, "screen_brightness")==0) {
        display_drv_bl_set(m_app_ctx.config->screen_brightness);
    }
#endif
#endif
}

static void ctx_load_cb() {
    ILOG(TAG, "[%s]", __FUNCTION__);
    vfs_select_part();
#if defined(CONFIG_GPS_LOG_ENABLED)
    log_config_init();
#endif
#if defined(CONFIG_LOGGER_HTTP_ENABLED)
    http_rest_init(CONFIG_WEB_APP_PATH);
#endif
    m_app_ctx.config = config_new();
    m_app_ctx.config->config_changed_screen_cb = config_changed_cb;
    g_context_defaults(&m_context);
    m_context.config = m_app_ctx.config;
    if (!m_context.gps.Gps_fields_OK) {
#if defined(CONFIG_GPS_LOG_ENABLED)
        gps_init(&m_context.gps);
#endif
    }
    delay_ms(50);

    config_load_json(m_app_ctx.config);
#if defined(CONFIG_DISPLAY_ENABLED) && !defined(CONFIG_LCD_IS_EPD)
    if(m_context_rtc.RTC_screen_brightness != m_app_ctx.config->screen_brightness)
        display_drv_bl_set(m_app_ctx.config->screen_brightness);
#endif
    g_context_rtc_add_config(&m_context_rtc, m_app_ctx.config);
#if defined(CONFIG_DISPLAY_ENABLED)
    if(display_drv_get_rotation() != m_context_rtc.RTC_screen_rotation){
        display_set_rotation(m_context_rtc.RTC_screen_rotation);
    }
#endif
    g_context_add_config(&m_context, m_app_ctx.config);
    config_fix_values(m_app_ctx.config);
    // g_context_ubx_add_config(&m_context, 0);
    // log_config_add_config(m_context.gps.log_config, m_app_ctx.config);
    m_app_ctx.config_initialized = 1;
#if defined(CONFIG_DISPLAY_ENABLED)
    if(!m_app_ctx.screen_auto_refresh){
       display_task_resume_for_times(1, -1, -1, false);
    }
#endif
}

static void setup(void) {
    DMEAS_START();
    m_app_ctx.app_mode = APP_MODE_BOOT;
    int ret = 0;

#if defined(CONFIG_IDF_TARGET_ESP32S3)
    init_power();
#endif

    events_init();
#if defined(CONFIG_LOGGER_ADC_ENABLED)
    init_adc();
#endif
    delay_ms(50);
    update_bat(0);
    
#if defined(CONFIG_LOGGER_USE_WDT)
    init_watchdog();
#endif
    init_rtc();
    m_app_ctx.screen_auto_refresh = m_context_rtc.RTC_screen_auto_refresh;
#if defined(CONFIG_DISPLAY_ENABLED)
    lcd_init();
    display_set_rotation(m_context_rtc.RTC_screen_rotation);
#if !defined(CONFIG_LCD_IS_EPD)
    display_drv_bl_set(m_context_rtc.RTC_screen_brightness==-1 ? SCR_DEFAULT_BRIGHTNESS : m_context_rtc.RTC_screen_brightness);
#endif
    display_task_start();

    if(!m_app_ctx.screen_auto_refresh){
       display_task_pause();
    }
    delay_ms(50);
#endif

    wakeup_init();  // Print the wakeup reason for ESP32, go back to sleep is timer is wake-up source !
     
    delay_ms(50);
#if defined(CONFIG_LOGGER_BUTTON_ENABLED)
    init_button();
    const esp_timer_create_args_t button_timer_args = {
        .callback = &button_timer_cb,
        .name = "btn_tmr",
        .arg = 0
    };
    ESP_ERROR_CHECK(esp_timer_create(&button_timer_args, &button_timer));
    delay_ms(50);
#endif

    vfs_init();

    delay_ms(50);
#if defined(CONFIG_DISPLAY_ENABLED)
    if(!m_app_ctx.screen_auto_refresh){
       display_task_resume_for_times(3, 0, 1, true);
    }
#endif
    // appstage 1, structures initialized, start gps.
    delay_ms(50);
    ret += 50;
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
            if(m_app_ctx.low_bat_countdown) {
                millis = get_millis();
                if(millis > m_app_ctx.low_bat_countdown) bat_timeout = 100;
                ESP_LOGW(TAG, "[%s] %lu low bat count:%d, seconds left: %lu", __FUNCTION__, loops, m_context.low_bat_count, (bat_timeout == 100 ? 0 : (m_app_ctx.low_bat_countdown-millis)));
                if(!bat_timeout && m_context.low_bat_count==LOW_BAT_TRIGGER) bat_timeout = 1;
                if(m_context.low_bat_count == LOW_BAT_TRIGGER+2) m_context.low_bat_count=0; // increase low bat count
                else m_context.low_bat_count++;
            }
            update_bat(0);
        }
        if (m_context.request_restart) {
            m_app_ctx.app_mode = APP_MODE_RESTART;
            m_context.request_restart = 1;
        }
        else if (m_context.request_shutdown || (m_app_ctx.low_bat_countdown && bat_timeout==100)) {
            m_app_ctx.app_mode = APP_MODE_SHUT_DOWN;
            m_context.request_shutdown = 1;
        }
        if(m_context.request_shutdown || m_context.request_restart || bat_timeout) {
#if defined(CONFIG_DISPLAY_ENABLED)
            if(!m_app_ctx.screen_auto_refresh && display_task_is_paused()) {
                display_task_resume_for_times(1, -1, -1, false); // one partial refresh
            }
#endif
            m_context.request_shutdown = 0;
            m_context.request_restart = 0;
        }
        if(!m_app_ctx.config && m_context.sdOK) {
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
#if defined(CONFIG_LOGGER_USE_WDT)
    run_wdt_loop = false;
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
#if !CONFIG_ESP_TASK_WDT_INIT
    esp_task_wdt_deinit();
#endif
#endif
#if defined(CONFIG_BMX_ENABLE)
    deinit_bmx();
#endif
#if defined(CONFIG_GPS_LOG_ENABLED)
    gps_uninit();
#endif
    vfs_uninit();
    // esp_timer_stop(screen_periodic_timer);
#if defined(CONFIG_LOGGER_BUTTON_ENABLED)
    esp_timer_stop(button_timer);
#endif
#if defined(CONFIG_DISPLAY_ENABLED)
    lcd_uninit();
#endif
    config_delete(m_app_ctx.config);
#ifdef CONFIG_UBLOX_ENABLED
    ubx_config_delete(m_context.gps.ubx_device);
#endif
    events_uninit();
}
