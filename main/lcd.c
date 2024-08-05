
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
#include "logger_wifi.h"
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
screen_mode_t current_screen_mode = SCREEN_MODE_UNKNOWN, old_screen_mode = SCREEN_MODE_UNKNOWN;

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

static const char *TAG = "lcd";

static esp_err_t reset_display_state(struct display_state_s *display_state) {
    ILOG(TAG, "[%s]", __func__);
    display_state->initialized = 0;
    return ESP_OK;
}

static uint32_t count = 0, count_last_full_refresh = 0, count_last_fast_refresh = 0;
static uint8_t dots_counter = 0, offscreen_counter = 0;

size_t append_dots(char * p, uint8_t max_dots, uint8_t * cur_dots) {
    if(!p) return 0;
    if((*cur_dots)++ > max_dots) *cur_dots = 1;
    char * i = p + (*cur_dots) - 1, *j = p;
    for(; j < i; j++) *j = '.';
    i = p + max_dots;
    for(; j < i; j++) *j = ' ';
    *j = 0;
    return j - p;
}

typedef struct sleep_scr_s {
    float *data;
    const char *info;
} sleep_scr_t;

static const struct sleep_scr_s sleep_scr_info_fields[2][6] = {
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
    ILOG(TAG, "[%s]", __func__);

    char tmp[24], *p = tmp;
    lv_label_t *panel;
    // if (_lvgl_lock(50)) {
    current_screen_mode = SCREEN_MODE_SLEEP;
    showSleepScreen();
    statusbar_update();
    for(int i = 0; i < 6; i++) {
        for(int j = 0; j < 2; j++) {
            f2_to_char(*sleep_scr_info_fields[j][i].data, p);
            lv_label_set_text(ui_sleep_screen.cells[i][j].title, p);
            lv_label_set_text(ui_sleep_screen.cells[i][j].info, sleep_scr_info_fields[j][i].info);
        }
    }
    lv_label_set_text(ui_sleep_screen.myid, m_context_rtc.RTC_Sleep_txt);
    lcd_ui_request_full_refresh(0); // first screen load will cause full refresh
    ++count;
    return 100;
}

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
    ILOG(TAG, "[%s]", __func__);
    // TIMER_S
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
    // TIMER_E
    return ++len;
    end:
#endif
    *str=0;
    return 0;
}

static const char *scr_fld[2][8][2] = {
    {
        {"Run", "Avg"},
        {"Gate", "Ex"},
        {"AlpR", "AlpM"},
        {"NmR", "NmM"},
        {"Dst", "500M"},
        {"2sM", "10sM"},
        {".5hR", ".5hM"},
        {"1hR", "1hM"},
    },
    {
        {"R", "A"},
        {"G", "E"},
        {"AlR", 0},
        {"NmR", ""},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
    }
};

static const char * scr_fld_2[] = {
    "-.--",
    "0.00"
};

