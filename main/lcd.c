
#include <string.h>

#include <esp_log.h>
#include <adc.h>

#include "display.h"
#include "private.h"
#if defined(CONFIG_DISPLAY_USE_EPD)
#include <epdg.h>
#include <gfxfont.h>
#else
#include "driver_vendor.h"
#endif

#if defined(CONFIG_BMX_ENABLE)
#include "bmx280.h"
#include "bmx.h"
#endif
#include "button.h"
#include "context.h"
#include "dstat_screens.h"
#include "logger_config.h"
#include "str.h"
#include "gps_data.h"
#include "ubx.h"
#include "wifi.h"
#if defined(STATUS_PANEL_V1)
// #include "ui.h"
#else
#include "lv_comp_statusbar.h"
#endif

#define UI_INFO_SCREEN ui_InfoScreen
#define UI_SPEED_SCREEN ui_SpeedScreen
#define UI_STATS_SCREEN ui_StatsScreen
#define UI_SPEED_MAIN_SLOT UI_COMP_BIGDATAPANEL_DATALABEL

#define UI_LOGO_IMG espidf_logo_v2_48px

extern struct context_s m_context;
extern struct context_rtc_s m_context_rtc;
//extern struct UBXMessage ubxMessage;
extern struct m_wifi_context wifi_context;
static void lcd_ui_start();

esp_lcd_panel_handle_t dspl = 0;
TaskHandle_t display_task_handle = 0;

extern stat_screen_t sc_screens[];

struct display_state_s {
    bool initialized;
    uint16_t update_delay;
};

struct display_priv_s {
    struct display_state_s state;
    int16_t displayHeight;
    int16_t displayWidth;
    uint8_t test_field, test_font;
    bool displayOK;
};

struct display_priv_s display_priv = {{false}, 0, 0, 0, 0, false};

static const char *TAG = "display";
//static lcd_g_t screen;
TIMER_INIT

//lv_obj_t *lv_statusbar=0;

static esp_err_t reset_display_state(struct display_state_s *display_state) {
    LOGR;
    display_state->initialized = 0;
    // memset(&display_state->speed_1_fonts, 0, sizeof(speed_num_font_t));
    // memset(&display_state->speed_2_fonts, 0, sizeof(speed_num_font_t));
    return ESP_OK;
}

static uint32_t _boot_screen(const struct display_s *me) {
    TIMER_S
    if (_lvgl_lock(100)) {
        //driver_st7789_bl_set(75);
        // lv_scr_load(UI_INFO_SCREEN);

        if(m_context.low_bat_count > 10) {
            showLowBatScreen();
        } else {
            showBootScreen("Booting...");
        }
        _lvgl_unlock();
    }
    TIMER_E
    return 1500;
}
static uint32_t _off_screen(const struct display_s *me, int choice) {
    TIMER_S
    int ret = 3000;
    if (_lvgl_lock(-1)) {
        uint32_t milli = get_millis();
        float session_time = (milli - m_context.gps.start_logging_millis) / 1000;
        const char *title = m_context.request_restart ? "Reboot device" : m_context.Shut_down_Save_session ? "Saving session" : "Going to sleep";
        if (session_time > 0) {
            ESP_LOGD(TAG, "session time: %.2f s", session_time);
        }
        // lv_scr_load(UI_INFO_SCREEN);

        if(m_context.low_bat_count > 10) {
            showLowBatScreen();
        } else {
            showBootScreen(title);
        }
        _lvgl_unlock();
    }
    TIMER_E
    return ret;
}

typedef struct sleep_scr_s {
    float *data;
    const char *info;
} sleep_scr_t;

static struct sleep_scr_s sleep_scr_info_fields[2][6] = {
    {
        {&m_context_rtc.RTC_avg_10s, "AV:"},
        {&m_context_rtc.RTC_R1_10s, "R1:"},
        {&m_context_rtc.RTC_R2_10s, "R2:"},
        {&m_context_rtc.RTC_R3_10s, "R3:"},
        {&m_context_rtc.RTC_R4_10s, "R4:"},
        {&m_context_rtc.RTC_R5_10s, "R5:"}
    },
    {
        {&m_context_rtc.RTC_max_2s, "2sec:"},
        {&m_context_rtc.RTC_1h, "1h:"},
        {&m_context_rtc.RTC_500m, "500m:"},
        {&m_context_rtc.RTC_mile, "NM:"},
        {&m_context_rtc.RTC_distance, "Dist:"},
        {&m_context_rtc.RTC_alp, "Alfa:"}
    }
};

static void statusbar_update();

