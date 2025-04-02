
#include "private.h"

#if defined(CONFIG_LOGGER_BUTTON_ENABLED)

#include "button.h"
#include "button_events.h"
#include "context.h"
#include "logger_config.h"
#if defined(CONFIG_GPS_LOG_ENABLED)
#include "gps_user_cfg.h"
#include "dstat_screens.h"
#endif
#ifdef CONFIG_LOGGER_WIFI_ENABLED
#include "logger_wifi.h"
#endif
#ifdef CONFIG_UBLOX_ENABLED
#include "ubx.h"
#endif
#ifdef CONFIG_DISPLAY_ENABLED
#include "driver_vendor.h"
#include "display.h"
#endif

static const char *TAG = "hid_button";
extern struct context_s m_context;
extern struct context_rtc_s m_context_rtc;
#ifdef CONFIG_LOGGER_WIFI_ENABLED
extern struct m_wifi_context wifi_context;
#endif
static esp_timer_handle_t button_timer = 0;
static uint8_t button_clicks = 0;
#define L_CFG_GROUP_FIELDS 4
// 200ms before exec cb
#if (defined(CONFIG_DISPLAY_DRIVER_ST7789) || defined(CONFIG_DISPLAY_DRIVER_QEMU))
#define BUTTON_CB_WAIT_BEFORE 300000U
#else
#define BUTTON_CB_WAIT_BEFORE 210000U
#endif