static int low_speed_seconds = 0;
static esp_err_t speed_info_bar_update() {  // info bar when config->speed_large_font is 1 or 0
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

    float s[] = {0, 0};
    const char *var[] = {0, 0};
    char val[][24] = {{0}, {0}}, *p;
    if (!m_context.gps.ublox_config->ready || !m_context.gps.ublox_config->signal_ok) {
        memcpy(val[0], scr_fld_2[0], 4);
        memcpy(val[1], scr_fld_2[0], 4);
        goto topoint;
    }
    if(m_context.gps.S2.avg_s < 1000) { // 1ms = 3.6km/h
        memcpy(val[0], scr_fld_2[1], 4);
        memcpy(val[1], scr_fld_2[1], 4);
        goto topoint;
    }
    // double s1 = 0, s2 = 0;
    else if (field <= 2 || display_priv.test_field == 2) { // 10 seconds stats
        s[0] = avail_fields[57].value.num(); // s10 current run max speed
        s[1] = avail_fields[2].value.num();  // s10 avg speed
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
            var[0] = scr_fld[0][0][0];
            var[1] = scr_fld[0][0][1];
        } else {
            var[0] = scr_fld[1][0][0];
            var[1] = scr_fld[1][0][1];
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
            var[0] =  scr_fld[0][1][0];
            f_to_char(m_context.gps.alfa_window, val[0], 0);
            var[1] =  scr_fld[1][1][1];
            f_to_char(m_context.gps.alfa_exit, val[1], 0);
        } else { // alfa speed stats
            s[0] = avail_fields[26].value.num(); // a500 current run max speed
            s[1] = avail_fields[61].value.num(); // a500 max speed
            if (s[0] > 100 || s[1] > 100) {
                if (font_size == 0)
                    f1_to_char(s[0], val[0]); // last alpha
                else
                    val[0][0] = 0; // var in 1. col, val in 2. col
                if (s[1] > 1) {
                    f1_to_char(s[1], val[1]); // best alpha
                } else {
                    goto tonpoint;
                }
            } else {
                if (font_size == 0)
                    f2_to_char(s[0], val[0]);
                else
                    val[0][0] = 0;
                if (s[1] > 1) {
                    f2_to_char(s[1], val[1]);
                } else {
                tonpoint:
                    memcpy(val[1], scr_fld_2[1], 4);
                    val[1][4]=0;
                }
            }
            if (font_size == 0) {
                var[0] = scr_fld[0][2][0];
                var[1] = scr_fld[0][2][1];  // nieuwe alfa laatste gijp or MISSED !!!!
            } else {
                // best Alfa from session on 500 m !!
                var[0] = scr_fld[1][2][0];
                var[1] = scr_fld[1][2][1];
            }
        }
    } else if (field == 4 || display_priv.test_field == 4) { // nautical mile
        s[0] = avail_fields[22].value.num(); // m1852 current run max speed
        s[1] = avail_fields[62].value.num(); // m1852 max speed
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
            var[0] = scr_fld[0][3][0];  // Actuele nautical mile
            var[1] = scr_fld[0][3][1];
        } else {
            var[0] = scr_fld[1][3][0];  // Actuele nautical mile
            var[1] = scr_fld[1][3][1];
        }
    } else if (field == 5 || display_priv.test_field == 5) { // total distance
        s[0] = avail_fields[41].value.num(); // total dist
        s[1] = avail_fields[63].value.num(); // m500 current run max speed
        var[0] = scr_fld[0][4][0];
        var[1] = scr_fld[0][4][1];
        f1_to_char(s[0], val[0]);
        if (s[1] > 100) {
            f1_to_char(s[1], val[1]);
        }
        else
            f2_to_char(s[1], val[1]);
    } else if (field == 6 || display_priv.test_field == 6) { // 2 and 10 seconds stats
        s[0] = avail_fields[5].value.num(); // s2 max speed
        s[1] = avail_fields[1].value.num(); // s10 max speed
        if (s[0] > 100 || s[1] > 100) {
            f1_to_char(s[0], val[0]);
            f1_to_char(s[1], val[1]);
        } else {
            f2_to_char(s[0], val[0]);
            f2_to_char(s[1], val[1]);
        }
        var[0] = scr_fld[0][5][0];
        var[1] = scr_fld[0][5][1];
    } else if (field == 7 || display_priv.test_field == 7) { // 30 minutes stats
        s[0] = avail_fields[64].value.num();
        s[1] = avail_fields[34].value.num(); // s1800 max speed
        if (s[0] > 100 || s[1] > 100) {
            f1_to_char(s[0], val[0]);
            f1_to_char(s[1], val[1]);
        } else {
            f2_to_char(s[0], val[0]);
            f2_to_char(s[1], val[1]);
        }
        var[0] = scr_fld[0][6][0];
        var[1] = scr_fld[0][6][1];
    } else if (field == 8 || display_priv.test_field == 8) { // 60 minutes stats
        s[0] = avail_fields[65].value.num();
        s[1] = avail_fields[38].value.num(); // 1h max speed
        if (s[0] > 100 || s[1] > 100) {
            f1_to_char(s[0], val[0]);
            f1_to_char(s[1], val[1]);
        } else {
            f2_to_char(s[0], val[0]);
            f2_to_char(s[1], val[1]);
        }
        var[0] = scr_fld[0][7][0];
        var[1] = scr_fld[0][7][1];
    }

    // col 1
        lv_label_set_text(ui_speed_screen.cells[0][0].info, var[0]);
        lv_label_set_text(ui_speed_screen.cells[0][0].title, val[0]);
    // col 2
        lv_label_set_text(ui_speed_screen.cells[0][1].info, var[1]);
        lv_label_set_text(ui_speed_screen.cells[0][1].title, val[1]);

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
    char tmp[24]={0}, *p = tmp;
    lv_obj_t *panel;
    if ((panel = statusbar->time_label)) {
        if (lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN))
            lv_obj_clear_flag(panel, LV_OBJ_FLAG_HIDDEN);
        if (statusbar->viewmode==0 || statusbar->viewmode == 2) {
            struct tm tms;
            memset(&tms, 0, sizeof(struct tm));
            getLocalTime(&tms, 0);
            time_to_char_hm(tms.tm_hour, tms.tm_min, p);
        } else {
            p += time_to_char_hm(m_context_rtc.RTC_hour, m_context_rtc.RTC_min, p), *p++ = ' ';
            p += date_to_char(m_context_rtc.RTC_day, m_context_rtc.RTC_month, m_context_rtc.RTC_year, 0, p), *p = 0;
        }
        if(strcmp(lv_label_get_text(panel), &(tmp[0]))) {
#if (CONFIG_LOGGER_COMMON_LOG_LEVEL < 2)
            printf("** (date)time: %s ** \n", tmp);
#endif
            lv_label_set_text(panel, &(tmp[0]));
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

// static bool sdblink = false;
// static void statusbar_sdcard_cb(lv_timer_t *timer) {
//     #if defined(STATUS_PANEL_V1)
//     ui_status_panel_t * statusbar = &ui_status_panel;
//     if(!statusbar->parent) {
//         return;
//     }
// #else
//     lv_statusbar_t * statusbar = (lv_statusbar_t *)ui_StatusPanel;
// #endif
//     char tmp[24], *p = tmp;
//     lv_obj_t *panel;
//     if ((panel = statusbar->sdcard_image)) {
//         if (m_context.sdOK) {
// #if defined(CONFIG_DISPLAY_DRIVER_ST7789)
//             lv_obj_set_style_img_recolor(panel, lv_color_hex(0xA9B7B9), LV_PART_MAIN | LV_STATE_DEFAULT);
//             lv_obj_set_style_img_recolor_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
// #endif
//             if (lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN)) {
//                 lv_obj_clear_flag(panel, LV_OBJ_FLAG_HIDDEN);
//             }
//         } else {
//             if (sdblink == 0) {
//                 if (!lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN)) {
//                     lv_obj_add_flag(panel, LV_OBJ_FLAG_HIDDEN);
//                 }
//                 sdblink = 1;
//             } else {
//                 if (lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN)) {
//                     lv_obj_clear_flag(panel, LV_OBJ_FLAG_HIDDEN);
//                 }
// #if defined(CONFIG_DISPLAY_DRIVER_ST7789)
//                 lv_obj_set_style_img_recolor(panel, lv_color_hex(0xE32424), LV_PART_MAIN | LV_STATE_DEFAULT);
//                 lv_obj_set_style_img_recolor_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
// #endif
//                 sdblink = 0;
//             }
//         }
//     }
// }

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
        if ((m_context.gps.ublox_config->is_on)) {
            if (lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_clear_flag(panel, LV_OBJ_FLAG_HIDDEN);
            }
        } else {
            if (!lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_add_flag(panel, LV_OBJ_FLAG_HIDDEN);
            }
            return;
        }
        //if(statusbar->viewmode==2) { 
            if (!m_context.gps.ublox_config->ready){
                memcpy(p, "-n-", 3);
            }
            // else if(!m_context.gps.ublox_config->signal_ok) {
            //     uint8_t qp = gblink;
            //     memcpy(p, "gps   ", 6);
            //     *(p+6)=0;
            //     p += 3;
            //     while(qp--) {
            //         *p++ = '.';
            //     }
            //     gblink = gblink==3 ? 0 : gblink+1;
            // }
            else {
                p += xltoa(m_context.gps.ublox_config->ubx_msg.navPvt.numSV, p);
                *p = 0;
            }
            p = lv_label_get_text(panel);
            if(!p || strcmp(p, &(tmp[0]))) {
                lv_label_set_text(panel, &(tmp[0]));
            }
    //   }
    //     else {
    //         p = lv_label_get_text(panel);
    //         if(!p || memcmp(p, LV_SYMBOL_GPS, 3)) {
    //             lv_label_set_text(panel, LV_SYMBOL_GPS);
    //         }
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
    // }
}