static uint32_t _sleep_screen(const struct display_s *me, int choice) {
    TIMER_S
    char tmp[24], *p = tmp;
    lv_label_t *panel;
    if (_lvgl_lock(-1)) {
        showSleepScreen();
        statusbar_update();
        for(int i = 0; i < 6; i++) {
            for(int j = 0; j < 2; j++) {
                f2_to_char(*sleep_scr_info_fields[j][i].data, p);
                lv_label_set_text(ui_sleep_screen.cells[i][j].title, p);
                lv_label_set_text(ui_sleep_screen.cells[i][j].info, sleep_scr_info_fields[j][i].info);
            }
        }
        // f2_to_char(m_context_rtc.RTC_avg_10s, p);
        // lv_label_set_text(ui_sleep_screen.cells[0][0].title,p);
        // lv_label_set_text(ui_sleep_screen.cells[0][0].info, "AV:");
        // f2_to_char(m_context_rtc.RTC_R1_10s, p);
        // lv_label_set_text(ui_sleep_screen.cells[1][0].title,p);
        // lv_label_set_text(ui_sleep_screen.cells[1][0].info, "R1:");
        // f2_to_char(m_context_rtc.RTC_R2_10s, p);
        // lv_label_set_text(ui_sleep_screen.cells[2][0].title,p);
        // lv_label_set_text(ui_sleep_screen.cells[2][0].info, "R2:");
        // f2_to_char(m_context_rtc.RTC_R3_10s, p);
        // lv_label_set_text(ui_sleep_screen.cells[3][0].title,p);
        // lv_label_set_text(ui_sleep_screen.cells[3][0].info, "R3:");
        // f2_to_char(m_context_rtc.RTC_R4_10s, p);
        // lv_label_set_text(ui_sleep_screen.cells[4][0].title,p);
        // lv_label_set_text(ui_sleep_screen.cells[4][0].info, "R4:");
        // f2_to_char(m_context_rtc.RTC_R5_10s, p);
        // lv_label_set_text(ui_sleep_screen.cells[5][0].title,p);
        // lv_label_set_text(ui_sleep_screen.cells[5][0].info, "R5:");
        
        // f2_to_char(m_context_rtc.RTC_max_2s, p);
        // lv_label_set_text(ui_sleep_screen.cells[0][1].title,p);
        // lv_label_set_text(ui_sleep_screen.cells[0][1].info, "2sec:");
        // f2_to_char(m_context_rtc.RTC_1h, p);
        // lv_label_set_text(ui_sleep_screen.cells[1][1].title,p);
        // lv_label_set_text(ui_sleep_screen.cells[1][1].info, "1h:");
        // f2_to_char(m_context_rtc.RTC_500m, p);
        // lv_label_set_text(ui_sleep_screen.cells[2][1].title,p);
        // lv_label_set_text(ui_sleep_screen.cells[2][1].info, "500m:");
        // f2_to_char(m_context_rtc.RTC_mile, p);
        // lv_label_set_text(ui_sleep_screen.cells[3][1].title,p);
        // lv_label_set_text(ui_sleep_screen.cells[3][1].info, "NM:");
        // f2_to_char(m_context_rtc.RTC_distance, p);
        // lv_label_set_text(ui_sleep_screen.cells[4][1].title,p);
        // lv_label_set_text(ui_sleep_screen.cells[4][1].info, "Dist:");
        // f2_to_char(m_context_rtc.RTC_alp, p);
        // lv_label_set_text(ui_sleep_screen.cells[5][1].title,p);
        // lv_label_set_text(ui_sleep_screen.cells[5][1].info, "ALFA:");
        
        lv_label_set_text(ui_sleep_screen.myid, m_context_rtc.RTC_Sleep_txt);

        _lvgl_unlock();
    }
    TIMER_E
    return 100;
}

static uint8_t screen_num = 0;

static size_t bat_to_char(char *str, uint8_t full) {
    char *p = str + uint_to_char(full, str);
    *p++ = '%';
    *p = 0;
    return p - str;
}

#if defined(CONFIG_BMX_ENABLE)
extern bmx280_t *bmx280;
#endif

static float last_temp=0;

static size_t temp_to_char(char *str) {
#if defined(CONFIG_BMX_ENABLE)
    TIMER_S
    float t=0;
    float p=0, h=0;
    bmx_readings_f(&t, &p, &h);
    if(t==0 && p==0 && h==0)
        goto end;
    if(t==last_temp) {
        goto end;
    }
    else {
        last_temp = t;
    }
    size_t len = f1_to_char(t, str);
    char *pt = str + len;
    *pt = 176;
    ++pt;
    ++len;
    *pt = 'C';
    *(++pt) = 0;
    TIMER_E
    return ++len;
    end:
#endif
    *str=0;
    return 0;
}

