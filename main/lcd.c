#include "private.h"

#include "lcd.h"

#if defined(CONFIG_DISPLAY_ENABLED)

#include <string.h>

#if defined(CONFIG_LOGGER_ADC_ENABLED)
#include <adc.h>
#endif

#include "driver_vendor.h"

#if defined(CONFIG_BMX_ENABLE)
#include "bmx280.h"
#include "bmx.h"
#endif
#if defined(CONFIG_LOGGER_BUTTON_ENABLED)
#include "button.h"
#endif
#include "context.h"
#include "dstat_screens.h"
#include "logger_config.h"
#include "numstr.h"
#if defined(CONFIG_GPS_LOG_ENABLED)
#include "gps_data.h"
#include "gps_user_cfg.h"
#endif
#if defined(CONFIG_UBLOX_ENABLED)
#include "ubx.h"
#endif
#if defined(CONFIG_LOGGER_WIFI_ENABLED)
#include "logger_wifi.h"
#endif

typedef struct sleep_scr_s {
    float *data;
    const char *info;
} sleep_scr_t;

typedef struct sat_count_s {
    uint8_t gps;
    uint8_t sbas;
    uint8_t galileo;
    uint8_t beidou;
    uint8_t qzss;
    uint8_t glonass;
    uint8_t navic;
} sat_count_t;

struct display_state_s {
    struct display_s *display;
    uint16_t update_delay;
    sat_count_t sat_count;
#if defined(CONFIG_BMX_ENABLE)
    float last_temp;
#endif
    char gps_status_str[32];
    char bat_status_str[32];
    uint32_t next_gps_str_update;
    uint32_t next_bat_str_update;
    screen_mode_t current_screen_mode;
    screen_mode_t old_screen_mode;
    uint8_t test_field;
    int16_t gps_image_angle;
#if defined(CONFIG_LCD_IS_EPD)
    int8_t offset_mark;
    uint8_t dots_counter;
    uint8_t offscreen_counter;
    uint16_t screen_mode_counter;
    uint8_t offset_mark_dir;
#endif
};

static struct display_s display;
static struct display_state_s display_state = { &display, 0, {0, 0, 0, 0, 0, 0, 0},
#if defined(CONFIG_BMX_ENABLE) 
0, 
#endif
"", "", 0, 0, SCREEN_MODE_UNKNOWN, SCREEN_MODE_UNKNOWN, 0, 0
#if defined(CONFIG_LCD_IS_EPD)
    , 0, 0, 0, 0, 0
#endif
};

static const char *TAG = "lcd";

extern struct context_rtc_s m_context_rtc;

#if defined(CONFIG_BMX_ENABLE)
extern bmx280_t *bmx280;
#endif

extern stat_screen_t sc_screens[];

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


//struct display_priv_s display_priv = {{false}, 0, 0, 0, 0, false};


#if !defined(CONFIG_LCD_IS_EPD)
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
#else
#define append_dots(p, max_dots, cur_dots) do{}while(0)
#endif

// static void statusbar_update();

static uint32_t _sleep_screen(const struct display_s *me, int choice) {
    ILOG(TAG, "[%s]", __func__);

    char tmp[24], *p = tmp;
    lv_label_t *panel;
    // if (_lvgl_lock(50)) {
    display_state.current_screen_mode = SCREEN_MODE_SLEEP;
#if defined(CONFIG_LCD_IS_EPD)
    if(!m_app_ctx.screen_auto_refresh && display_get_flush_count() == 0) {
        showSleepScreen();
    }
    else {
    if(m_context_rtc.RTC_voltage_bat < MINIMUM_VOLTAGE) {
        showBlankScreen(0);
    // } else if(m_context_rtc.RTC_voltage_bat < MINIMUM_VOLTAGE + 0.15) {
    //     showLowBatScreen();
    } 
    else {
#endif
        showSleepScreen();
        // statusbar_update();
        uint8_t num = m_context_rtc.RTC_Sail_Logo > 0 ? m_context_rtc.RTC_Sail_Logo - 1 : 0;
        const lv_img_dsc_t * img = sail_logo_img[num];
        lv_img_set_src(ui_sleep_screen.bottom_img, img ? img : sail_logo_img[0]);
        num = m_context_rtc.RTC_Board_Logo > 0 ? m_context_rtc.RTC_Board_Logo - 1 : 0;
        img = board_logo_img[num];
        lv_img_set_src(ui_sleep_screen.up_img, img ? img : board_logo_img[0]);

        for(int i = 0; i < 6; i++) {
            for(int j = 0; j < 2; j++) {
                f2_to_char(*sleep_scr_info_fields[j][i].data, p);
                lv_label_set_text(ui_sleep_screen.cells[i][j].title, p);
                lv_label_set_text(ui_sleep_screen.cells[i][j].info, sleep_scr_info_fields[j][i].info);
            }
        }
        lv_label_set_text(ui_sleep_screen.myid, m_context_rtc.RTC_Sleep_txt);
#if defined(CONFIG_LCD_IS_EPD)
    }
    }
#endif
    // lcd_ui_request_full_refresh(0); // first screen load will cause full refresh
    // display_state.current_screen_mode = display_state.old_screen_mode = SCREEN_MODE_SLEEP;
    // display_incr_buf_update_count();
    return 100;
}

static size_t bat_to_char(char *str, uint8_t full) {
    char *p = str + uint_to_char(full, str);
    *p++ = '%';
    *p = 0;
    return p - str;
}