static void statusbar_update() {
    // battery //
    statusbar_bat_cb(0);
    // // time //
    statusbar_time_cb(0);
    // // temp //
    statusbar_temp_cb(0);
    // // sdcard image //
    // statusbar_sdcard_cb(0);
    // // gps/wifi image //
    statusbar_gps_cb(0);
}

typedef struct sat_count_s {
    uint8_t gps;
    uint8_t sbas;
    uint8_t galileo;
    uint8_t beidou;
    uint8_t qzss;
    uint8_t glonass;
    uint8_t navic;
} sat_count_t;

sat_count_t sat_count = {0};
char gps_status_str[32] = {0};
char bat_status_str[32] = {0};

static void update_sat_count() {
    struct nav_sat_s *nav_sat = &m_context.gps.ublox_config->ubx_msg.nav_sat;
    const struct svs_nav_sat_s * sat = 0;
    memset(&sat_count, 0, sizeof(sat_count_t));
    for(uint8_t i=0; i < nav_sat->numSvs; i++) {
        sat = &nav_sat->sat[i];
#if defined(CONFIG_LOGGER_COMMON_LOG_LEVEL_TRACE)
        printf("sat[%hhu]: %hhu, %hhu, %hhu, %hhu, %hu, %lu %lu %lu\n", i, sat->gnssId, sat->svId, sat->cno, sat->elev, sat->azim, sat->flags, (sat->flags & 0x08), (sat->flags & 0x07));
#endif
        if((sat->flags & 0x08) == 0 || (sat->flags & 0x07) < 4)
            continue;
        switch(sat->gnssId) {
            case 0:
                sat_count.gps++;
                break;
            case 1:
                sat_count.sbas++;
                break;
            case 2:
                sat_count.galileo++;
                break;
            case 3:
                sat_count.beidou++;
                break;
            case 5:
                sat_count.qzss++;
                break;
            case 6:
                sat_count.glonass++;
                break;
            case 7:
                sat_count.navic++;
                break;
            default:
                break;
        }
    }
    ILOG(TAG, "gnss: %hhu, count: %hhu, G:%d, S:%d, E:%d, B:%d, Q:%d, R:%d, N:%d", m_context.gps.ublox_config->rtc_conf->gnss, nav_sat->numSvs, sat_count.gps, sat_count.sbas, sat_count.galileo, sat_count.beidou, sat_count.qzss, sat_count.glonass, sat_count.navic);
}