static int low_speed_seconds = 0;
static esp_err_t speed_info_bar_update() {  // info bar when config->speed_large_font is 1 or 0
    const float calspd = m_context_rtc.RTC_calibration_speed;
    const logger_config_t *config = m_context.config;
    uint8_t field = m_context.config->speed_field;          // default is in config.txt
    uint8_t bar_max = 240;                                  // 240 pixels is volledige bar
    uint16_t bar_length = config->bar_length * 1000 / bar_max;  // default 100% length = 1852 m
    uint8_t font_size = m_context.config->speed_large_font;

    if (config->speed_field == 1) {  // only switch if config.field==1 !!!
        if (((int)(m_context.gps.Ublox.total_distance / 1000000) % 10 == 0) && (m_context.gps.Ublox.alfa_distance / 1000 > 1000))
            field = 5;  // indien x*10km, totale afstand laten zien
        // if(m_context.gps.S10.s_max_speed<(m_context.gps.S10.display_speed[5]*0.95))
        //     field=8;//if run slower dan 95% of slowest run, show 1h result
        if ((m_context.gps.Ublox.alfa_distance / 1000 < 350) && (m_context.gps.alfa_window < 100))
            field = 3;  // first 350 m after gibe  alfa screen !!
        if (m_context.gps.Ublox.alfa_distance / 1000 > config->bar_length)
            field = 4;  // run longer dan 1852 m, NM scherm !!
    } else if (config->speed_field == 2) { // show Nautical Mile status
        if (m_context.gps.Ublox.run_distance / 1000 > config->bar_length)
            field = 4;  // if run longer dan 1852 m, NM scherm !!
    } else if (config->speed_field == 7) { // show alpha status
        if ((m_context.gps.Ublox.alfa_distance / 1000 < 350) && (m_context.gps.alfa_window < 100))
            field = 3;  // first 350 m after gibe  alfa screen !!
        else
            field = 7;
    } else if (config->speed_field == 8) { // show 1 hour status
        field = 8;
    } else if (config->speed_field == 9) {  // 1 hour default, but first alfa, and if good run, last run
        field = 8;
        if (m_context.gps.Ublox.alfa_distance / 1000 > config->bar_length)
            field = 4;  // run longer dan 1852 m, NM scherm !!
        if (m_context.gps.S10.s_max_speed > m_context.gps.S10.display_speed[5])
            field = 1;  // if run faster then slowest run, show AVG & run
        if ((m_context.gps.Ublox.alfa_distance / 1000 < 350) && (m_context.gps.alfa_window < 100))
            field = 3;  // first 350 m after gibe  alfa screen !!
    }

    size_t len;

    double s[] = {0, 0};
    const char *var[] = {0, 0};
    char val[][24] = {{0}, {0}}, *p;
    if (!m_context.gps.ublox_config->ready || !m_context.gps.ublox_config->signal_ok) {
        memcpy(val[0], "-.--", 4);
        memcpy(val[1], "-.--", 4);
        goto topoint;
    }
    if(m_context.gps.S2.avg_s < 1000) { // 1ms = 3.6km/h
        memcpy(val[0], "0.00", 4);
        memcpy(val[1], "0.00", 4);
        goto topoint;
    }
    // double s1 = 0, s2 = 0;
    else if (field <= 2 || display_priv.test_field == 2) { // 10 seconds stats
        s[0] = m_context.gps.S10.s_max_speed * calspd;
        s[1] = m_context.gps.S10.avg_5runs * calspd;
        if(s[0] >= 100 || s[1] >= 100) {
            f1_to_char(s[0], val[0]);
            f1_to_char(s[1], val[1]);
        }
        else {
            f2_to_char(s[0], val[0]);
            f2_to_char(s[1], val[1]);
        }
        topoint:
        if (font_size == 0) {
            var[0] = "Run";
            var[1] = "Avg";
        } else {
            var[0] = "R";
            var[1] = "A";
        }
    }

    // First 250m after jibe, if Window>99 m : Window and Exit
    // Between 250m and 400m after jibe : Result Alfa (speed or MISS)
    // Between 400m and 1852m after jibe : Actual Run + AVG
    // More then 1852m : NM actual speed and NM Best speed

    else if ((field == 3 || display_priv.test_field == 3)) {
        bar_length = 250 * 1000 / bar_max;  // full bar length with Alfa = 250 meter
        if ((m_context.gps.alfa_window < 99) && (m_context.gps.Ublox.alfa_distance / 1000 < 255)) { // 250 meter na gijp
            if (m_context.gps.alfa_exit > 99)
                m_context.gps.alfa_exit = 99;  // begrenzen alfa_exit...
            var[0] = "Gate";
            len = f_to_char(m_context.gps.alfa_window, val[0], 0);
            var[1] = "Ex";
            len = f_to_char(m_context.gps.alfa_exit, val[1], 0);
        } else { // alfa speed stats
            s[0] = m_context.gps.A500.display_max_speed * calspd;
            s[1] = m_context.gps.A500.alfa_speed_max * calspd;
            if (s[0] > 100 || s[1] > 100) {
                if (font_size == 0)
                    f1_to_char(s[0], val[0]); // last alpha
                else
                    val[0][0] = 0;
                if (s[1] > 1) {
                    f1_to_char(s[1], val[1]); // best alpha
                } else {
                    goto tonpoint;
                }
            } else {
                if (font_size == 0)
                    f2_to_char(s[0], val[0]);
                else
                    *val[0] = 0;
                if (s[1] > 1) {
                    f2_to_char(s[1], val[1]);
                } else {
                tonpoint:
                    memcpy(val[1], "0.00", 4);
                    val[1][4]=0;
                }
            }
            if (font_size == 0) {
                var[0] = "Alfa";
                var[1] = "Amax";  // nieuwe alfa laatste gijp or MISSED !!!!
            } else {
                // best Alfa from session on 500 m !!
                var[0] = "Alfa=";
                var[1] = "";
            }
        }
    } else if (field == 4 || display_priv.test_field == 4) { // nautical mile
        s[0] = m_context.gps.M1852.m_max_speed * calspd;
        s[1] = m_context.gps.M1852.display_max_speed * calspd;
        if (s[0] > 100 || s[1] > 100) {
            f1_to_char(s[0], val[0]);
            if (font_size == 0) 
                f1_to_char(s[1], val[1]);
            else
                *val[1] = 0;
        } else {
            f2_to_char(s[0], val[0]);
            if (font_size == 0)
                f2_to_char(s[1], val[1]);
            else
                *val[1] = 0;
        }
        if (font_size == 0) {
            var[0] = "NMa";  // Actuele nautical mile
            var[1] = "NM";
        } else {
            var[0] = "NM=";  // Actuele nautical mile
            var[1] = "";
        }
    } else if (field == 5 || display_priv.test_field == 5) { // total distance
        s[0] = m_context.gps.Ublox.total_distance / 1000000;
        var[0] = "Dst";
        len = f1_to_char(s[0], val[0]);
        if (s[0] >= 100){
            p=val[0]+len;
            memcpy(p, " km", 3);
            *(p+3)=0;
        }
    } else if (field == 6 || display_priv.test_field == 6) { // 2 and 10 seconds stats
        s[0] = m_context.gps.S2.display_max_speed * calspd;
        s[1] = m_context.gps.S10.display_max_speed * calspd;
        if (s[0] > 100 || s[1] > 100) {
            f1_to_char(s[0], val[0]);
            f1_to_char(s[1], val[1]);
        } else {
            f2_to_char(s[0], val[0]);
            f2_to_char(s[1], val[1]);
        }
        var[0] = "2S";
        var[1] = "10S";
    } else if (field == 7 || display_priv.test_field == 7) { // 30 minutes stats
        s[0] = m_context.gps.S1800.avg_s * calspd;
        s[1] = m_context.gps.S1800.display_max_speed * calspd;
        if (s[0] > 100 || s[1] > 100) {
            f1_to_char(s[0], val[0]);
            f1_to_char(s[1], val[1]);
        } else {
            f2_to_char(s[0], val[0]);
            f2_to_char(s[1], val[1]);
        }
        var[0] = ".5hA";
        var[1] = ".5hB";
    } else if (field == 8 || display_priv.test_field == 8) { // 60 minutes stats
        s[0] = m_context.gps.S3600.avg_s * calspd;
        s[1] = m_context.gps.S3600.display_max_speed * calspd;
        if (s[0] > 100 || s[1] > 100) {
            f1_to_char(s[0], val[0]);
            f1_to_char(s[1], val[1]);
        } else {
            f2_to_char(s[0], val[0]);
            f2_to_char(s[1], val[1]);
        }
        var[0] = "1hA";
        var[1] = "1hB";
    }

    // col 1
    if (var[0]) {
        // DISPLAY_PRINT(var[0]);
        lv_label_set_text(ui_speed_screen.cells[0][0].info, var[0]);
    }
    if (*val[0]) {
        // DISPLAY_PRINT(val[0]);  // 1 // last 10s max from run
        lv_label_set_text(ui_speed_screen.cells[0][0].title, val[0]);
    }
    // col 2
    if (var[1]) {
        // DISPLAY_PRINT(var[1]);
        lv_label_set_text(ui_speed_screen.cells[0][1].info, var[1]);
    }
    if (*val[1]) {
        // DISPLAY_PRINT(val[1]);  // 1
        lv_label_set_text(ui_speed_screen.cells[0][1].title, val[1]);
    }

    int run_rectangle_length;
    int32_t millis = get_millis();
    int log_seconds = (millis - m_context.gps.start_logging_millis) / 1000;  // aantal seconden sinds loggen is gestart
    if (m_context.gps.S10.avg_s > 2000) {  // if the speed is higher then 2000 mm/s, reset the counter
        low_speed_seconds = 0;
    }
    low_speed_seconds++;
    if (low_speed_seconds > 120) { // bar will be reset if the 10s speed drops under 2m/s for more then 120 s !!!!
        m_context.gps.start_logging_millis = millis;
    } 
    run_rectangle_length = (m_context.gps.Ublox.alfa_distance / bar_length);  // 240 pixels is volledige bar, m_context.gps.ublox.alfa_distance zijn mm
    if (field == 7) {
        run_rectangle_length = log_seconds * 240 / 1800;
        if (log_seconds > 1800) {
            m_context.gps.start_logging_millis = millis;
        }
    }  // 30 minutes = full bar
    else if (field == 8) {
        run_rectangle_length = log_seconds * 240 / 3600;
        if (log_seconds > 3600) {
            m_context.gps.start_logging_millis = millis;
        }
    }  // 60 minutes = full bar
    // if (bar_length) {
        // screen.op->fillRect(&screen, offset, DISPLAY_TOP_PAD + bar_position, run_rectangle_length, SPEED_INFO_BOTTOM_HEIGHT, FG_COLOR);  // balk voor run_distance weer te geven...
        lv_bar_set_value(ui_speed_screen.bar, run_rectangle_length, 0);
    // }
    return ESP_OK;
}