static size_t temp_to_char(char *str) {
#if defined(CONFIG_BMX_ENABLE)
    ILOG(TAG, "[%s]", __func__);
    // TIMER_S
    float t=0;
    float p=0, h=0;
    esp_err_t ret = bmx_readings_f(&t, &p, &h);
    if(ret) {
        memcpy(str, "-.-", 3);
        *(str+3) = 0;
        return 3;
    }
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
    *pt = 176U;
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

static esp_err_t speed_info_bar_update() {  // info bar when config->screen.speed_large_font is 1 or 0
    const logger_config_t *config = m_app_ctx.config;
    if(!config) return ESP_ERR_INVALID_STATE;
    uint8_t field = config->screen.speed_field;          // default is in config.txt
    const uint8_t bar_max = 240;                                  // 240 pixels is volledige bar
    uint16_t bar_length = config->bar_length * 1000 / bar_max;  // default 100% length = 1852 m
    const uint8_t font_size = config->screen.speed_large_font;
#if defined(CONFIG_GPS_LOG_ENABLED)
    struct gps_context_s *gps = &m_app_ctx.ctx->gps;
    const struct gps_data_s * gps_data = &gps->Ublox;
    const struct ubx_config_s * ubx_dev = gps->ubx_device;

    if (config->screen.speed_field == 1) {  // only switch if config.field==1 !!!
        if (((int)(gps_data->total_distance / 1000000) % 10 == 0) && (gps_data->alfa_distance / 1000 > 1000))
            field = 5;  // indien x*10km, totale afstand laten zien
        // if(gps->S10.s_max_speed<(gps->S10.display_speed[5]*0.95))
        //     field=8;//if run slower dan 95% of slowest run, show 1h result
        if ((gps_data->alfa_distance / 1000 < 350) && (gps->alfa_window < 100))
            field = 3;  // first 350 m after gibe  alfa screen !!
        if (gps_data->alfa_distance / 1000 > config->bar_length)
            field = 4;  // run longer dan 1852 m, NM scherm !!
    } else if (config->screen.speed_field == 2) { // show Nautical Mile status
        if (gps_data->run_distance / 1000 > config->bar_length)
            field = 4;  // if run longer dan 1852 m, NM scherm !!
    } else if (config->screen.speed_field == 7) { // show alpha status
        if ((gps_data->alfa_distance / 1000 < 350) && (gps->alfa_window < 100))
            field = 3;  // first 350 m after gibe  alfa screen !!
        else
            field = 7;
    } else if (config->screen.speed_field == 8) { // show 1 hour status
        field = 8;
    } else if (config->screen.speed_field == 9) {  // 1 hour default, but first alfa, and if good run, last run
        field = 8;
        if (gps_data->alfa_distance / 1000 > config->bar_length)
            field = 4;  // run longer dan 1852 m, NM scherm !!
        if (gps->S10.s_max_speed > gps->S10.display_speed[5])
            field = 1;  // if run faster then slowest run, show AVG & run
        if ((gps_data->alfa_distance / 1000 < 350) && (gps->alfa_window < 100))
            field = 3;  // first 350 m after gibe  alfa screen !!
    }
#endif

    float s[] = {0, 0};
    const char *var[] = {0, 0};
    char val[][24] = {{0}, {0}}, *p;
#if defined(CONFIG_GPS_LOG_ENABLED)
    if (!ubx_dev->ready || !gps->signal_ok) {
        memcpy(val[0], scr_fld_2[0], 4);
        memcpy(val[1], scr_fld_2[0], 4);
        goto topoint;
    }
    if(gps->S2.avg_s < 1000) { // 1ms = 3.6km/h
        memcpy(val[0], scr_fld_2[1], 4);
        memcpy(val[1], scr_fld_2[1], 4);
        goto topoint;
    }
    // double s1 = 0, s2 = 0;
    else 
#endif
    if (field <= 2 || display_state.test_field == 2) { // 10 seconds stats
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
#if defined(CONFIG_GPS_LOG_ENABLED)
        topoint:
#endif
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

    else if ((field == 3 || display_state.test_field == 3)) {
        bar_length = 250 * 1000 / bar_max;  // full bar length with Alfa = 250 meter
#if defined(CONFIG_GPS_LOG_ENABLED)
        if ((gps->alfa_window < 99) && (gps_data->alfa_distance / 1000 < 255)) { // 250 meter na gijp
            if (gps->alfa_exit > 99)
                gps->alfa_exit = 99;  // begrenzen alfa_exit...
            var[0] =  scr_fld[0][1][0];
            f_to_char(gps->alfa_window, val[0], 0);
            var[1] =  scr_fld[1][1][1];
            f_to_char(gps->alfa_exit, val[1], 0);
        } else 
#endif
        { // alfa speed stats
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
    } else if (field == 4 || display_state.test_field == 4) { // nautical mile
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
    } else if (field == 5 || display_state.test_field == 5) { // total distance
        s[0] = avail_fields[41].value.num(); // total dist
        s[1] = avail_fields[63].value.num(); // m500 current run max speed
        var[0] = scr_fld[0][4][0];
        var[1] = scr_fld[0][4][1];
        if (s[0] > 100) 
            f1_to_char(s[0], val[0]);
        else
            f2_to_char(s[0], val[0]);
        if (s[1] > 100)
            f1_to_char(s[1], val[1]);
        else
            f2_to_char(s[1], val[1]);
    } else if (field == 6 || display_state.test_field == 6) { // 2 and 10 seconds stats
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
    } else if (field == 7 || display_state.test_field == 7) { // 30 minutes stats
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
    } else if (field == 8 || display_state.test_field == 8) { // 60 minutes stats
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

    uint32_t run_rectangle_length = 0;
#if defined(CONFIG_GPS_LOG_ENABLED)
    uint32_t millis = get_millis();
    uint32_t log_seconds = (millis - gps->start_logging_millis) / 1000;  // aantal seconden sinds loggen is gestart
    if (gps->S10.avg_s > 2000) {  // if the speed is higher then 2000 mm/s, reset the counter
        gps->low_speed_seconds = 0;
    }
    gps->low_speed_seconds++;
    if (gps->low_speed_seconds > 120) { // bar will be reset if the 10s speed drops under 2m/s for more then 120 s !!!!
        gps->start_logging_millis = millis;
    } 
    run_rectangle_length = (gps_data->alfa_distance / bar_length);  // 240 pixels is volledige bar, gps->ublox.alfa_distance zijn mm
    if (field == 7) {
        run_rectangle_length = log_seconds * 240 / 1800;
        if (log_seconds > 1800) {
            gps->start_logging_millis = millis;
        }
    }  // 30 minutes = full bar
    else if (field == 8) {
        run_rectangle_length = log_seconds * 240 / 3600;
        if (log_seconds > 3600) {
            gps->start_logging_millis = millis;
        }
    }  // 60 minutes = full bar
#endif
    // if (bar_length) {
        // screen.op->fillRect(&screen, offset, DISPLAY_TOP_PAD + bar_position, run_rectangle_length, SPEED_INFO_BOTTOM_HEIGHT, FG_COLOR);  // balk voor run_distance weer te geven...
        lv_bar_set_value(ui_speed_screen.bar, run_rectangle_length, 0);
    // }
    return ESP_OK;
}

static void statusbar_time_cb(lv_timer_t *timer) {
    ILOG(TAG, "[%s]", __func__);
#if defined(STATUS_PANEL_V1)
    ui_status_panel_t * statusbar = &ui_status_panel;
    if(!statusbar->parent) {
        return;
    }
#else
    lv_statusbar_t * statusbar = (lv_statusbar_t *)ui_StatusPanel;
#endif
    struct tm *tm = &m_context_rtc.rtc_tm;
    char tmp[24]={0}, *p = tmp;
    lv_obj_t *panel;
    if ((panel = statusbar->time_label)) {
        if (statusbar->viewmode==0 || statusbar->viewmode == 2) {
            memset(tm, 0, sizeof(struct tm));
            getLocalTime(tm, 0);
        }
        if(tm->tm_year > 70) {
            p += time_to_char_hm(tm->tm_hour, tm->tm_min, p);
            if(statusbar->viewmode==1) {
                *p++ = ' ';
                p += date_to_char(tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900, 0, p), *p = 0;
            }
        }
#if (C_LOG_LEVEL < 2)
            DLOG(TAG, "** [%s] (date)time: %s {d: %d, m:%d, y:%d} ** \n", __func__, tmp, tm->tm_mday, tm->tm_mon, tm->tm_year);
#endif
        if(strcmp(lv_label_get_text(panel), &(tmp[0]))) {
            lv_label_set_text(panel, &(tmp[0]));
        }
    }
}

static void statusbar_temp_cb(lv_timer_t *timer) {
    ILOG(TAG, "[%s]", __func__);
#if defined(STATUS_PANEL_V1)
    ui_status_panel_t * statusbar = &ui_status_panel;
    if(!statusbar->parent) {
        return;
    }
#else
    lv_statusbar_t * statusbar = (lv_statusbar_t *)ui_StatusPanel;
#endif
    char tmp[12], *p = tmp;
    lv_obj_t *panel;
    if ((panel = statusbar->temp_label)) {
#if defined(CONFIG_BMX_ENABLE)
        size_t len = temp_to_char(p);
#if (CONFIG_LOGGER_COMMON_LOG_LEVEL > 1)
        if(!len && (len == 3 && *p == '-' && *p+2 == '-')) {
#else
        if (!len) {
#endif
            if (!lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN))
                lv_obj_add_flag(panel, LV_OBJ_FLAG_HIDDEN);
        }
        else {
            if (lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN))
                lv_obj_clear_flag(panel, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text(panel, p);
        }
#endif
    }
}

static void statusbar_bat_cb(lv_timer_t *timer) {
    ILOG(TAG, "[%s]", __func__);
#if defined(STATUS_PANEL_V1)
    ui_status_panel_t * statusbar = &ui_status_panel;
    if(!statusbar->parent) {
        return;
    }
#else
    lv_statusbar_t * statusbar = (lv_statusbar_t *)ui_StatusPanel;
#endif
    float bat = m_context_rtc.RTC_voltage_bat;
    char tmp[24], *p = tmp;
    const char *r;
    lv_obj_t *panel;
    uint8_t full = bat > 4.8 ? 110 
                : bat >= 4.2 ? 101
#if defined(CONFIG_LOGGER_ADC_ENABLED)
                : calc_bat_perc_v(bat);
#else
                : 51;
#endif
    if ((panel = statusbar->bat_label)) {
        if(full<100) *p++=' ';
        p += bat_to_char(p, full > 100 ? 100 : full);
        r = lv_label_get_text(panel);
        if(!r || memcmp(r,&tmp[0],4))
            lv_label_set_text(panel, tmp);
    }

    if ((panel = statusbar->bat_image)) {
        const char * s = adc_on_ac() ? LV_SYMBOL_CHARGE
                : full < 10  ? LV_SYMBOL_BATTERY_EMPTY
                : full < 20  ? LV_SYMBOL_BATTERY_1
                : full < 60  ? LV_SYMBOL_BATTERY_2
                : full < 101 ? LV_SYMBOL_BATTERY_FULL
                : full < 110 ? LV_SYMBOL_CHARGE
                            : LV_SYMBOL_USB;
        lv_label_set_text(panel, s);
// #if !defined(CONFIG_LCD_IS_EPD)
//         lv_obj_set_style_text_color(panel, full>20 ? lv_color_hex(0xFFFFFF) : full>10 ? lv_color_hex(0xEECE44) : lv_color_hex(0xE32424), LV_PART_MAIN | LV_STATE_DEFAULT );
//         lv_obj_set_style_text_opa(panel, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
// #endif
    }
}

static void statusbar_gps_cb(lv_timer_t *timer) {
    ILOG(TAG, "[%s]", __func__);
#if defined(STATUS_PANEL_V1)
    ui_status_panel_t * statusbar = &ui_status_panel;
    if(!statusbar->parent) {
        return;
    }
#else
    lv_statusbar_t * statusbar = (lv_statusbar_t *)ui_StatusPanel;
#endif
    const struct main_ctx_s *ctx = 0;
    if(timer)
        ctx = timer->user_data;
    else
        ctx = &m_app_ctx;
    if(!ctx || ! ctx->ctx || !ctx->ctx->gps.ubx_device) return;
    const ubx_config_t *ubx_dev = ctx->ctx->gps.ubx_device;
    lv_obj_t *panel;
    if ((panel = statusbar->gps_image)) {
        char tmp[24]={0}, *p = tmp;
        uint8_t numsat = ubx_dev->ubx_msg.navPvt.numSV;
        if (ctx->app_mode == APP_MODE_GPS && ubx_dev->is_on) {
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
            // if (!ubx_dev || !ubx_dev->ready){
            //     memcpy(p, "-n-", 3);
            // }
            // else if(!ubx_dev->signal_ok) {
            //     uint8_t qp = gblink;
            //     memcpy(p, "gps   ", 6);
            //     *(p+6)=0;
            //     p += 3;
            //     while(qp--) {
            //         *p++ = '.';
            //     }
            //     gblink = gblink==3 ? 0 : gblink+1;
            // }
            
            // else 
            if(numsat > 0) {
                p += xltoa(numsat, p);
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
        // if (numsat >= 4) {
        //     lv_obj_set_style_img_recolor(panel, lv_color_hex(0xA9B7B9), LV_PART_MAIN | LV_STATE_DEFAULT);
        //     lv_obj_set_style_img_recolor_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        // } else if (ubx_dev->ready) {
        //     lv_obj_set_style_img_recolor_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        //     lv_obj_set_style_img_recolor(panel, lv_color_hex(0xEECE44), LV_PART_MAIN | LV_STATE_DEFAULT);
        // } else {
        //     if (ubx_dev->rtc_conf->hw_type == UBX_TYPE_UNKNOWN) {
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

lv_timer_t * gps_timer = 0;
lv_timer_t * time_timer = 0;
lv_timer_t * bat_timer = 0;
lv_timer_t * temp_timer = 0;

void update_lv_timers() {
    ILOG(TAG, "[%s]", __func__);
    if(m_app_ctx.app_mode == APP_MODE_SLEEP) {
        statusbar_time_cb(0);
        statusbar_bat_cb(0);
    }
    else {
        if(!gps_timer && m_app_ctx.app_mode == APP_MODE_GPS) {
            gps_timer = lv_timer_create(statusbar_gps_cb, 1000, &m_app_ctx);
        }
        else if(gps_timer && m_app_ctx.app_mode != APP_MODE_GPS) {
            lv_timer_del(gps_timer);
            gps_timer = 0;
        }
        if(!time_timer) {
            statusbar_time_cb(0);
            time_timer = lv_timer_create(statusbar_time_cb, 1000, &m_app_ctx);
        }
        if(!bat_timer) {
            statusbar_bat_cb(0);
            bat_timer = lv_timer_create(statusbar_bat_cb, 1000, &m_app_ctx);
        }
#if defined(CONFIG_BMX_ENABLE)
        if(!temp_timer) temp_timer = lv_timer_create(statusbar_temp_cb, 1000, 0);
#endif
    }
}

void stop_lv_timers() {
    if(gps_timer) {
        lv_timer_del(gps_timer);
        gps_timer = 0;
    }
    if(time_timer) {
        lv_timer_del(time_timer);
        time_timer = 0;
    }
    if(bat_timer) {
        lv_timer_del(bat_timer);
        bat_timer = 0;
    }
#if defined(CONFIG_BMX_ENABLE)
    if(temp_timer) {
        lv_timer_del(temp_timer);
        temp_timer = 0;
    }
#endif
}

// static void statusbar_update() {
//     // battery //
//     statusbar_bat_cb(0);
//     // // time //
//     statusbar_time_cb(0);
//     // // temp //
//     statusbar_temp_cb(0);
//     // // sdcard image //
//     // statusbar_sdcard_cb(0);
//     // // gps/wifi image //
//     statusbar_gps_cb(0);
// }

static void update_sat_count(const struct ubx_config_s *ubx_dev) {
    if(!ubx_dev) return;
    const struct nav_sat_s *nav_sat = &(ubx_dev->ubx_msg.nav_sat);
    const struct svs_nav_sat_s * sat = 0;
    memset(&(display_state.sat_count), 0, sizeof(sat_count_t));
    for(uint8_t i=0; i < nav_sat->numSvs; i++) {
        sat = &nav_sat->sat[i];
#if (C_LOG_LEVEL < 2)
        DLOG(TAG, "sat[%hhu]: %hhu, %hhu, %hhu, %hhu, %hu, %lu %lu %lu\n", i, sat->gnssId, sat->svId, sat->cno, sat->elev, sat->azim, sat->flags, (sat->flags & 0x08), (sat->flags & 0x07));
#endif
        if((sat->flags & 0x08) == 0 || (sat->flags & 0x07) < 4)
            continue;
        switch(sat->gnssId) {
            case 0:
                display_state.sat_count.gps++;
                break;
            case 1:
                display_state.sat_count.sbas++;
                break;
            case 2:
                display_state.sat_count.galileo++;
                break;
            case 3:
                display_state.sat_count.beidou++;
                break;
            case 5:
                display_state.sat_count.qzss++;
                break;
            case 6:
                display_state.sat_count.glonass++;
                break;
            case 7:
                display_state.sat_count.navic++;
                break;
            default:
                break;
        }
    }
#if (C_LOG_LEVEL < 2)
    ILOG(TAG, "gnss: %hhu, count: %hhu, G:%d, S:%d, E:%d, B:%d, Q:%d, R:%d, N:%d", ubx_dev->rtc_conf->gnss, nav_sat->numSvs, display_state.sat_count.gps, display_state.sat_count.sbas, display_state.sat_count.galileo, display_state.sat_count.beidou, display_state.sat_count.qzss, display_state.sat_count.glonass, display_state.sat_count.navic);
#endif
}

static size_t update_gps_info_row_str(const struct ubx_config_s *ubx_dev, char * p) {
    if(!ubx_dev) return 0;
    char * pc = p;
    if(ubx_dev->config_progress) {
        memcpy(pc, "initializing", 12), pc += 12;
    } else if(ubx_dev->ready) {
        if(ubx_dev->rtc_conf->hw_type == UBX_HW_TYPE_DEFAULT) {
            memcpy(pc, "gps not found", 13), pc += 13;
            goto end;
        }
        update_sat_count(ubx_dev);
        uint8_t gnss = ubx_dev->rtc_conf->gnss;
        pc += xultoa(ubx_dev->ubx_msg.navPvt.numSV, pc);
        memcpy(pc, "sat", 3), pc += 3;
        if((gnss & (1 << 0))!=0) {
            *pc++ = ' ';
            *pc++ = 'G';
            pc += xultoa(display_state.sat_count.gps, pc);
        }
        if((gnss & (1 << 2))!=0) {
            *pc++ = ' ';
            *pc++ = 'E';
            pc += xultoa(display_state.sat_count.galileo, pc);
        }
        if((gnss & (1 << 3))!=0) {
            *pc++ = ' ';
            *pc++ = 'B';
            pc += xultoa(display_state.sat_count.beidou, pc);
        }
        if((gnss & (1 << 6))!=0) {
            *pc++ = ' ';
            *pc++ = 'R';
            pc += xultoa(display_state.sat_count.glonass, pc);
        }
    }
    else {
        memcpy(pc, "gps not ready", 13), pc += 13;
    }
    end:
    *pc = 0;
    return pc - p;
}

static size_t update_gps_desc_row_str(const struct gps_context_s * gps, char * p) {
    char * pb = p;
    memcpy(pb, "Bat: ", 5), pb += 5;
    pb += f3_to_char(m_context_rtc.RTC_voltage_bat, pb);
    memcpy(pb, "V ", 2), pb += 2;
    if(gps->first_fix){
        memcpy(pb, " fx: ", 5), pb += 5;
        pb += xultoa(gps->first_fix, pb);
        *pb++ = 's';
    }
    if(gps->lost_frames) {
        memcpy(pb, " lst: ", 6), pb += 6;
        pb += xultoa(gps->lost_frames, pb), *pb=0;
    }
    return pb - p;
}

static uint32_t _update_screen(const struct display_s *me, const screen_mode_t screen_mode, void *arg) {
    uint32_t buf_update_count = display_get_buf_update_count();
#if (C_LOG_LEVEL < 3)
    ILOG(TAG, "[%s] buf_update_count:%ld mode: %d", __func__, buf_update_count, screen_mode);
#endif
    uint32_t ret = 0;
    UNUSED_PARAMETER(ret);
    if(display_refresh_lock(portMAX_DELAY) == pdTRUE) {
        logger_config_t *config = m_app_ctx.config;
        char str[24] = {0}, *p = str, str1[32]={0}, *pb = str1, str2[32]={0}, *pc = str2;
        bool is_gps_stat_screen = (screen_mode > 0 && screen_mode < 10);
        display_state.update_delay = 500;
        // ESP_LOGI(TAG, "update screen: mode:%" PRIu8 ", update nr:%lu", screen_mode, buf_update_count);
        float gpsspd;
        int state = (int)arg;
        lv_obj_t *panel, *parent;
        stat_screen_t *sc_data = 0;
        const struct gps_context_s *gps = &m_app_ctx.ctx->gps;
        const struct ubx_config_s *ubx_dev = gps->ubx_device;
        const char *gpsstr = 0;
        const lv_img_dsc_t *img_src = 0;
        const char * scr_mode_str = "Screen mode ";
        display_state.current_screen_mode = screen_mode; 
#if defined(CONFIG_LCD_IS_EPD)
#if defined(USE_DOTS_COUNTER)
        if(display_state.dots_counter && display_state.old_screen_mode != screen_mode) {
            display_state.dots_counter = 0;
        }
#endif
        if(display_state.old_screen_mode == screen_mode) {
            ++display_state.screen_mode_counter;
        }
        else {
            display_state.screen_mode_counter = 0;
            display_state.offset_mark = 0;
        }
#endif
        int8_t offset = 
#if defined(CONFIG_LCD_IS_EPD)
            display_state.offset_mark;
#else
            0;
#endif
        switch (screen_mode) {
            case SCREEN_MODE_GPS_TROUBLE:
                showGpsTroubleScreen();
                break;
            case SCREEN_MODE_GPS_INIT:
            case SCREEN_MODE_GPS_READY:
                gpsstr = ubx_dev ? ubx_chip_str(ubx_dev) : 0;
                if(!gpsstr) {
                    p=str;
                    memcpy(p, "NO GPS YET", 6), p += 6;
                }
                else if(!strcmp(gpsstr, "UNKNOWN")) {
                    p=str;
                    memcpy(p, "GPS", 3), p += 3;
                    if(ubx_dev->ready && ubx_dev->rtc_conf->hw_type == UBX_HW_TYPE_DEFAULT) {
                        img_src = &near_me_disabled_bold_48px;
                        memcpy(p, " -", 2), p += 2;
                    }
                }
                else {
                    p += strlen(gpsstr);
                    memcpy(&str[0], gpsstr, p-&str[0]);
                    *p++ = '@';
                    p += xltoa(ubx_dev->rtc_conf->output_rate, p);
                    memcpy(p, "Hz", 2), p += 2;
                }
                *p = 0;
                
                uint32_t now = get_millis();
                if(now > display_state.next_gps_str_update) {
                    display_state.next_gps_str_update = now + 2000;
                    update_gps_info_row_str(ubx_dev, &display_state.bat_status_str[0]);
                    update_gps_desc_row_str(&m_app_ctx.ctx->gps, &display_state.gps_status_str[0]);
                }
                ui_set_main_cnt_offset(&ui_info_screen.screen, offset);
                showGpsScreen(&str[0], &display_state.bat_status_str[0], &display_state.gps_status_str[0], img_src, display_state.gps_image_angle);
                // statusbar_update();

// #if !defined(CONFIG_LCD_IS_EPD)
//                 if(screen_mode==SCREEN_MODE_GPS_INIT) {
//                     lv_obj_set_style_img_recolor(ui_info_screen.info_img, lv_color_hex(0x162B2E), LV_PART_MAIN | LV_STATE_DEFAULT);
//                     lv_obj_set_style_img_recolor_opa(ui_info_screen.info_img, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//                 }
//                 else if(screen_mode==SCREEN_MODE_GPS_READY) {
//                     //104951
//                     lv_obj_set_style_img_recolor(ui_info_screen.info_img, lv_color_hex(0x104951), LV_PART_MAIN | LV_STATE_DEFAULT);
//                     lv_obj_set_style_img_recolor_opa(ui_info_screen.info_img, 255, LV_PART_MAIN | LV_STATE_DEFAULT);               
//                 }
//                 else if(screen_mode==SCREEN_MODE_GPS_TROUBLE) {
//                     lv_obj_set_style_img_recolor(ui_info_screen.info_img, lv_color_hex(0xE32424), LV_PART_MAIN | LV_STATE_DEFAULT);
//                     lv_obj_set_style_img_recolor_opa(ui_info_screen.info_img, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
//                 }
//                 gps_image_angle += 225;
//                 if (gps_image_angle >= 3600)
//                     gps_image_angle = 0;
// #endif
                break;
            case SCREEN_MODE_PUSH:
                struct push_forwarder_s * pf = (struct push_forwarder_s*)arg;
                showPushScreen(pf->state, pf->title);
                break;
            case SCREEN_MODE_SD_TROUBLE:
                showSdTroubleScreen();
                break;
            case SCREEN_MODE_LOW_BAT:
                link_for_low_bat:
                showLowBatScreen();
                break;
            case SCREEN_MODE_FW_UPDATE:
                const v_settings_t *s = arg;
                const struct m_config_item_s *i = (const struct m_config_item_s *)s->settings_data;
                ui_set_main_cnt_offset(&ui_info_screen.screen, offset);
                showFwUpdateScreen(s->name, i->name, i->desc);
                break;
            case SCREEN_MODE_SHUT_DOWN:
                ui_flush_screens(&ui_init_screen.screen);
                const char *title = m_app_ctx.ctx->request_restart ? "Reboot device" : m_app_ctx.ctx->Shut_down_Save_session ? 0 : "Going to sleep";
                memcpy(&str[0], title ? title : "Save session", title ? strlen(title) : 12), p += title ? strlen(title) : 12;
#ifdef USE_DOTS_COUNTER
                append_dots(p, 3, &m_app_ctx.ctx->dots_counter);
#endif
                if(m_app_ctx.ctx->low_bat_count > 5) {
                    //current_screen_mode = SCREEN_MODE_LOW_BAT;
                    goto link_for_low_bat;
                } else {
                    //current_screen_mode = SCREEN_MODE_OFF_SCREEN;
                    if(!title) {
                        float session_time = avail_fields[59].value.num();
                        float distance = avail_fields[41].value.num();
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
                display_state.update_delay = 
#if defined(CONFIG_LCD_IS_EPD)
                    display_state.screen_mode_counter < 2 ? 100 : 
#endif
                    500;
                break;
            case SCREEN_MODE_BOOT:
                // ui_flush_screens(&ui_init_screen.screen);
                // current_screen_mode = SCREEN_MODE_BOOT;
#if defined(CONFIG_LCD_IS_EPD)
                 if(m_app_ctx.screen_auto_refresh || display_get_flush_count() > 0) {
#endif
                    memcpy(p, "Booting", 7), p += 7;
#ifdef USE_DOTS_COUNTER
                    append_dots(p, 3, &m_app_ctx.ctx->dots_counter);
#endif
                    showBootScreen(&str[0]);
#if defined(CONFIG_LCD_IS_EPD)
                }
                 else {
                    // display_request_full_refresh(0); // first screen load will cause full refresh
                    showBlankScreen(0);
                 }
#endif
                if (buf_update_count < 2) display_state.update_delay = 100;
                break;
            case SCREEN_MODE_SPEED_1:
            link_for_screen_mode_speed_2:
                gpsspd = gps_last_speed_smoothed(2) * c_gps_cfg.speed_calibration;
                if (!ubx_dev || !ubx_dev->ready || !gps->signal_ok) {
                    memcpy(p, "-.--", 4);
                    *(p+4) = 0;
                }
#if defined(CONFIG_GPS_LOG_ENABLED)
                else if(gps->S2.avg_s < 1000) {
                    memcpy(p, "0.00", 4);
                    *(p+4) = 0;
                }
#endif
                else {
                    display_state.update_delay = 100;
                    if(gpsspd < 100)
                        f2_to_char(gpsspd, p);
                    else
                        f1_to_char(gpsspd, p);
                }
                if(display_state.update_delay>100)
                    ui_set_main_cnt_offset(&ui_speed_screen.screen, offset);
                showSpeedScreen();
                panel = ui_speed_screen.speed;
                lv_label_set_text(panel, p);
                speed_info_bar_update();
                // statusbar_update();

                break;
            case SCREEN_MODE_SPEED_2:
                goto link_for_screen_mode_speed_2;
                break;
            case SCREEN_MODE_SPEED_STATS_1:
#if (C_LOG_LEVEL < 2)
                DLOG(TAG, "[%s] %s, stat 10s: %d\n", __func__, scr_mode_str, screen_mode);
#endif
                sc_data = &sc_screens[0];
                break;
            case SCREEN_MODE_SPEED_STATS_2:
#if (C_LOG_LEVEL < 2)
                DLOG(TAG, "[%s] %s, stat 2s: %d\n", __func__, scr_mode_str, screen_mode);
#endif
                sc_data = &sc_screens[1];
                break;
            case SCREEN_MODE_SPEED_STATS_3:
#if (C_LOG_LEVEL < 2)
                DLOG(TAG, "[%s] %s, stat 250m: %d\n", __func__, scr_mode_str, screen_mode);
#endif
                sc_data = &sc_screens[2];
                break;
            case SCREEN_MODE_SPEED_STATS_4:
#if (C_LOG_LEVEL < 2)
                DLOG(TAG, "[%s] %s, stat 500m: %d\n", __func__, scr_mode_str, screen_mode);
#endif
                sc_data = &sc_screens[3];
                break;
            case SCREEN_MODE_SPEED_STATS_5:
#if (C_LOG_LEVEL < 2)
                DLOG(TAG, "[%s] %s, stat 1852m: %d\n", __func__, scr_mode_str, screen_mode);
#endif
                sc_data = &sc_screens[4];
                break;
            case SCREEN_MODE_SPEED_STATS_6:
#if (C_LOG_LEVEL < 2)
                DLOG(TAG, "[%s] %s, stat A500: %d\n", __func__, scr_mode_str, screen_mode);
#endif
                sc_data = &sc_screens[5];
                break;
            case SCREEN_MODE_SPEED_STATS_7:
#if (C_LOG_LEVEL < 2)
                DLOG(TAG, "[%s] %s, stat 10sec avg: %d\n", __func__, scr_mode_str, screen_mode);
#endif
                    sc_data = &sc_screens[6];
                    break;
            case SCREEN_MODE_SPEED_STATS_8:
#if (C_LOG_LEVEL < 2)
                DLOG(TAG, "[%s] %s, stat stats: %d\n", __func__, scr_mode_str, screen_mode);
#endif
                sc_data = &sc_screens[7];
                break;
            case SCREEN_MODE_SPEED_STATS_9:
#if (C_LOG_LEVEL < 2)
                DLOG(TAG, "[%s] %s, stat alpha avg: %d\n", __func__, scr_mode_str, screen_mode);
#endif
                sc_data = &sc_screens[8];
                break;
            case SCREEN_MODE_WIFI_START:
            case SCREEN_MODE_WIFI_AP:
            case SCREEN_MODE_WIFI_STATION:
#if (C_LOG_LEVEL < 2)
                DLOG(TAG, "[%s] %s, wifi %d\n", __func__, scr_mode_str, screen_mode);
#endif
                display_state.update_delay = 600;
#if defined(CONFIG_LOGGER_WIFI_ENABLED)
                struct m_wifi_context *wctx = m_app_ctx.wifi_ctx;
                if(wctx->s_ap_connection) {
                    memcpy(p, wctx->ap.ssid, strlen(wctx->ap.ssid)), p += strlen(wctx->ap.ssid);
                    // pb += sprintf(pb, "%hhu.%hhu.%hhu.%hhu", wctx->ap.ipv4_address[0], wctx->ap.ipv4_address[1], wctx->ap.ipv4_address[2], wctx->ap.ipv4_address[3]);
                }
                if (wctx->s_sta_connection && wctx->s_sta_connected) {
                    if(wctx->s_ap_connection) memcpy(p, " / ", 3), p += 3;
                    memcpy(p, wctx->stas[wctx->s_sta_num_connect].ssid, strlen(wctx->stas[wctx->s_sta_num_connect].ssid)), p += strlen(wctx->stas[wctx->s_sta_num_connect].ssid);
                    // if(wctx->s_ap_connection) memcpy(pb, " / ", 3), pb += 3;
                    // sprintf(pb, "%hhu.%hhu.%hhu.%hhu", wctx->stas[wctx->s_sta_num_connect].ipv4_address[0], wctx->stas[wctx->s_sta_num_connect].ipv4_address[1], wctx->stas[wctx->s_sta_num_connect].ipv4_address[2], wctx->stas[wctx->s_sta_num_connect].ipv4_address[3]), pb+=strlen(pb);
                }
                if(wctx->s_ap_connection || wctx->s_sta_connection) {
                    memcpy(pb, wctx->hostname, strlen(wctx->hostname)), pb += strlen(wctx->hostname);
                    memcpy(pb, ".local", 11), pb+=11;
                }
                if(wctx->s_ap_connection) memcpy(pc, "password", 11), pc+=11;
                *pc = 0;
#endif
                ui_set_main_cnt_offset(&ui_info_screen.screen, offset);
                
                showWifiScreen(&str[0], &str1[0], &str2[0]);
                // statusbar_update();
#if !defined(CONFIG_LCD_IS_EPD)
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
                    const struct m_config_item_s *i = (const struct m_config_item_s *)s->settings_data;
                    if(!i) goto plain_setting;
                    ui_set_main_cnt_offset(&ui_info_screen.screen, offset);
                    showSettingsScreen(s->name, i->name, i->desc);
                }
                else {
                    plain_setting:
                    showSettingsScreen("Settings","-","-");
                }
                break;
            case SCREEN_MODE_SLEEP:
                _sleep_screen(display_state.display, 0);
                break;
            default:
                break;
        }
        if (is_gps_stat_screen) {
            const char * f_name = 0;
            uint8_t r, c, n, rows, cols;
            if (sc_data->num_fields == 6) {
#if defined(CONFIG_SSD168X_PANEL_SSD1681)
#if (C_LOG_LEVEL < 2)
                DLOG(TAG, "[%s] stats panel: 6Row x 1Slot\n", __func__);
#endif
                rows = 6, cols = 1;
#else
#if (C_LOG_LEVEL < 2)
                DLOG(TAG, "[%s] stats panel: 3Row x 2Slot\n", __func__);
#endif
                rows = 3, cols = 2;
#endif
            } 
            else if (sc_data->num_fields == 4) {
#if defined(CONFIG_SSD168X_PANEL_SSD1681)
#if (C_LOG_LEVEL < 2)
                DLOG(TAG, "[%s] stats panel: 4Row x 1Slot\n", __func__);
#endif
                rows = 4, cols = 1;
#else
#if (C_LOG_LEVEL < 2)
                DLOG(TAG, "[%s] stats panel: 2Row x 2Slot\n", __func__);
#endif
                rows = 2, cols = 2;
#endif
            } else if (sc_data->num_fields == 2) {
#if (C_LOG_LEVEL < 2)
                DLOG(TAG, "[%s] stats panel: 2Row x 1Slot\n", __func__);
#endif
                rows = 2, cols = 1;
            } else {
#if (C_LOG_LEVEL < 2)
                DLOG(TAG, "[%s] stats panel: 3Row x 1Slot\n", __func__);
#endif
                rows = 3, cols = 1;
            }
            ui_set_main_cnt_offset(&ui_stats_screen.screen, offset);
            loadStatsScreen(rows,cols);
            for(c=0; c < cols; c++) {
                for(r = 0; r < rows; r++) {
                    n = c*rows+r;
                    panel = ui_stats_screen.cells[r][c].title;
                    get_display_fld_str(sc_data->fields[n].field, p, f2_to_char);
                    lv_label_set_text(panel, p);
                    panel = ui_stats_screen.cells[r][c].info;
                    f_name = sc_data->use_abbr ? sc_data->fields[n].field->abbr : sc_data->fields[n].field->name;
                    if(strcmp(lv_label_get_text(panel), f_name))
                        lv_label_set_text(panel, f_name);
                }
            }
            // statusbar_update();
        }
#if defined(CONFIG_LCD_IS_EPD)
        // ILOG(TAG, "[%s] done, %ld mode: %d offset: %hhd, dir: %hhu, screen_mode_counter: %hu", __func__, buf_update_count, screen_mode, display_state.offset_mark, display_state.offset_mark_dir, display_state.screen_mode_counter);
        if(display_state.screen_mode_counter && display_state.screen_mode_counter%10 == 0) {
            if(display_state.offset_mark > 4) {
                display_state.offset_mark_dir = 1;
            }
            else if(display_state.offset_mark == 0) {
                display_state.offset_mark_dir = 0;
            }
            if(!display_state.offset_mark_dir) ++display_state.offset_mark;
            else --display_state.offset_mark;
        }
        if(display_state.display->task_resumed_for_times) --display_state.display->task_resumed_for_times;
        if(display_state.display->task_fast_refresh_on_time==buf_update_count) {
            display_request_fast_refresh();
            display_task_cancel_req_fast_refresh();
        }
        else if(display_state.display->task_full_refresh_on_time==buf_update_count) {
            display_request_full_refresh(display_state.display->task_full_refresh_on_time_force);
            display_task_cancel_req_full_refresh();
        }
        else if(buf_update_count > 0 && buf_update_count%100==0){
            display_request_full_refresh(0);
        }
#endif
        display_incr_buf_update_count();
        ret += display_state.update_delay;
        display_state.old_screen_mode = display_state.current_screen_mode;
        display_refresh_unlock();
    }
    return ret;
}

uint32_t screen_cb(void* arg) {
    main_ctx_t *ctx = &m_app_ctx;
    // const uint32_t lcd_count = get_lcd_ui_count();
    uint32_t buf_update_count = display_get_buf_update_count();
#if (C_LOG_LEVEL < 2)
    ILOG(TAG, "[%s] %ld app_mode: %s, cur_screen: %s, next_screen: %s", __func__, buf_update_count, app_mode_str[ctx->app_mode], cur_screen_str[ctx->cur_screen], cur_screen_str[ctx->next_screen]);
#endif
    DMEAS_START();
    // struct display_s *dspl = &display;
    uint32_t delay=0;
    if(!display_state.display || !display_state.display->op) {
        goto end;
    }
    bool run_is_active = false;
    struct gps_context_s *gps = &m_app_ctx.ctx->gps;
#ifdef CONFIG_UBLOX_ENABLED
    const struct ubx_config_s *ubx_dev = gps->ubx_device;
#endif
    if(ctx->app_mode == APP_MODE_GPS) {
        run_is_active = (ctx->config && gps && gps->signal_ok && gps->gps_speed / 1000.0f >= m_app_ctx.config->screen.stat_speed);
        if (run_is_active && ctx->next_screen != CUR_SCREEN_NONE){
            ctx->next_screen = CUR_SCREEN_NONE;
        }
        ctx->stat_screen_count = m_app_ctx.ctx->stat_screen_count;
        if (ctx->stat_screen_count > get_stat_screens_count())
            ctx->stat_screen_count = get_stat_screens_count();
    }
    
    // display_op_t *op = display_state.display->op;
    int32_t now, emillis, elapsed;
#if defined(CONFIG_LOGGER_BUTTON_ENABLED)
    if(ctx->button_down) {
        if(ctx->button_press_mode > 0 && btns[0].button_down) {
            const char *s = 
            (ctx->button_press_mode==3) ? "Reboot" : 
            (ctx->button_press_mode==2) ? "Mode change" : 
            ((ctx->next_screen == CUR_SCREEN_SETTINGS) ? "Next group" : ctx->next_screen == CUR_SCREEN_FW_UPDATE ? "Do update" : "Shut down");
            delay = _update_screen(display_state.display, SCREEN_MODE_PUSH, (void*)(&((struct push_forwarder_s){ ctx->button_press_mode, s })));
            goto end;
        }
    }
#endif

    if(m_app_ctx.ctx->firmware_update_started>=1 && ctx->next_screen == CUR_SCREEN_FW_UPDATE) {
        struct m_config_item_s item = {.name = "Confirm"};
        if(m_app_ctx.ctx->fw_update_is_allowed || m_app_ctx.ctx->firmware_update_started == 1) {
            item.name = "Updating...";
            item.desc = "Please wait";
            item.value = 1;
            item.pos = 0;
        }
        else if(ctx->fw_update_screen == 0) {
            item.desc = "Update";
            item.value = 0;
            item.pos = 0;
        } else if(ctx->fw_update_screen == 1){
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
        delay=_update_screen(display_state.display, SCREEN_MODE_FW_UPDATE, (void*)&s);
        ctx->cur_screen = CUR_SCREEN_FW_UPDATE;
        goto end;
    }

    // if(low_bat_countdown) {
    //     if(low_bat_countdown - get_millis() < 10000) {
    //         m_context.low_bat_count = 10;
    //     }
    // }
    else if(ctx->app_mode == APP_MODE_SLEEP){
        // delay = _update_screen(display_state.display, SCREEN_MODE_SLEEP, 0);
        _sleep_screen(display_state.display, 0);
        ctx->cur_screen = CUR_SCREEN_SLEEP_SCREEN;
        display_state.old_screen_mode = SCREEN_MODE_SLEEP;
        display_incr_buf_update_count();
        goto end;
    }
    else if ((!m_app_ctx.screen_auto_refresh && display_get_flush_count() < 2) || ctx->app_mode == APP_MODE_BOOT) {
#if (C_LOG_LEVEL < 2)
        ILOG(TAG, "[%s] Boot screen requested lcd_count: %lu, app_mode: %s", __func__, buf_update_count, app_mode_str[ctx->app_mode]);
#endif
        delay = _update_screen(display_state.display, SCREEN_MODE_BOOT, 0);
        ctx->cur_screen = CUR_SCREEN_BOOT;
        goto end;
    } 
    
    else if (ctx->app_mode == APP_MODE_SHUT_DOWN || ctx->app_mode == APP_MODE_RESTART || m_app_ctx.ctx->request_shutdown || m_app_ctx.ctx->request_restart) {
        delay = _update_screen(display_state.display, SCREEN_MODE_SHUT_DOWN, 0);
        ctx->cur_screen = CUR_SCREEN_OFF_SCREEN;
        goto end;
    }

    else if(m_app_ctx.low_bat_countdown) {
        delay=_update_screen(display_state.display, SCREEN_MODE_LOW_BAT, 0);
        goto end;
    }
    else if(!m_app_ctx.ctx->sdOK) {
        delay=_update_screen(display_state.display, SCREEN_MODE_SD_TROUBLE, 0);
        goto end;
    }
    
    if (ctx->next_screen == CUR_SCREEN_SETTINGS) {
        struct m_config_item_s item = {0};
        if(ctx->cfg_screen == CFG_GROUP_GPS) {
            get_gps_cfg_item(ctx->gps_cfg_item, &item);
            const v_settings_t s = { CFG_GROUP_GPS, "GPS", &item };
            delay = _update_screen(display_state.display, SCREEN_MODE_SETTINGS, (void*)&s);
        } else if(ctx->cfg_screen == CFG_GROUP_STAT_SCREENS) {
            get_stat_screen_cfg_item(ctx->config, ctx->stat_screen_cfg_item, &item);
            const v_settings_t s = { CFG_GROUP_STAT_SCREENS, "Stat Screens", &item };
            delay = _update_screen(display_state.display, SCREEN_MODE_SETTINGS, (void*)&s);
        } else if(ctx->cfg_screen == CFG_GROUP_SCREEN) {
            get_screen_cfg_item(ctx->config, ctx->screen_cfg_item, &item);
            const v_settings_t s = { CFG_GROUP_SCREEN, "Display", &item };
            delay = _update_screen(display_state.display, SCREEN_MODE_SETTINGS, (void*)&s);
        } else if(ctx->cfg_screen == CFG_GROUP_FW) {
            get_fw_update_cfg_item(ctx->config, ctx->fw_cfg_item, &item);
            const v_settings_t s = { CFG_GROUP_FW, "FW Update", &item };
            delay = _update_screen(display_state.display, SCREEN_MODE_SETTINGS, (void*)&s);
        }
        ctx->cur_screen = CUR_SCREEN_SETTINGS;
    }
#if defined(CONFIG_LOGGER_WIFI_ENABLED)
    else if (ctx->app_mode == APP_MODE_WIFI) {
        int wifistatus = wifi_status();
        if(ctx->next_screen==CUR_SCREEN_SAVE_SESSION){
            delay=_update_screen(display_state.display, SCREEN_MODE_SHUT_DOWN, 0);
            ctx->cur_screen = CUR_SCREEN_SAVE_SESSION;
        } else {
            if (wifistatus < 1) {
                delay=_update_screen(display_state.display, SCREEN_MODE_WIFI_START, 0);
            } else if (wifistatus == 1) {
                delay=_update_screen(display_state.display, SCREEN_MODE_WIFI_STATION, 0);
            } else {
                delay=_update_screen(display_state.display, SCREEN_MODE_WIFI_AP, 0);
            }
            ctx->cur_screen = CUR_SCREEN_WIFI;
        }
    } 
#endif
    else if (ctx->app_mode == APP_MODE_GPS) {
#if (defined(CONFIG_UBLOX_ENABLED) && defined(CONFIG_GPS_LOG_ENABLED))
        if (ubx_dev && gps && gps->time_set && (ubx_dev->ubx_msg.navPvt.iTOW - gps->old_nav_pvt_itow) > (gps->time_out_gps_msg * 5) && ctx->next_screen == CUR_SCREEN_NONE) {
            gpstrblscr:
            delay=_update_screen(display_state.display, SCREEN_MODE_GPS_TROUBLE, 0);  // gps signal lost !!!
            ctx->cur_screen = CUR_SCREEN_GPS_TROUBLE;
        } else if ((ctx->next_screen != CUR_SCREEN_GPS_STATS && (!ubx_dev || !ubx_dev->ready || (ubx_dev->ready && !ubx_dev->ubx_msg.mon_ver.hwVersion[0]))) || (!run_is_active && ctx->next_screen == CUR_SCREEN_GPS_INFO)) {
            // if(!ubx_dev->ubx_msg.mon_ver.hwVersion[0]) goto bootscreen;
            delay=_update_screen(display_state.display, SCREEN_MODE_GPS_INIT, 0);
            ctx->cur_screen = CUR_SCREEN_GPS_INFO;
        }
        else if (!run_is_active && (gps->S2.display_max_speed  > 1000 || ctx->next_screen == CUR_SCREEN_GPS_STATS)) {
            if (gps->record && ctx->record_done == 255) {
                if(gps->S2.display_max_speed > 10000) // when more than 32k/h show records
                    ctx->record_done=0;
                gps->record = 0;
            }
            if (gps->S10.record && ctx->record_done < 2) { // 10sec max record
                struct record_forwarder_s r = { &avail_fields[16], &avail_fields[17], ctx->record_done==0};
                delay=_update_screen(display_state.display, SCREEN_MODE_RECORD, &r);
                ++ctx->record_done;
                goto end;
            }
            else if (gps->S2.record && ctx->record_done < 4) { // 2sec max record
                if(ctx->record_done<2) ctx->record_done = 2;
                struct record_forwarder_s r = { &avail_fields[47], &avail_fields[48] , ctx->record_done==2};
                delay=_update_screen(display_state.display, SCREEN_MODE_RECORD, &r);
                ++ctx->record_done;
                goto end;
            }
            else if (gps->M250.record && ctx->record_done < 6) { // 500m max record
                if(ctx->record_done<4) ctx->record_done = 4;
                struct record_forwarder_s r = { &avail_fields[51], &avail_fields[52] , ctx->record_done==4};
                delay=_update_screen(display_state.display, SCREEN_MODE_RECORD, &r);
                ++ctx->record_done;
                goto end;
            }
            else if (gps->M500.record && ctx->record_done < 8) { // 500m max record
                if(ctx->record_done<6) ctx->record_done = 6;
                struct record_forwarder_s r = { &avail_fields[53], &avail_fields[54] , ctx->record_done==6};
                delay=_update_screen(display_state.display, SCREEN_MODE_RECORD, &r);
                ++ctx->record_done;
                goto end;
            }
            else if (gps->M1852.record && ctx->record_done < 10) { // 1852m max record
                if(ctx->record_done<8) ctx->record_done = 8;
                struct record_forwarder_s r = { &avail_fields[55], &avail_fields[56] , ctx->record_done==8};
                delay=_update_screen(display_state.display, SCREEN_MODE_RECORD, &r);
                ++ctx->record_done;
                goto end;
            }
             else if (gps->A500.record && ctx->record_done < 12) { // 500m alfa max record
                if(ctx->record_done<10) ctx->record_done = 10;
                struct record_forwarder_s r = { &avail_fields[42], &avail_fields[43] , ctx->record_done==10};
                delay=_update_screen(display_state.display, SCREEN_MODE_RECORD, &r);
                ++ctx->record_done;
                goto end;
            }
            else if(ctx->record_done < 240) {
                ctx->record_done = 240;
            }
            // lower than 5 km/h
            while(m_app_ctx.ctx->stat_screen[m_app_ctx.ctx->stat_screen_cur] == 0) {
                if(++m_app_ctx.ctx->stat_screen_cur >= get_stat_screens_count()) { // 16 fields used eq uint16_t
                    m_app_ctx.ctx->stat_screen_cur = 0;
                    break;
                }
            }
            delay=_update_screen(display_state.display, m_app_ctx.ctx->stat_screen_cur+1, 0);
            ctx->cur_screen = CUR_SCREEN_GPS_STATS;
        } else {
            if(ubx_dev && !ubx_dev->ubx_msg.mon_ver.hwVersion[0] && ubx_dev->ready) goto gpstrblscr;
            if (ctx->config && ctx->config->screen.speed_large_font == 2) {
                delay=_update_screen(display_state.display, SCREEN_MODE_SPEED_2, 0);
            } else {
                delay=_update_screen(display_state.display, SCREEN_MODE_SPEED_1, 0);
            }
            m_app_ctx.ctx->stat_screen_cur = 0;
            ctx->stat_screen_count = m_app_ctx.ctx->stat_screen_count;
            ctx->cur_screen = CUR_SCREEN_GPS_SPEED;
            ctx->record_done = 255;
            ctx->gps_cfg_item = CFG_GPS_ITEM_BASE;
        }
#else
        delay=_update_screen(display_state.display, SCREEN_MODE_GPS_INIT, 0);
        cur_screen = CUR_SCREEN_GPS_INFO;
#endif
    }
    end:
    if(!m_app_ctx.screen_auto_refresh && display_get_flush_count() == 2) {
        delay_ms(300);
    }

    update_lv_timers();
    DMEAS_END(TAG, "[%s] took: %llu us",  __FUNCTION__);
    return delay;
}

display_op_t screen_ops = {
    .screen_cb = screen_cb,
};

struct display_s *lcd_init() {
    ILOG(TAG, "[%s]", __func__);
    display_init(display_state.display, &screen_ops);
    return display_state.display;
}

void lcd_deinit() {
    ILOG(TAG, "[%s]", __func__);
    stop_lv_timers();
    display_uninit(display_state.display);
}

#endif