static size_t update_gps_info_row_str(char * p) {
    update_sat_count();
    char * pc = p;
    uint8_t gnss = m_context.gps.ublox_config->rtc_conf->gnss;
    pc += xultoa(m_context.gps.ublox_config->ubx_msg.navPvt.numSV, pc);
    memcpy(pc, "sat", 3), pc += 3;
    if((gnss & (1 << 0))!=0) {
        *pc++ = ' ';
        *pc++ = 'G';
        pc += xultoa(sat_count.gps, pc);
    }
    if((gnss & (1 << 2))!=0) {
        *pc++ = ' ';
        *pc++ = 'E';
        pc += xultoa(sat_count.galileo, pc);
    }
    if((gnss & (1 << 3))!=0) {
        *pc++ = ' ';
        *pc++ = 'B';
        pc += xultoa(sat_count.beidou, pc);
    }
    if((gnss & (1 << 6))!=0) {
        *pc++ = ' ';
        *pc++ = 'R';
        pc += xultoa(sat_count.glonass, pc);
    }
    *pc = 0;
    return pc - p;
}

static size_t update_gps_desc_row_str(char * p) {
    char * pb = p;
    memcpy(pb, "Bat: ", 5), pb += 5;
    pb += f3_to_char(m_context_rtc.RTC_voltage_bat, pb);
    memcpy(pb, "V ", 2), pb += 2;
    if(m_context.gps.ublox_config->first_fix){
        memcpy(pb, " fx: ", 5), pb += 5;
        pb += xultoa(m_context.gps.ublox_config->first_fix, pb);
        *pb++ = 's';
    }
    if(m_context.gps.lost_frames) {
        memcpy(pb, " lst: ", 6), pb += 6;
        pb += xultoa(m_context.gps.lost_frames, pb), *pb=0;
    }
    return pb - p;
}

static uint32_t next_gps_str_update = 0;
static uint16_t screen_mode_counter = 0;
uint16_t get_offscreen_counter() {
    return current_screen_mode == SCREEN_MODE_SHUT_DOWN ? screen_mode_counter : 0;
}