static uint32_t count = 0;
static int old_screen = 0;
//static int low_speed_seconds = 0;
static int16_t gps_image_angle = 0;
static uint8_t last_hr=0, last_min=0;

void statusbar_time_cb(lv_timer_t *timer) {
#if defined(STATUS_PANEL_V1)
    ui_status_panel_t * statusbar = &ui_status_panel;
    if(!statusbar->parent) {
        return;
    }
#else
    lv_statusbar_t * statusbar = (lv_statusbar_t *)ui_StatusPanel;
#endif
    char tmp[24], *p = tmp;
    lv_obj_t *panel;
    if ((panel = statusbar->time_label)) {
        if (lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN))
            lv_obj_clear_flag(panel, LV_OBJ_FLAG_HIDDEN);
        if (statusbar->viewmode==0 || statusbar->viewmode == 2) {
            struct tm tms;
            memset(&tms, 0, sizeof(struct tm));
            getLocalTime(&tms, 0);
            if (tms.tm_year != 0 && tms.tm_mday != 0) {
                if (tms.tm_hour != last_hr || tms.tm_min != last_min) {
                    last_hr = tms.tm_hour;
                    last_min = tms.tm_min;
                    time_to_char_hm(last_hr, last_min, p);
                    lv_label_set_text(panel, p);
                }
            }
        } else {
            p = tmp;
            p += time_to_char_hm(m_context_rtc.RTC_hour, m_context_rtc.RTC_min, p);
            *p = ' ';
            ++p;
            p += date_to_char(m_context_rtc.RTC_day, m_context_rtc.RTC_month, m_context_rtc.RTC_year, 0, p);
            *p = 0;
            p = tmp;
            lv_label_set_text(panel, p);
        }
    }
}

static void statusbar_temp_cb(lv_timer_t *timer) {
    #if defined(STATUS_PANEL_V1)
    ui_status_panel_t * statusbar = &ui_status_panel;
    if(!statusbar->parent) {
        return;
    }
#else
    lv_statusbar_t * statusbar = (lv_statusbar_t *)ui_StatusPanel;
#endif
    char tmp[24], *p = tmp;
    lv_obj_t *panel;
    if ((panel = statusbar->temp_label)) {
#if defined(CONFIG_BMX_ENABLE)
        temp_to_char(p);
        if (*p) {
            if (lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN))
                lv_obj_clear_flag(panel, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(panel, p);
        }
#else
        if(!lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_add_flag(panel, LV_OBJ_FLAG_HIDDEN);
        }
#endif
    }
}