static void button_timer_cb(void *arg) {
    ILOG(TAG, "[%s]", __func__);
#if (defined(CONFIG_UBLOX_ENABLED) && defined(CONFIG_GPS_LOG_ENABLED))
    ubx_config_t *ubx_dev = m_context.gps.ubx_device;
    const ubx_hw_t hw_type = ubx_dev->rtc_conf->hw_type;
#endif
    if(button_clicks == 1) {
#if (C_LOG_LEVEL < 2)
        ILOG(TAG, "[%s] Button single click arrived, %s.", __func__, m_app_ctx.button_press_mode == 3 ? "lllong" : m_app_ctx.button_press_mode == 2 ? "llong" : m_app_ctx.button_press_mode == 1 ? "long" : "short");
#endif
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
#if (C_LOG_LEVEL < 2)
                ILOG(TAG, "[%s] settings new screen requested %d", __func__, 1);
#endif
                    if(m_app_ctx.cfg_screen >= L_CFG_GROUP_FIELDS-1)
                        m_app_ctx.cfg_screen = 0;
                    else
                        ++m_app_ctx.cfg_screen;
            } else if (m_app_ctx.cur_screen == CUR_SCREEN_FW_UPDATE){
#if (C_LOG_LEVEL < 2)
                ILOG(TAG, "[%s] fw update choice saved %d", __func__, 1);
#endif
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
#if (C_LOG_LEVEL < 2)
                ILOG(TAG, "[%s] fw update next choice %d", __func__, 1);
#endif
                    if(m_app_ctx.fw_update_screen >= config_fw_update_item_count-1)
                        m_app_ctx.fw_update_screen = 0;
                    else
                        ++m_app_ctx.fw_update_screen;
                goto refresh;
            }
            else if(m_app_ctx.cur_screen == CUR_SCREEN_SETTINGS) {
#if (C_LOG_LEVEL < 2)
                ILOG(TAG, "[%s] settings next requested %d", __func__, 1);
#endif
                if(m_app_ctx.cfg_screen == CFG_GROUP_GPS) {
                    if(m_app_ctx.gps_cfg_item < (CFG_GPS_ITEM_BASE))
                        m_app_ctx.gps_cfg_item = CFG_GPS_ITEM_BASE;
                    else if(++m_app_ctx.gps_cfg_item >= (CFG_GPS_ITEM_BASE + gps_user_cfg_item_count))
                        m_app_ctx.gps_cfg_item = CFG_GPS_ITEM_BASE;
#if (C_LOG_LEVEL < 2)
                    ILOG(TAG, "[%s] gps settings next requested %hhu", __func__, m_app_ctx.gps_cfg_item);
#endif
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
#if (C_LOG_LEVEL < 2)
                   ILOG(TAG, "[%s] gps info next screen requested, cur: %hhu", __func__, m_context.config->screen.speed_field);
#endif
                    m_context.config->screen.speed_field++;
                    if (m_context.config->screen.speed_field >= config_speed_field_item_count)
                        m_context.config->screen.speed_field = 1;
                    m_context.Field_choice = 1;
                }
                else if(m_app_ctx.cur_screen==CUR_SCREEN_GPS_STATS) {
                    if(++m_context.stat_screen_cur >= get_stat_screens_count()) m_context.stat_screen_cur = 0;
#if (C_LOG_LEVEL < 2)
                    ILOG(TAG, "[%s] next screen requested, cur: %hhu", __func__, m_context.stat_screen_cur);
#endif
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
#if (C_LOG_LEVEL < 2)
        ILOG(TAG, "[%s] Button double click arrived, %s", __func__,  m_app_ctx.button_press_mode == 3 ? "lllong" :  m_app_ctx.button_press_mode == 2 ? "llong" :  m_app_ctx.button_press_mode == 1 ? "long" : "short");
#endif
        if (m_app_ctx.app_mode == APP_MODE_GPS) {
            if(m_app_ctx.next_screen == CUR_SCREEN_GPS_INFO || m_app_ctx.cur_screen == CUR_SCREEN_GPS_INFO) {
#if (C_LOG_LEVEL < 2)
                ILOG(TAG, "[%s] setting screen requested", __func__);
#endif
                m_app_ctx.next_screen = CUR_SCREEN_SETTINGS;
            }
            else if(m_app_ctx.next_screen==CUR_SCREEN_NONE) {
#if (C_LOG_LEVEL < 2)
                ILOG(TAG, "[%s] gps info screen requested", __func__);
#endif
                m_app_ctx.next_screen = CUR_SCREEN_GPS_INFO;
            }
            else if(m_app_ctx.next_screen==CUR_SCREEN_SETTINGS) {
#if (C_LOG_LEVEL < 2)
                ILOG(TAG, "[%s] gps_stats screen requested", __func__);
#endif
                m_app_ctx.next_screen = CUR_SCREEN_GPS_STATS;
            }
            else {
#if (C_LOG_LEVEL < 2)
                ILOG(TAG, "[%s] default screen requested", __func__);
#endif
                m_app_ctx.next_screen = CUR_SCREEN_NONE;
            }
        }
#if defined (CONFIG_LOGGER_WIFI_ENABLED)
        else if(m_app_ctx.app_mode == APP_MODE_WIFI) {
            wifi_sta_conf_sync();
            if(wifi_context.s_ap_connection && wifi_context.s_sta_connection) {
#if (C_LOG_LEVEL < 2)
                ILOG(TAG, "[%s] wifi ap mode requested", __func__);
#endif
                wifi_mode(0, 1); // wifi set station mode
            }
            else if(wifi_context.s_ap_connection && !wifi_context.s_sta_connection) {
#if (C_LOG_LEVEL < 2)
                ILOG(TAG, "[%s] wifi sta mode requested", __func__);
#endif
                wifi_mode(1, 0); // wifi set station mode
            }
            else {
#if (C_LOG_LEVEL < 2)
                ILOG(TAG, "[%s] wifi sta + ap mode requested", __func__);
#endif
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
#if (C_LOG_LEVEL < 2)
        ILOG(TAG, "[%s] Button triple click arrived, %s", __func__,  m_app_ctx.button_press_mode == 3 ? "lllong" :  m_app_ctx.button_press_mode == 2 ? "llong" :  m_app_ctx.button_press_mode == 1 ? "long" : "short");
#endif
#if (defined(CONFIG_UBLOX_ENABLED) && defined(CONFIG_GPS_LOG_ENABLED))
        if(!(m_app_ctx.app_mode == APP_MODE_GPS && m_app_ctx.next_screen == CUR_SCREEN_SETTINGS)) {
#if (C_LOG_LEVEL < 2)
            ILOG(TAG, "[%s] screen rotation change requested", __func__);
#endif
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
#if (C_LOG_LEVEL < 2)
                ILOG(TAG, "[%s] settings screen change requested", __func__);
#endif
                if(m_app_ctx.cfg_screen == CFG_GROUP_GPS) {
                    if(set_gps_cfg_item(m_app_ctx.gps_cfg_item)) {
#if (C_LOG_LEVEL < 2)
                        ILOG(TAG, "[%s] settings screen gps change saved", __func__);
#endif
                        // g_context_ubx_add_config(&m_context, ubx_dev);
                        // g_context_rtc_add_config(&m_context_rtc, m_context.config);
                        // m_app_ctx.ubx_restart_requested = 1;
                    }
                }
                else if(m_app_ctx.cfg_screen == CFG_GROUP_STAT_SCREENS) {
                    if(set_stat_screen_cfg_item(m_app_ctx.config, m_app_ctx.stat_screen_cfg_item)) {
#if (C_LOG_LEVEL < 2)
                        ILOG(TAG, "[%s] settings screen change requested", __func__);
#endif
                        g_context_add_config(&m_context, m_context.config);
                    }
                }
                else if(m_app_ctx.cfg_screen == CFG_GROUP_SCREEN) {
                    int changed = 0;
                    if((changed = (set_screen_cfg_item(m_app_ctx.config, m_app_ctx.screen_cfg_item)))) {
#if (C_LOG_LEVEL < 2)
                        ILOG(TAG, "[%s] settings screen change requested", __func__);
#endif
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
#if (C_LOG_LEVEL < 2)
                        ILOG(TAG, "[%s] settings fw change requested", __func__);
#endif
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
#if (C_LOG_LEVEL < 2)
            ILOG(TAG, "[%s] Button 4 click arrived", __func__);
#endif
#if !defined(CONFIG_LCD_IS_EPD)
            int changed = 0;
            if((changed = (set_screen_cfg_item(m_app_ctx.config, CGG_SCREEN_ITEM_BRIGHTNESS_POS)))) {
                g_context_rtc_add_config(&m_context_rtc, m_context.config);
#if defined(CONFIG_DISPLAY_ENABLED)
                if(changed == cfg_screen_brightness)
                    display_drv_bl_set(m_context_rtc.RTC_screen_brightness);
#endif
            }
#else
#if defined(CONFIG_DISPLAY_ENABLED)
            display_request_full_refresh(0);
            if(!m_app_ctx.screen_auto_refresh){
                display_task_resume_for_times(1, -1, -1, false);
            }
#endif
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
#if (C_LOG_LEVEL < 3)
    ILOG(TAG, "[%s] num: %d event: %s", __func__, ev, l_button_ev_list[ev]);
#endif
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
#if (C_LOG_LEVEL < 3)
            ILOG(TAG,"[%s] cancel timer, num: %d", __FUNCTION__, num);
#endif
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
#if (C_LOG_LEVEL < 2)
            ILOG(TAG, "[%s] Button num: %d lllong press detected, time: %lld, restart requested.", __func__, num, time);
#endif
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

void init_button() {
    ILOG(TAG, "[%s]", __func__);
    button_init();
    btns[0].cb = button_cb;
#if defined(CONFIG_LOGGER_BUTTON_GPIO_1)
    btns[1].cb = button_cb;
#endif
    const esp_timer_create_args_t button_timer_args = {
        .callback = &button_timer_cb,
        .name = "btn_tmr",
        .arg = 0
    };
    ESP_ERROR_CHECK(esp_timer_create(&button_timer_args, &button_timer));
}
void deinit_button() {
    ILOG(TAG, "[%s]", __func__);
    esp_timer_stop(button_timer);
    button_deinit();
}
#endif