static uint32_t _update_screen(const struct display_s *me, const screen_mode_t screen_mode, void *arg) {
    ILOG(TAG, "[%s] %d", __func__, screen_mode);
    uint32_t ret = 0;
    UNUSED_PARAMETER(ret);
        logger_config_t *config = m_context.config;
        char str[24] = {0}, *p = str, str1[32]={0}, *pb = str1, str2[32]={0}, *pc = str1;
        bool is_gps_stat_screen = (screen_mode > 0 && screen_mode < 10);
        me->self->state.update_delay = 500;
        // ESP_LOGI(TAG, "update screen: mode:%" PRIu8 ", update nr:%lu", screen_mode, count);
        float gpsspd;
        int state = (int)arg;
        lv_obj_t *panel, *parent;
        stat_screen_t *sc_data = 0;
        const char *gps = 0;
        size_t gpslen=0;
        const lv_img_dsc_t *img_src = 0;
        const char * scr_mode_str = "Screen mode ";
        current_screen_mode = screen_mode;
        
        if(dots_counter && old_screen_mode != screen_mode) {
            dots_counter = 0;
        }
        if(old_screen_mode == screen_mode) {
            ++screen_mode_counter;
        }
        else {
            screen_mode_counter = 1;
        }
        switch (screen_mode) {
            case SCREEN_MODE_GPS_TROUBLE:
                showGpsTroubleScreen();
                break;
            case SCREEN_MODE_GPS_INIT:
            case SCREEN_MODE_GPS_READY:
                gps = ubx_chip_str(m_context.gps.ublox_config);
                gpslen = strlen(gps);
                memcpy(p, gps, gpslen);
                p += gpslen;
                uint8_t gps_err = 0;
                if(gpslen==7 && !strcmp(gps, "UNKNOWN")) {
                    img_src = &near_me_disabled_bold_48px;
                    memcpy(&str[0], "GPS FAIL", 8), p += 8;
                    gps_err = 1;
                }
                else {
                    img_src = &near_me_bold_48px;
                    *p++ = '@';
                    p += xltoa(m_context.gps.ublox_config->rtc_conf->output_rate, p);
                    memcpy(p, "Hz", 2), p += 2;
                }
                *p = 0;
                
                uint32_t now = get_millis();
                if(now > next_gps_str_update) {
                    next_gps_str_update = now + 2000;
                    update_gps_info_row_str(&bat_status_str[0]);
                    if(!gps_err) update_gps_desc_row_str(&gps_status_str[0]);
                }
                showGpsScreen(&str[0], &bat_status_str[0], &gps_status_str[0], img_src, gps_image_angle);
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
                append_dots(p, 10, &dots_counter);
                showPushScreen(state, &str[0]);
                break;
            case SCREEN_MODE_SD_TROUBLE:
                showSdTroubleScreen();
                break;
            case SCREEN_MODE_LOW_BAT:
                link_for_low_bat:
                showLowBatScreen();
                break;
            case SCREEN_MODE_SHUT_DOWN:
                ui_flush_screens(&ui_init_screen.screen);
                float session_time = avail_fields[59].value.num();
                float distance = avail_fields[41].value.num();
                const char *title = m_context.request_restart ? "Reboot device" : m_context.Shut_down_Save_session ? 0 : "Going to sleep";
                memcpy(&str[0], title ? title : "Save session", title ? strlen(title) : 12), p += title ? strlen(title) : 12;
                append_dots(p, 3, &dots_counter);
                if(m_context.low_bat_count > 5) {
                    //current_screen_mode = SCREEN_MODE_LOW_BAT;
                    goto link_for_low_bat;
                } else {
                    //current_screen_mode = SCREEN_MODE_OFF_SCREEN;
                    if(!title) {
                        memcpy(pb, "Time: ", 6), pb += 6;
                        sec_to_hms_str(session_time, pb);
                        memcpy(pc, "Distance: ", 10), pc += 10;
                        pc += f2_to_char(distance, pc);
                        memcpy(pc, " km", 3), pc+=3, *pc = 0;
                        showSaveSessionScreen(&str[0], &str1[0], &str2[0]);
                    }
                    else
                        showBootScreen(&str[0]);
                }
                // lcd_ui_request_fast_refresh(0);
                me->self->state.update_delay = screen_mode_counter < 2 ? 100 : 500;
                break;
            case SCREEN_MODE_BOOT:
                ui_flush_screens(&ui_init_screen.screen);
                lcd_ui_request_full_refresh(0); // first screen load will cause full refresh
                // current_screen_mode = SCREEN_MODE_BOOT;
                memcpy(p, "Booting", 7), p += 7;
                append_dots(p, 3, &dots_counter);
                showBootScreen(&str[0]);
                if (count<2) me->self->state.update_delay = 100;
                break;
            case SCREEN_MODE_SPEED_1:
            link_for_screen_mode_speed_2:
                showSpeedScreen();
                gpsspd = gps_last_speed_smoothed(2) * m_context_rtc.RTC_calibration_speed;
                if (!m_context.gps.ublox_config->ready || !m_context.gps.ublox_config->signal_ok) {
                    memcpy(p, "-.--", 4);
                    *(p+4) = 0;
                }
                else if(m_context.gps.S2.avg_s < 1000) {
                    memcpy(p, "0.00", 4);
                    *(p+4) = 0;
                } else {
                    me->self->state.update_delay = 100;
                    if(gpsspd < 100)
                        f2_to_char(gpsspd, p);
                    else
                        f1_to_char(gpsspd, p);
                }
                panel = ui_speed_screen.speed;
                lv_label_set_text(panel, p);
                speed_info_bar_update();
                statusbar_update();

                break;
            case SCREEN_MODE_SPEED_2:
                goto link_for_screen_mode_speed_2;
                break;
            case SCREEN_MODE_SPEED_STATS_1:
                DLOG(TAG, "[%s] %s, stat 10s: %d", __func__, scr_mode_str, screen_mode);
                sc_data = &sc_screens[0];
                break;
            case SCREEN_MODE_SPEED_STATS_2:
                DLOG(TAG, "[%s] %s, stat 2s: %d", __func__, scr_mode_str, screen_mode);
                sc_data = &sc_screens[1];
                break;
            case SCREEN_MODE_SPEED_STATS_3:
                DLOG(TAG, "[%s] %s, stat 250m: %d", __func__, scr_mode_str, screen_mode);
                sc_data = &sc_screens[2];
                break;
            case SCREEN_MODE_SPEED_STATS_4:
                DLOG(TAG, "[%s] %s, stat 500m: %d", __func__, scr_mode_str, screen_mode);
                sc_data = &sc_screens[3];
                break;
            case SCREEN_MODE_SPEED_STATS_5:
                DLOG(TAG, "[%s] %s, stat 1852m: %d", __func__, scr_mode_str, screen_mode);
                sc_data = &sc_screens[4];
                break;
            case SCREEN_MODE_SPEED_STATS_6:
                DLOG(TAG, "[%s] %s, stat A500: %d", __func__, scr_mode_str, screen_mode);
                sc_data = &sc_screens[5];
                break;
            case SCREEN_MODE_SPEED_STATS_7:
                DLOG(TAG, "[%s] %s, stat 10sec avg: %d", __func__, scr_mode_str, screen_mode);
                    sc_data = &sc_screens[6];
                    break;
            case SCREEN_MODE_SPEED_STATS_8:
                DLOG(TAG, "[%s] %s, stat stats: %d", __func__, scr_mode_str, screen_mode);
                sc_data = &sc_screens[7];
                break;
            case SCREEN_MODE_SPEED_STATS_9:
                DLOG(TAG, "[%s] %s, stat alpha avg: %d", __func__, scr_mode_str, screen_mode);
                sc_data = &sc_screens[8];
                break;
            case SCREEN_MODE_WIFI_START:
            case SCREEN_MODE_WIFI_AP:
            case SCREEN_MODE_WIFI_STATION:
                DLOG(TAG, "[%s] %s, wifi %d", __func__, scr_mode_str, screen_mode);
                me->self->state.update_delay = 600;
                const char *ssid = wifi_context.s_ap_connection ? wifi_context.ap.ssid : wifi_context.s_sta_num_connect < M_WIFI_STA_MAX ? wifi_context.stas[wifi_context.s_sta_num_connect].ssid : "";
                showWifiScreen(ssid, wifi_context.ip_address, wifi_context.s_ap_connection ? "password" : "");
                statusbar_update();
#if defined(CONFIG_DISPLAY_DRIVER_ST7789)
                lv_obj_set_style_img_recolor(ui_info_screen.info_img, lv_color_hex(0x104951), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_img_recolor_opa(ui_info_screen.info_img, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
#endif
                break;
            case SCREEN_MODE_RECORD:
                const struct record_forwarder_s *rec = arg;
                showRecordScreen(rec->num);
                if(arg) {
                    panel = ui_record_screen.prev_lbl;
                    get_display_fld_str(rec->prev, p, f2_to_char);
                    lv_label_set_text(panel, p);
                    panel = ui_record_screen.cur_lbl;
                    get_display_fld_str(rec->cur, p, f2_to_char);
                    lv_label_set_text(panel, p);
                    panel = ui_record_screen.info_lbl;
                    lv_label_set_text(panel, rec->cur->grp);
                }
                break;
            case SCREEN_MODE_SETTINGS:
                if(arg) {
                    const v_settings_t *s = arg;
                    const logger_config_item_t *i = (const logger_config_item_t *)s->settings_data;
                    if(!i) goto plain_setting;
                    showSettingsScreen(s->name, i->name, i->desc);
                }
                else {
                    plain_setting:
                    showSettingsScreen("Settings","-","-");
                }
                break;
            default:
                break;
        }
        if (is_gps_stat_screen) {
            const char * f_name = 0;
            if (sc_data->num_fields == 6) {
                showStatsScreen32();
                DLOG(TAG, "[%s] stats panel: 3Row x 2Slot", __func__);
                //lv_obj_add_flag(ui_StatsPanel4Row, LV_OBJ_FLAG_HIDDEN);
                // 1. left slot
                panel = ui_stats_screen.cells[0][0].title;
                get_display_fld_str(sc_data->fields[0].field, p, f2_to_char);
                lv_label_set_text(panel, p);
                panel = ui_stats_screen.cells[0][0].info;
                if(strcmp(lv_label_get_text(panel), sc_data->fields[0].field->name))
                    lv_label_set_text(panel, sc_data->fields[0].field->name);
                // 1. right slot
                panel = ui_stats_screen.cells[0][1].title;
                get_display_fld_str(sc_data->fields[3].field, p, f2_to_char);
                lv_label_set_text(panel, p);
                panel = ui_stats_screen.cells[0][1].info;
                f_name = sc_data->use_abbr ? sc_data->fields[3].field->abbr : sc_data->fields[3].field->name;
                if(strcmp(lv_label_get_text(panel), f_name))
                    lv_label_set_text(panel, f_name);
                // 2. left slot
                panel = ui_stats_screen.cells[1][0].title;
                get_display_fld_str(sc_data->fields[1].field, p, f2_to_char);
                lv_label_set_text(panel, p);
                panel = ui_stats_screen.cells[1][0].info;
                f_name = sc_data->use_abbr ? sc_data->fields[1].field->abbr : sc_data->fields[1].field->name;
                if(strcmp(lv_label_get_text(panel), f_name))
                    lv_label_set_text(panel, f_name);
                // 2. right slot
                panel = ui_stats_screen.cells[1][1].title;
                get_display_fld_str(sc_data->fields[4].field, p, f2_to_char);
                lv_label_set_text(panel, p);
                panel = ui_stats_screen.cells[1][1].info;
                f_name = sc_data->use_abbr ? sc_data->fields[4].field->abbr : sc_data->fields[4].field->name;
                if(strcmp(lv_label_get_text(panel), f_name))
                    lv_label_set_text(panel, f_name);
                // 3. left slot
                // parent=ui_Stats3Row3LeftPanel;
                panel = ui_stats_screen.cells[2][0].title;
                get_display_fld_str(sc_data->fields[2].field, p, f2_to_char);
                lv_label_set_text(panel, p);
                panel = ui_stats_screen.cells[2][0].info;
                f_name = sc_data->use_abbr ? sc_data->fields[2].field->abbr : sc_data->fields[2].field->name;
                if(strcmp(lv_label_get_text(panel), f_name))
                    lv_label_set_text(panel, f_name);
                // 3. right slot
                // parent=ui_Stats3Row3RightPanel;
                panel = ui_stats_screen.cells[2][1].title;
                lv_label_set_text(panel, p);
                get_display_fld_str(sc_data->fields[5].field, p, f2_to_char);
                panel = ui_stats_screen.cells[2][1].info;
                f_name = sc_data->use_abbr ? sc_data->fields[5].field->abbr : sc_data->fields[5].field->name;
                if(strcmp(lv_label_get_text(panel), f_name))
                    lv_label_set_text(panel, f_name);
            } 
            else if (sc_data->num_fields == 4) {
                showStatsScreen22();
                DLOG(TAG, "[%s] stats panel: 2Row x 2Slot", __func__);
                //lv_obj_add_flag(ui_StatsPanel4Row, LV_OBJ_FLAG_HIDDEN);
                // 1. left slot
                // parent= ui_Stats3Row1LeftPanel;
                panel = ui_stats_screen.cells[0][0].title;
                get_display_fld_str(sc_data->fields[0].field, p, f2_to_char);
                lv_label_set_text(panel, p);
                panel = ui_stats_screen.cells[0][0].info;
                if(strcmp(lv_label_get_text(panel), sc_data->fields[0].field->name))
                    lv_label_set_text(panel, sc_data->fields[0].field->name);
                // 1. right slot
                panel = ui_stats_screen.cells[0][1].title;
                get_display_fld_str(sc_data->fields[1].field, p, f2_to_char);
                lv_label_set_text(panel, p);
                panel = ui_stats_screen.cells[0][1].info;
                f_name = sc_data->use_abbr ? sc_data->fields[1].field->abbr : sc_data->fields[1].field->name;
                if(strcmp(lv_label_get_text(panel), f_name))
                    lv_label_set_text(panel, f_name);
                // 2. left slot
                panel = ui_stats_screen.cells[1][0].title;
                get_display_fld_str(sc_data->fields[2].field, p, f2_to_char);
                lv_label_set_text(panel, p);
                panel = ui_stats_screen.cells[1][0].info;
                f_name = sc_data->use_abbr ? sc_data->fields[2].field->abbr : sc_data->fields[2].field->name;
                if(strcmp(lv_label_get_text(panel), f_name))
                    lv_label_set_text(panel, f_name);
                // 2. right slot
                panel = ui_stats_screen.cells[1][1].title;
                get_display_fld_str(sc_data->fields[3].field, p, f2_to_char);
                lv_label_set_text(panel, p);
                panel = ui_stats_screen.cells[1][1].info;
                f_name = sc_data->use_abbr ? sc_data->fields[3].field->abbr : sc_data->fields[3].field->name;
                if(strcmp(lv_label_get_text(panel), f_name))
                    lv_label_set_text(panel, f_name);
            } else if (sc_data->num_fields == 2) {
                DLOG(TAG, "[%s] stats panel: 2Row x 1Slot", __func__);
                loadStatsScreen(2,1);
                // field 1
                panel = ui_stats_screen.cells[0][0].title;
                get_display_fld_str(sc_data->fields[0].field, p, f2_to_char);
                lv_label_set_text(panel, p);
                panel = ui_stats_screen.cells[0][0].info;
                if(strcmp(lv_label_get_text(panel), sc_data->fields[0].field->abbr))
                    lv_label_set_text(panel, sc_data->fields[0].field->name);
                // field 2
                panel = ui_stats_screen.cells[1][0].title;
                get_display_fld_str(sc_data->fields[1].field, p, f2_to_char);
                lv_label_set_text(panel, p);
                panel = ui_stats_screen.cells[1][0].info;
                f_name = sc_data->use_abbr ? sc_data->fields[1].field->abbr : sc_data->fields[1].field->name;
                if(strcmp(lv_label_get_text(panel), f_name))
                    lv_label_set_text(panel, f_name);
            } else {
                DLOG(TAG, "[%s] stats panel: 3Row x 1Slot", __func__);
                loadStatsScreen(3,1);
                // 1. full last run
                panel = ui_stats_screen.cells[0][0].title;
                get_display_fld_str(sc_data->fields[0].field, p, f2_to_char);
                lv_label_set_text(panel, p);
                panel = ui_stats_screen.cells[0][0].info;
                if(strcmp(lv_label_get_text(panel), sc_data->fields[0].field->abbr))
                    lv_label_set_text(panel, sc_data->fields[0].field->name);
                // 2. max run
                panel = ui_stats_screen.cells[1][0].title;
                get_display_fld_str(sc_data->fields[1].field, p, f2_to_char);
                lv_label_set_text(panel, p);
                panel = ui_stats_screen.cells[1][0].info;
                f_name = sc_data->use_abbr ? sc_data->fields[1].field->abbr : sc_data->fields[1].field->name;
                if(strcmp(lv_label_get_text(panel), f_name))
                    lv_label_set_text(panel, f_name);
                // 3. avg run
                panel = ui_stats_screen.cells[2][0].title;
                get_display_fld_str(sc_data->fields[2].field, p, f2_to_char);
                lv_label_set_text(panel, p);
                panel = ui_stats_screen.cells[2][0].info;
                f_name = sc_data->use_abbr ? sc_data->fields[2].field->abbr : sc_data->fields[2].field->name;
                if(strcmp(lv_label_get_text(panel), f_name))
                    lv_label_set_text(panel, f_name);
            }
            statusbar_update();
        }
        if(count%200==0){
            lcd_ui_request_full_refresh(0);
        }
        ++count;
        ret += me->self->state.update_delay;
        old_screen_mode = current_screen_mode;
    return ret;
}

display_op_t screen_ops = {
    .sleep_screen = _sleep_screen,
    .update_screen = _update_screen,
    .uninit = display_uninit};

struct display_s *display_init(struct display_s *me) {
    ILOG(TAG, "[%s]", __func__);
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

uint32_t lcd_ui_screen_draw() {
    ILOG(TAG, "[%s]", __func__);
    IMEAS_START();
    _lvgl_lock(0);
    _lvgl_unlock();
    uint32_t task_delay_ms = screen_cb(0);
    IMEAS_END(TAG, "[%s] next delay %lu ms, screen_cb took: %llu us",  __FUNCTION__, task_delay_ms);
    uint32_t timer_delay_ms = lcd_lv_timer_handler();
    IMEAS_END(TAG, "[%s] next delay %lu ms, timer_handler took: %llu us",  __FUNCTION__, timer_delay_ms);
#if !defined(CONFIG_DISPLAY_DRIVER_ST7789)
    _lv_disp_refr_timer(NULL);
#endif
    if(timer_delay_ms > task_delay_ms) task_delay_ms = timer_delay_ms;
    IMEAS_END(TAG, "[%s] next delay %lu ms, refr_timer took: %llu us", __FUNCTION__, task_delay_ms);
    return task_delay_ms;
};

static bool lcd_ui_task_running = true;
static bool lcd_ui_task_paused  = false;
static bool lcd_ui_task_finished = 0;
uint32_t ms = 0;

void lcd_ui_task(void *args) {
    ILOG(TAG, "[%s] task starting", __FUNCTION__);
    uint32_t task_delay_ms = lcd_lv_timer_handler();
    while (lcd_ui_task_running) {
        if(!lcd_ui_task_paused)
            task_delay_ms = lcd_ui_screen_draw();
        if(lcd_ui_task_running) {
            ms = get_millis() + task_delay_ms;
            ILOG(TAG, "[%s] will delay for %lu ms", __FUNCTION__, task_delay_ms);
            while (lcd_ui_task_running && get_millis() < ms) {
#if defined(CONFIG_DISPLAY_DRIVER_ST7789)
                delay_ms(L_LVGL_TASK_MIN_DELAY_MS);
#else
                delay_ms(L_LVGL_TASK_MAX_DELAY_MS);
#endif
            }
        }
    }
    ILOG(TAG, "[%s] task finishing", __FUNCTION__);
    lcd_ui_task_finished = 1;
    vTaskDelete(NULL);
}

void cancel_lcd_ui_delay() {
    ILOG(TAG, "[%s]", __func__);
    ms=0;
}

uint32_t get_lcd_ui_count() {
    return count;
}

void pause_lcd_ui_task() {
    ILOG(TAG, "[%s]", __func__);
    lcd_ui_task_paused = true;
}

bool lcd_ui_task_is_paused() {
    return lcd_ui_task_paused;
}

void lcd_ui_request_full_refresh(bool force) {
    ILOG(TAG, "[%s] force: %d count:%lu last_refr_counter:%lu", __func__, force ?1:0, count, count_last_full_refresh);
    if(force || count==1 || count_last_full_refresh + 5  < count)
        display_epd_request_full_update();
    count_last_full_refresh = count;
}

// void lcd_ui_request_fast_refresh(bool force) {
//     ILOG(TAG, "[%s] force: %d count:%lu last_fast_refr_counter:%lu", __func__, force ?1:0, count, count_last_fast_refresh);
//     if(force || count_last_full_refresh + 5  < count || count_last_fast_refresh + 5  < count)
//         display_epd_request_fast_update();
// }

static void lcd_ui_start() {
    ILOG(TAG, "[%s]", __func__);
    ui_init();
#if !defined(CONFIG_DISPLAY_DRIVER_ST7789)
    lv_disp_t * disp = lv_disp_get_default();
    lv_timer_del(disp->refr_timer);
    disp->refr_timer = NULL;
#endif
}

TaskHandle_t lcd_ui_task_handle = 0;
void lcd_ui_start_task() {
    ILOG(TAG, "[%s]", __func__);
    xTaskCreate(lcd_ui_task, "lcd_ui_task", LCD_UI_TASK_STACK_SIZE, NULL, 5, &lcd_ui_task_handle);
}

static void lcd_ui_stop() {
    ILOG(TAG, "[%s]", __func__);
    IMEAS_START();
    uint32_t wait = get_millis() + 3000;
    lcd_ui_task_running = false;
    uint16_t i = 0;
    while (!lcd_ui_task_finished) {
        delay_ms(150);
        if (get_millis() > wait) {
            if(lcd_ui_task_handle)
                vTaskDelete(lcd_ui_task_handle);
            break;
        }
        ++i;
    }
    lcd_ui_screen_draw();
    if(current_screen_mode == SCREEN_MODE_SLEEP) {
        lcd_ui_screen_draw();
        //delay_ms(5000);
    }
    ui_deinit();
    IMEAS_END(TAG, "[%s] %hu 20 ms loops, total %llu us ",  __FUNCTION__, i);
}

void display_uninit(struct display_s *me) {
    ILOG(TAG, "[%s]", __func__);
    if (me && me->self && me->self->displayOK) {
        lcd_ui_stop();
        display_del();
        dspl = 0;
        memset(me->self, 0, sizeof(struct display_priv_s));
        memset(me, 0, sizeof(struct display_s));
    }
}