static void statusbar_bat_cb(lv_timer_t *timer) {
    #if defined(STATUS_PANEL_V1)
    ui_status_panel_t * statusbar = &ui_status_panel;
    if(!statusbar->parent) {
        return;
    }
#else
    lv_statusbar_t * statusbar = (lv_statusbar_t *)ui_StatusPanel;
#endif
    char tmp[24], *p = tmp;
    const char *r;
    lv_obj_t *panel;
    uint8_t full = m_context_rtc.RTC_voltage_bat > 4.8 ? 110 : m_context_rtc.RTC_voltage_bat >= 4.2 ? 101
                                                                                                    : calc_bat_perc_v(m_context_rtc.RTC_voltage_bat);
    if ((panel = statusbar->bat_label)) {
        if(full<100)
            *p++=' ';
        p += bat_to_char(p, full > 100 ? 100 : full);
        r = lv_label_get_text(panel);
        if(!r || memcmp(r,&tmp[0],4))
            lv_label_set_text(panel, tmp);
    }

    if ((panel = statusbar->bat_image)) {
        const char * s = full < 10 ? LV_SYMBOL_BATTERY_EMPTY : full < 20 ? LV_SYMBOL_BATTERY_1
                                                          : full < 60   ? LV_SYMBOL_BATTERY_2
                                                          : full < 101  ? LV_SYMBOL_BATTERY_FULL
                                                          : full < 110  ? LV_SYMBOL_CHARGE
                                                                        : LV_SYMBOL_USB;
        r = lv_label_get_text(panel);
        if(!r || memcmp(r, s, 3))
            lv_label_set_text(panel, r);
#if defined(CONFIG_DISPLAY_DRIVER_ST7789)
        lv_obj_set_style_text_color(panel, full>20 ? lv_color_hex(0xFFFFFF) : full>10 ? lv_color_hex(0xEECE44) : lv_color_hex(0xE32424), LV_PART_MAIN | LV_STATE_DEFAULT );
        lv_obj_set_style_text_opa(panel, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
#endif
    }
}

static bool sdblink = false;
static void statusbar_sdcard_cb(lv_timer_t *timer) {
    #if defined(STATUS_PANEL_V1)
    ui_status_panel_t * statusbar = &ui_status_panel;
    if(!statusbar->parent) {
        return;
    }
#else
    lv_statusbar_t * statusbar = (lv_statusbar_t *)ui_StatusPanel;
#endif
    char tmp[24], *p = tmp;
    lv_obj_t *panel;
    if ((panel = statusbar->sdcard_image)) {
        if (m_context.sdOK) {
#if defined(CONFIG_DISPLAY_DRIVER_ST7789)
            lv_obj_set_style_img_recolor(panel, lv_color_hex(0xA9B7B9), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
#endif
            if (lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_clear_flag(panel, LV_OBJ_FLAG_HIDDEN);
            }
        } else {
            if (sdblink == 0) {
                if (!lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN)) {
                    lv_obj_add_flag(panel, LV_OBJ_FLAG_HIDDEN);
                }
                sdblink = 1;
            } else {
                if (lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN)) {
                    lv_obj_clear_flag(panel, LV_OBJ_FLAG_HIDDEN);
                }
#if defined(CONFIG_DISPLAY_DRIVER_ST7789)
                lv_obj_set_style_img_recolor(panel, lv_color_hex(0xE32424), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_img_recolor_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
#endif
                sdblink = 0;
            }
        }
    }
}

static uint8_t gblink = 0;
static void statusbar_gps_cb(lv_timer_t *timer) {
#if defined(STATUS_PANEL_V1)
    ui_status_panel_t * statusbar = &ui_status_panel;
    if(!statusbar->parent) {
        return;
    }
#else
    lv_statusbar_t * statusbar = (lv_statusbar_t *)ui_StatusPanel;
#endif
    lv_obj_t *panel;
    if ((panel = statusbar->gps_image)) {
        char tmp[24]={0}, *p = tmp;
        if ((m_context.gps.ublox_config->ready && m_context.gps.ublox_config->signal_ok) || statusbar->viewmode == 2 ) {
            if (lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_clear_flag(panel, LV_OBJ_FLAG_HIDDEN);
            }
        } else {
            if (!lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_add_flag(panel, LV_OBJ_FLAG_HIDDEN);
            }
            return;
        }
        if(statusbar->viewmode==2) { 
            if (!m_context.gps.ublox_config->ready){
                memcpy(p, "gps fail", 8);
            }
            else if(!m_context.gps.ublox_config->signal_ok) {
                uint8_t qp = gblink;
                memcpy(p, "gps   ", 6);
                *(p+6)=0;
                p += 3;
                while(qp--) {
                    *p++ = '.';
                }
                gblink = gblink==3 ? 0 : gblink+1;
            }
            else {
                p += xltoa(m_context.gps.ublox_config->ubx_msg.navPvt.numSV, p);
                *p = 0;
            }
            p = lv_label_get_text(panel);
            if(!p || strcmp(p, &(tmp[0]))) {
                lv_label_set_text(panel, &(tmp[0]));
            }
       }
        else {
            p = lv_label_get_text(panel);
            if(!p || memcmp(p, LV_SYMBOL_GPS, 3)) {
                lv_label_set_text(panel, LV_SYMBOL_GPS);
            }
        }
        // if (gps->ublox_config->ubx_msg.navPvt.numSV >= 4) {
        //     lv_obj_set_style_img_recolor(panel, lv_color_hex(0xA9B7B9), LV_PART_MAIN | LV_STATE_DEFAULT);
        //     lv_obj_set_style_img_recolor_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        // } else if (m_context.gps.ublox_config->ready) {
        //     lv_obj_set_style_img_recolor_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        //     lv_obj_set_style_img_recolor(panel, lv_color_hex(0xEECE44), LV_PART_MAIN | LV_STATE_DEFAULT);
        // } else {
        //     if (gps->ublox_config->rtc_conf->hw_type == UBX_TYPE_UNKNOWN) {
        //         if (blink == 0) {
        //             lv_obj_add_flag(panel, LV_OBJ_FLAG_HIDDEN);
        //             blink = 1;
        //         } else {
        //             lv_obj_clear_flag(panel, LV_OBJ_FLAG_HIDDEN);
        //             lv_obj_set_style_img_recolor(panel, lv_color_hex(0xE32424), LV_PART_MAIN | LV_STATE_DEFAULT);
        //             lv_obj_set_style_img_recolor_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        //             blink = 0;
        //         }
        //     }
        // }
    }
}

static void statusbar_update() {
    // battery //
    statusbar_bat_cb(0);
    // // time //
    statusbar_time_cb(0);
    // // temp //
    statusbar_temp_cb(0);
    // // sdcard image //
    statusbar_sdcard_cb(0);
    // // gps/wifi image //
    statusbar_gps_cb(0);
}

static uint32_t _update_screen(const struct display_s *me, const screen_mode_t screen_mode, void *arg) {
    uint32_t ret = 100;
    if (_lvgl_lock(ret)) {
        // TIMER_S
        // char sz[64];
        logger_config_t *config = m_context.config;
        char tmp[24] = {0}, *p = tmp, tmpb[24]={0}, *pb = tmpb;
        bool is_gps_stat_screen = (screen_mode > 0 && screen_mode < 10);
        me->self->state.update_delay = 500;
        // ESP_LOGI(TAG, "update screen: mode:%" PRIu8 ", update nr:%lu", screen_mode, count);
        float gpsspd;
        int state = (int)arg;
        lv_obj_t *panel, *parent;
        stat_screen_t *sc_data = 0;
        //const char *title;
        switch (screen_mode) {
            case SCREEN_MODE_GPS_TROUBLE:
                //ui_flush_screens(&ui_init_screen.screen);
                showGpsTroubleScreen();
                break;
            case SCREEN_MODE_GPS_INIT:
            case SCREEN_MODE_GPS_READY:
                const char *gps = ubx_chip_str(m_context.gps.ublox_config);
                size_t gpslen = strlen(gps);
                const lv_img_dsc_t *img_src = 0;
                
                memcpy(p, gps, gpslen);
                p += gpslen;
                if(gpslen==7 && !strcmp(gps, "UNKNOWN")) {
                    img_src = &near_me_disabled_bold_48px;
                }
                else {
                    img_src = &near_me_bold_48px;
                    *p = '@';
                    ++p;
                    p += xltoa(m_context.config->sample_rate, p);
                    memcpy(p, "Hz", 2);
                    p += 2;
                }
                *p = 0;
                p = tmp;
                
                pb += f3_to_char(m_context_rtc.RTC_voltage_bat, pb);
                memcpy(pb, "V ", 2);
                pb += 2;
                pb += xltoa(m_context.gps.ublox_config->ubx_msg.navPvt.numSV, pb);
                memcpy(pb, "sat", 3);
                pb += 3;
                *pb = 0;
                pb=tmpb;
                //ui_flush_screens(&ui_info_screen.screen);
                showGpsScreen(p, pb, img_src, gps_image_angle);
                statusbar_update();

#if defined(CONFIG_DISPLAY_DRIVER_ST7789)
                if(screen_mode==SCREEN_MODE_GPS_INIT) {
                    lv_obj_set_style_img_recolor(ui_info_screen.info_img, lv_color_hex(0x162B2E), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_img_recolor_opa(ui_info_screen.info_img, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                else if(screen_mode==SCREEN_MODE_GPS_READY) {
                    //104951
                    lv_obj_set_style_img_recolor(ui_info_screen.info_img, lv_color_hex(0x104951), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_img_recolor_opa(ui_info_screen.info_img, 255, LV_PART_MAIN | LV_STATE_DEFAULT);               
                }
                else if(screen_mode==SCREEN_MODE_GPS_TROUBLE) {
                    lv_obj_set_style_img_recolor(ui_info_screen.info_img, lv_color_hex(0xE32424), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_img_recolor_opa(ui_info_screen.info_img, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                gps_image_angle += 225;
                if (gps_image_angle >= 3600)
                    gps_image_angle = 0;
#endif
                break;
            case SCREEN_MODE_PUSH:
                me->self->state.update_delay = 100;
                //ui_flush_screens(&ui_init_screen.screen);
                showPushScreen(state);
                break;
            case SCREEN_MODE_GO_SLEEP:
                ui_flush_screens(&ui_init_screen.screen);
                me->self->state.update_delay = _off_screen(me, 0);
                break;
            case SCREEN_MODE_BOOT:
                ui_flush_screens(&ui_init_screen.screen);
                me->self->state.update_delay = _boot_screen(me);
                break;
            case SCREEN_MODE_SPEED_1:
            link_for_screen_mode_speed_2:
                //ui_flush_screens(&ui_speed_screen.screen);
                showSpeedScreen();
                gpsspd = gps_last_speed_smoothed(2) * m_context_rtc.RTC_calibration_speed;
                if (!m_context.gps.ublox_config->ready || !m_context.gps.ublox_config->signal_ok) {
                    memcpy(p, "-.--", 4);
                    *(p+4) = 0;
                }
                else if(m_context.gps.S2.avg_s < 1000) {
                    f2_to_char(0.0, p);
                } else if(gpsspd < 100)
                    f2_to_char(gpsspd, p);
                else
                    f1_to_char(gpsspd, p);
                panel = ui_speed_screen.speed;
                lv_label_set_text(panel, p);
                speed_info_bar_update();
                statusbar_update();

                break;
            case SCREEN_MODE_SPEED_2:
                goto link_for_screen_mode_speed_2;
                break;
            case SCREEN_MODE_SPEED_STATS_1:
                ESP_LOGW(TAG, "Screen mode stats 1, 10s: %d", screen_mode);
                sc_data = &sc_screens[0];
                break;
            case SCREEN_MODE_SPEED_STATS_2:
                ESP_LOGW(TAG, "Screen mode stats 2, avg: %d", screen_mode);
                sc_data = &sc_screens[1];
                break;
            case SCREEN_MODE_SPEED_STATS_3:
                ESP_LOGW(TAG, "Screen mode stats 3, stats: %d", screen_mode);
                sc_data = &sc_screens[2];
                break;
            case SCREEN_MODE_SPEED_STATS_4:
                ESP_LOGW(TAG, "Screen mode stats 4, 500m: %d", screen_mode);
                sc_data = &sc_screens[3];
                break;
            case SCREEN_MODE_SPEED_STATS_5:
                ESP_LOGW(TAG, "Screen mode stats 5, 250m: %d", screen_mode);
                sc_data = &sc_screens[4];
                break;
            case SCREEN_MODE_SPEED_STATS_6:
                ESP_LOGW(TAG, "Screen mode stats 6, Alpha avg: %d", screen_mode);
                sc_data = &sc_screens[5];
                break;
            case SCREEN_MODE_SPEED_STATS_7:
            case SCREEN_MODE_SPEED_STATS_8:
            case SCREEN_MODE_SPEED_STATS_9:
                break;
            case SCREEN_MODE_WIFI_START:
            case SCREEN_MODE_WIFI_AP:
            case SCREEN_MODE_WIFI_STATION:
                me->self->state.update_delay = 1000;
                //ui_flush_screens(&ui_info_screen.screen);
                showWifiScreen(wifi_context.s_ap_connection ? wifi_context.ap.ssid : wifi_context.stas[0].ssid, wifi_context.ip_address);
                statusbar_update();
#if defined(CONFIG_DISPLAY_DRIVER_ST7789)
                lv_obj_set_style_img_recolor(ui_info_screen.info_img, lv_color_hex(0x104951), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_img_recolor_opa(ui_info_screen.info_img, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
#endif
                break;
            default:
                break;
        }
        if (is_gps_stat_screen) {
           //ui_flush_screens(&ui_stats_screen.screen);
            // lv_scr_load(UI_STATS_SCREEN);
            if (sc_data->num_fields == 6) {
                showStatsScreen32();
                ESP_LOGI(TAG, "6Field stats panel eq 3Rowx2Slots");
                //lv_obj_add_flag(ui_StatsPanel4Row, LV_OBJ_FLAG_HIDDEN);
                // 1. left slot
                // parent= ui_Stats3Row1LeftPanel;
                gpsspd = sc_data->fields[0].field->value.num() * m_context_rtc.RTC_calibration_speed;
                f2_to_char(gpsspd, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_DATALABEL);
                panel = ui_stats_screen.cells[0][0].title;
                lv_label_set_text(panel, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_INFOLABEL);
                panel = ui_stats_screen.cells[0][0].info;
                if(strcmp(lv_label_get_text(panel), sc_data->fields[0].field->name))
                    lv_label_set_text(panel, sc_data->fields[0].field->name);
                // 1. right slot
                // parent=ui_Stats3Row1RightPanel;
                gpsspd = sc_data->fields[3].field->value.num() * m_context_rtc.RTC_calibration_speed;
                f2_to_char(gpsspd, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_DATALABEL);
                panel = ui_stats_screen.cells[0][1].title;
                lv_label_set_text(panel, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_INFOLABEL);
                panel = ui_stats_screen.cells[0][1].info;
                if(strcmp(lv_label_get_text(panel), sc_data->fields[3].field->abbr))
                    lv_label_set_text(panel, sc_data->fields[3].field->abbr);
                // 2. left slot
                // parent=ui_Stats3Row2LeftPanel;
                gpsspd = sc_data->fields[1].field->value.num() * m_context_rtc.RTC_calibration_speed;
                f2_to_char(gpsspd, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_DATALABEL);
                panel = ui_stats_screen.cells[1][0].title;
                lv_label_set_text(panel, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_INFOLABEL);
                panel = ui_stats_screen.cells[1][0].info;
                if(strcmp(lv_label_get_text(panel), sc_data->fields[1].field->abbr))
                    lv_label_set_text(panel, sc_data->fields[1].field->abbr);
                // 2. right slot
                // parent=ui_Stats3Row2RightPanel;
                gpsspd = sc_data->fields[4].field->value.num() * m_context_rtc.RTC_calibration_speed;
                f2_to_char(gpsspd, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_DATALABEL);
                panel = ui_stats_screen.cells[1][1].title;
                lv_label_set_text(panel, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_INFOLABEL);
                panel = ui_stats_screen.cells[1][1].info;
                if(strcmp(lv_label_get_text(panel), sc_data->fields[4].field->abbr))
                    lv_label_set_text(panel, sc_data->fields[4].field->abbr);
                // 3. left slot
                // parent=ui_Stats3Row3LeftPanel;
                gpsspd = sc_data->fields[2].field->value.num() * m_context_rtc.RTC_calibration_speed;
                f2_to_char(gpsspd, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_DATALABEL);
                panel = ui_stats_screen.cells[2][0].title;
                lv_label_set_text(panel, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_INFOLABEL);
                panel = ui_stats_screen.cells[2][0].info;
                if(strcmp(lv_label_get_text(panel), sc_data->fields[2].field->abbr))
                    lv_label_set_text(panel, sc_data->fields[2].field->abbr);
                // 3. right slot
                // parent=ui_Stats3Row3RightPanel;
                gpsspd = sc_data->fields[5].field->value.num() * m_context_rtc.RTC_calibration_speed;
                f2_to_char(gpsspd, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_DATALABEL);
                panel = ui_stats_screen.cells[2][1].title;
                lv_label_set_text(panel, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_INFOLABEL);
                panel = ui_stats_screen.cells[2][1].info;
                if(strcmp(lv_label_get_text(panel), sc_data->fields[5].field->abbr))
                    lv_label_set_text(panel, sc_data->fields[5].field->abbr);
            } 
            else if (sc_data->num_fields == 4) {
                showStatsScreen22();
                ESP_LOGI(TAG, "3Field stats panel eq 2Rowx2lots");
                //lv_obj_add_flag(ui_StatsPanel4Row, LV_OBJ_FLAG_HIDDEN);
                // 1. left slot
                // parent= ui_Stats3Row1LeftPanel;
                gpsspd = sc_data->fields[0].field->value.num() * m_context_rtc.RTC_calibration_speed;
                f2_to_char(gpsspd, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_DATALABEL);
                panel = ui_stats_screen.cells[0][0].title;
                lv_label_set_text(panel, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_INFOLABEL);
                panel = ui_stats_screen.cells[0][0].info;
                if(strcmp(lv_label_get_text(panel), sc_data->fields[0].field->name))
                    lv_label_set_text(panel, sc_data->fields[0].field->name);
                // 1. right slot
                // parent=ui_Stats3Row1RightPanel;
                gpsspd = sc_data->fields[1].field->value.num() * m_context_rtc.RTC_calibration_speed;
                f2_to_char(gpsspd, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_DATALABEL);
                panel = ui_stats_screen.cells[0][1].title;
                lv_label_set_text(panel, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_INFOLABEL);
                panel = ui_stats_screen.cells[0][1].info;
                if(strcmp(lv_label_get_text(panel), sc_data->fields[1].field->abbr))
                    lv_label_set_text(panel, sc_data->fields[1].field->abbr);
                // 2. left slot
                // parent=ui_Stats3Row2LeftPanel;
                gpsspd = sc_data->fields[2].field->value.num() * m_context_rtc.RTC_calibration_speed;
                f2_to_char(gpsspd, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_DATALABEL);
                panel = ui_stats_screen.cells[1][0].title;
                lv_label_set_text(panel, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_INFOLABEL);
                panel = ui_stats_screen.cells[1][0].info;
                if(strcmp(lv_label_get_text(panel), sc_data->fields[2].field->abbr))
                    lv_label_set_text(panel, sc_data->fields[2].field->abbr);
                // 2. right slot
                // parent=ui_Stats3Row2LeftPanel;
                gpsspd = sc_data->fields[3].field->value.num() * m_context_rtc.RTC_calibration_speed;
                f2_to_char(gpsspd, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_DATALABEL);
                panel = ui_stats_screen.cells[1][1].title;
                lv_label_set_text(panel, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_INFOLABEL);
                panel = ui_stats_screen.cells[1][1].info;
                if(strcmp(lv_label_get_text(panel), sc_data->fields[3].field->abbr))
                    lv_label_set_text(panel, sc_data->fields[3].field->abbr);
            } else {
                ESP_LOGI(TAG, "2Field stats panel eq 2Rowx1+2Slots");
                showStatsScreen12();
                //lv_obj_clear_flag(ui_StatsPanel4Row, LV_OBJ_FLAG_HIDDEN);
                // 1. full last run
                // parent=ui_Stats2Row1FullPanel;
                gpsspd = sc_data->fields[0].field->value.num() * m_context_rtc.RTC_calibration_speed;
                f2_to_char(gpsspd, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_DATALABEL);
                panel = ui_stats_screen.cells[0][0].title;
                lv_label_set_text(panel, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_INFOLABEL);
                panel = ui_stats_screen.cells[0][0].info;
                if(strcmp(lv_label_get_text(panel), sc_data->fields[0].field->abbr))
                    lv_label_set_text(panel, sc_data->fields[0].field->name);
                // 2. left max run
                // parent=ui_Stats2Row2LeftPanel;
                gpsspd = sc_data->fields[1].field->value.num() * m_context_rtc.RTC_calibration_speed;
                f2_to_char(gpsspd, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_DATALABEL);
                panel = ui_stats_screen.cells[1][0].title;
                lv_label_set_text(panel, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_INFOLABEL);
                panel = ui_stats_screen.cells[1][0].info;
                if(strcmp(lv_label_get_text(panel), sc_data->fields[1].field->abbr))
                    lv_label_set_text(panel, sc_data->fields[1].field->abbr);
                // 2. right avg run
                // parent=ui_Stats2Row2RightPanel;
                gpsspd = sc_data->fields[2].field->value.num() * m_context_rtc.RTC_calibration_speed;
                f2_to_char(gpsspd, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_DATALABEL);
                panel = ui_stats_screen.cells[1][1].title;
                lv_label_set_text(panel, p);
                // panel = ui_comp_get_child(parent, UI_COMP_SMALLDATAPANEL_INFOLABEL);
                panel = ui_stats_screen.cells[1][1].info;
                if(strcmp(lv_label_get_text(panel), sc_data->fields[2].field->abbr))
                    lv_label_set_text(panel, sc_data->fields[2].field->abbr);
            }
            statusbar_update();
        }
        ++screen_num;
        _lvgl_unlock();
        // lv_timer_handler();
        ret += me->self->state.update_delay;
        old_screen = screen_mode;
        count++;
        TIMER_E
    }
    return ret;
}

display_op_t screen_ops = {
    .boot_screen = _boot_screen,
    .off_screen = _off_screen,
    .sleep_screen = _sleep_screen,
    .update_screen = _update_screen,
    .uninit = display_uninit};

struct display_s *display_init(struct display_s *me) {
    TIMER_S
    me->op = &screen_ops;
    me->self = &display_priv;
    reset_display_state(&display_priv.state);

    dspl = display_new();

    display_priv.displayHeight = LCD_V_RES;
    display_priv.displayWidth = LCD_H_RES;

    // init_fonts(fonts, sizeof(fonts) / sizeof(font_type_t));

    if(dspl)
        display_priv.displayOK = true;
    //lv_statusbar = lv_statusbar_create(lv_scr_act());
    
    lcd_ui_start();

    // TIMER_E
    return me;
}

uint32_t lcd_lv_timer_handler() {
    uint32_t task_delay_ms = L_LVGL_TASK_MAX_DELAY_MS;
    if (_lvgl_lock(-1)) {
        task_delay_ms = lv_timer_handler();
        _lvgl_unlock();
    }
    if (task_delay_ms > L_LVGL_TASK_MAX_DELAY_MS) {
        task_delay_ms = L_LVGL_TASK_MAX_DELAY_MS;
    } else if (task_delay_ms < L_LVGL_TASK_MIN_DELAY_MS) {
        task_delay_ms = L_LVGL_TASK_MIN_DELAY_MS;
    }
    return task_delay_ms;
}

void lcd_timer_task(void *args) {
    uint32_t task_delay_ms = lcd_lv_timer_handler();
    while (1) {
        task_delay_ms = lcd_lv_timer_handler();
        delay_ms(task_delay_ms); /*Sleep for 5 millisecond*/
    }
}

static void lcd_ui_start() {
    ui_init();
     xTaskCreatePinnedToCore(lcd_timer_task, "lcd_timer_task", 4096, NULL, 5, &display_task_handle, 1);
}

void lcd_ui_stop() {
    vTaskDelete(display_task_handle);
    ui_deinit();
}

void display_uninit(struct display_s *me) {
    LOGR
    if (me && me->self && me->self->displayOK) {
        lcd_ui_stop();
        display_del();
        dspl = 0;
        memset(me->self, 0, sizeof(struct display_priv_s));
        memset(me, 0, sizeof(struct display_s));
    }
}