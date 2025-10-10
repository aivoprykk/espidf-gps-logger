#include "gps_log.h"
#include "logger_common.h"
#include "private.h"

#include "lcd.h"
#include "vfs.h"

#if defined(CONFIG_DISPLAY_ENABLED)

#include <string.h>

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
    int16_t gps_image_angle;
#if defined(CONFIG_LCD_IS_EPD)
    int8_t offset_mark;
    uint8_t dots_counter;
    uint8_t offscreen_counter;
    uint16_t screen_mode_counter;
    uint8_t offset_mark_dir;
#endif
    uint32_t start_logging_millis;
    int16_t low_speed_seconds;
    uint8_t speed_timer_status;
};

static struct display_state_s display_state = { &m_app_ctx.display, 0, {0, 0, 0, 0, 0, 0, 0},
#if defined(CONFIG_BMX_ENABLE) 
0, 
#endif
"", "", 0, 0, SCREEN_MODE_UNKNOWN, SCREEN_MODE_UNKNOWN, 0,
#if defined(CONFIG_LCD_IS_EPD)
    0, 0, 0, 0, 0,
#endif
    0, 0, 0
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
        {&m_context_rtc.RTC_max_2s, "2s:"},
        {&m_context_rtc.RTC_1h, "1h:"},
        {&m_context_rtc.RTC_500m, "500:"},
        {&m_context_rtc.RTC_mile, "NM:"},
        {&m_context_rtc.RTC_distance, "Dst:"},
        {&m_context_rtc.RTC_alp, "Alfa:"}
    }
};

static const char *scr_fld[2][9][2] = {
    {
        {"Run", "Max"},
        {"Gate", "Ex"},
        {"Al", "Al"},
        {"Nm", "Nm"},
        {"500", "500"},
        {"Dst", "Tm"},
        {"2s", "10s"},
        {".5h", ".5h"},
        {"1h", "1h"},
    },
#if defined(CONFIG_LOGGER_SPEED_SCREEN_VARIANT)
    {
        {"R", "A"},
        {"G", "E"},
        {"Al", 0},
        {"Nm", ""},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0},
    }
#endif
};

// #define TEST_NUM 1

#if defined(TEST_NUM)
#define SCR_FLD_2_1_LEN 5
static const char * scr_fld_2[] = {
    "109.8",
    "0.00"
};
static size_t fake_f2_to_char(double f, char * p) {
    memcpy(p, scr_fld_2[0], SCR_FLD_2_1_LEN), p += 5, * p = 0;
    return 5;
    
}
#else
#define SCR_FLD_2_1_LEN 4
static const char * scr_fld_2[] = {
    "-.--",
    "0.00"
};
#endif


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
    FUNC_ENTRY(TAG);
    char tmp[24], *p = tmp;
    lv_label_t *panel;
    // if (_lvgl_lock(50)) {
    display_state.current_screen_mode = SCREEN_MODE_SLEEP;
#if defined(CONFIG_LCD_IS_EPD)
    // if(!m_app_ctx.screen_auto_refresh && display_get_flush_count() == 0) {
    //     showSleepScreen();
    // }
    // else {
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
#if defined(TEST_NUM)
                fake_f2_to_char(*sleep_scr_info_fields[j][i].data, p);
#else
                f2_to_char(*sleep_scr_info_fields[j][i].data, p);
#endif
                lv_label_set_text(ui_sleep_screen.cells[i][j].title, p);
                lv_label_set_text(ui_sleep_screen.cells[i][j].info, sleep_scr_info_fields[j][i].info);
            }
        }
        lv_label_set_text(ui_sleep_screen.myid, m_context_rtc.RTC_Sleep_txt);
#if defined(CONFIG_LCD_IS_EPD)
    // }
    }
#endif
    // lcd_ui_request_full_refresh(0); // first screen load will cause full refresh
    // display_state.current_screen_mode = display_state.old_screen_mode = SCREEN_MODE_SLEEP;
    // display_incr_buf_update_count();
    return 100;
}

static const char * bat_to_char(char *str, uint8_t mode) {
    float bat;
#if defined(CONFIG_LOGGER_ADC_ENABLED)
    get_battery_voltage_for_display(0, m_context_rtc.RTC_voltage_bat, &bat);
#else
    bat = m_context_rtc.RTC_voltage_bat;
#endif
    const uint8_t state_perc = bat > 4.5 ? 110
                : bat >= 4.2 ? 101
#if defined(CONFIG_LOGGER_ADC_ENABLED)
                : calc_bat_perc_v(bat);
#else
                : 51;
#endif
    const char * s = adc_on_ac() ? LV_SYMBOL_CHARGE
            : state_perc < 10  ? LV_SYMBOL_BATTERY_EMPTY
            : state_perc < 20  ? LV_SYMBOL_BATTERY_1
            : state_perc < 60  ? LV_SYMBOL_BATTERY_2
            : state_perc < 101 ? LV_SYMBOL_BATTERY_FULL
            : state_perc < 110 ? LV_SYMBOL_CHARGE
                        : LV_SYMBOL_USB;
    char *p = str;
    if(mode == bat_perc) {
        if(state_perc < 100) *p++ = ' ';
        p = str + uint_to_char((state_perc > 100 ? 100 : state_perc), str);
        *p++ = '%';
    } else if(mode == bat_volt) {
        p += f2_to_char(bat, p);
        *p++ = 'V';
    }
    *p = 0;
    return s;
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

#define CFG_ENUM_F(l) cfg_fld_##l,
enum cfg_speed_fld_e {
  SPEED_FIELD_ITEM_LIST(CFG_ENUM_F)
};

static inline bool is_dist_reached(float run_distance_after_turn, float bar_length) {
    return (run_distance_after_turn > bar_length);
}

static inline bool alfa_in_window(float alfa_window) {
    return (alfa_window > 0 && alfa_window < 100);
}

static inline bool show_alfa_after_turn(float run_distance_after_turn, float alfa_window) {
    return (!gps->skip_alfa_after_stop && run_distance_after_turn < 350.0f && alfa_in_window(alfa_window));
}

static esp_err_t speed_info_bar_update() {  // info bar when config->screen.speed_large_font is 1 or 0
    const logger_config_t *config = m_app_ctx.config;
    if(!config) return ESP_ERR_INVALID_STATE;
    uint8_t field = config->screen.speed_field;          // default is in config.txt
    const uint8_t bar_max = BAR_MAX;                                  // 240 pixels is volledige bar
    uint16_t bar_length = M_TO_MM(config->bar_length) / bar_max;  // default 100% length = 1852 m
#if defined(CONFIG_LOGGER_SPEED_SCREEN_VARIANT)
    const uint8_t font_size = config->screen.speed_large_font;
#endif
    float s[] = {0, 0};
    const char *var[] = {0, 0};
    char val[][24] = {{0}, {0}}, *p;
    bool has_moved = true;
#if defined(CONFIG_GPS_LOG_ENABLED)
    struct gps_context_s *gps = &m_app_ctx.ctx->gps;
    const struct gps_data_s * gps_data = &gps->Ublox;
    const struct ubx_config_s * ubx_dev = gps->ubx_device;

    if (!ubx_dev->ready || !gps->signal_ok) {
        memcpy(val[0], scr_fld_2[0], SCR_FLD_2_1_LEN);
        if (field != cfg_fld_spd_dist_time) {
            memcpy(val[1], scr_fld_2[0], SCR_FLD_2_1_LEN);
        }
        has_moved = false;
    }
    else if(time_cur_speed(time_2s) < SEC_TO_MS(1)) { /// 1ms = 3.6km/h
        memcpy(val[0], scr_fld_2[1], 4);
        if (field != cfg_fld_spd_dist_time) {
            memcpy(val[1], scr_fld_2[1], 4);
        }
        has_moved = false;
    }
    float run_distance_after_turn = MM_TO_M(gps_data->run_distance_after_turn);
    // float run_distance = get_distance_m(gps->A500.base->distance, ubx_dev->rtc_conf->output_rate);
    if (field == cfg_fld_dynamic) {
        if(!has_moved)
            field = cfg_fld_spd_10_sec;
        else if ((((int)MM_TO_KM(gps_data->total_distance) % 10 == 0) && (run_distance_after_turn > ONE_K)))
            field = cfg_fld_spd_alpha;
        else if (show_alfa_after_turn(run_distance_after_turn, gps->alfa_window))
            field = cfg_fld_spd_alpha;
        else if (is_dist_reached(run_distance_after_turn, config->bar_length))
            field =  cfg_fld_spd_1852_m; 
        else if (is_dist_reached(run_distance_after_turn, 500.0f))
            field =  cfg_fld_spd_500_m;
        else
            field = cfg_fld_spd_10_sec;
    } 
    else if (field == cfg_fld_spd_10_sec) {
        if(!has_moved)
            field = cfg_fld_spd_10_sec;
        else if (is_dist_reached(run_distance_after_turn, config->bar_length))
            field =  cfg_fld_spd_1852_m;
        else if (is_dist_reached(run_distance_after_turn, 500.0f))
            field =  cfg_fld_spd_500_m;
        else
            field = cfg_fld_spd_10_sec;
    }
    else if (field == cfg_fld_spd_half_hour) { // show alpha status
        if (has_moved && show_alfa_after_turn(run_distance_after_turn, gps->alfa_window))
            field =  cfg_fld_spd_alpha;
        else
            field = cfg_fld_spd_half_hour;
    }
    else if (field == cfg_fld_spd_1_hour) { // show 1 hour status
        if (has_moved && show_alfa_after_turn(run_distance_after_turn, gps->alfa_window))
            field =  cfg_fld_spd_alpha;
        else
            field = cfg_fld_spd_1_hour;
    }
    else if (field == cfg_fld_spd_1h_dynamic) {  // 1 hour default, but first alfa, and if good run, last run
        if(!has_moved)
            field = cfg_fld_spd_1_hour;
        else if (is_dist_reached(run_distance_after_turn, config->bar_length))
            field = cfg_fld_spd_1852_m;
        else if (is_dist_reached(run_distance_after_turn, 500.0f))
            field = cfg_fld_spd_500_m;
        else if (time_max_speed(time_10s) > time_display_speed(time_10s, 5))
            field = cfg_fld_spd_10_sec;  // if run faster then slowest run, show AVG & run
        else if (show_alfa_after_turn(run_distance_after_turn, gps->alfa_window))
            field = cfg_fld_spd_alpha;
        else
            field = cfg_fld_spd_1_hour;
  }
#endif
    if (field <= cfg_fld_spd_10_sec) { /// 10 seconds stats
        if(has_moved) {
            s[0] = avail_fields[fld_s10_cur_run_max].value.num(); // s10 current run max speed
            s[1] = avail_fields[fld_s10_display_max].value.num();  // s10 avg speed
            if (NUM_LT_3_DIG(s[0])) 
                f2_to_char(s[0], val[0]);
            else
                f1_to_char(s[0], val[0]);
            if (NUM_LT_3_DIG(s[1]))
                f2_to_char(s[1], val[1]);
            else
                f1_to_char(s[1], val[1]);
        }
#if defined(CONFIG_LOGGER_SPEED_SCREEN_VARIANT)
        if (font_size == 0) {
#endif
            var[0] = scr_fld[0][field-1][0];
            var[1] = scr_fld[0][field-1][1];
#if defined(CONFIG_LOGGER_SPEED_SCREEN_VARIANT)
        } else {
            var[0] = scr_fld[1][field-1][0];
            var[1] = scr_fld[1][field-1][1];
        }
#endif
    }

    // First 250m after jibe, if Window>99 m : Window and Exit
    // Between 250m and 400m after jibe : Result Alfa (speed or MISS)
    // Between 400m and 1852m after jibe : Actual Run + AVG
    // More then 1852m : NM actual speed and NM Best speed

    else if (field == cfg_fld_spd_alpha) {
        bar_length = M_TO_MM(250) / bar_max;  // full bar length with Alfa = 250 meter
#if defined(CONFIG_GPS_LOG_ENABLED)
        if (has_moved && !is_dist_reached(run_distance_after_turn, 250.0f) && alfa_in_window(gps->alfa_window)) {
            if (gps->alfa_exit > 99) gps->alfa_exit = 99;
            f_to_char(gps->alfa_window, val[0], 0);
            f_to_char(gps->alfa_exit, val[1], 0);
            var[1] =  scr_fld[1][field-1][1];
            var[0] =  scr_fld[0][field-1][0];
        } else 
#endif
        { // alfa speed stats
            if(has_moved) {
                s[0] = avail_fields[fld_a500_cur_run_max].value.num(); // a500 current run max speed
                s[1] = avail_fields[fld_a500_display_max].value.num(); // a500 max speed
#if defined(CONFIG_LOGGER_SPEED_SCREEN_VARIANT)
                if (font_size == 0) {
#endif
                    if (NUM_LT_3_DIG(s[0]))
                        f2_to_char(s[0], val[0]);
                    else
                        f1_to_char(s[0], val[0]);
#if defined(CONFIG_LOGGER_SPEED_SCREEN_VARIANT)
                }
                else
                    val[0][0] = 0;
#endif
                if (s[1] > 1) {
                    if (NUM_LT_3_DIG(s[1]))
                        f2_to_char(s[1], val[1]);
                    else
                        f1_to_char(s[1], val[1]);
                } else {
                    memcpy(val[1], scr_fld_2[1], 4);
                    val[1][4]=0;
                }
            }
            goto setvars;
        }
    }
    else if (field == cfg_fld_spd_1852_m) { // nautical mile
        if(has_moved) {
            s[0] = avail_fields[fld_m1852_cur_run_max].value.num(); // m1852 current run max speed
            s[1] = avail_fields[fld_m1852_display_max].value.num(); // m1852 max speed
            if (NUM_LT_3_DIG(s[0])) {
                f2_to_char(s[0], val[0]);
            } else {
                f1_to_char(s[0], val[0]);
            }
#if defined(CONFIG_LOGGER_SPEED_SCREEN_VARIANT)    
            if (font_size == 0){
#endif
                    if (NUM_LT_3_DIG(s[1]))
                        f2_to_char(s[1], val[1]);
                    else
                        f1_to_char(s[1], val[1]);
#if defined(CONFIG_LOGGER_SPEED_SCREEN_VARIANT)
            }
            else
                *val[1] = 0;
#endif
        }
        setvars:
#if defined(CONFIG_LOGGER_SPEED_SCREEN_VARIANT)
        if (font_size == 0) {
#endif
            var[0] = scr_fld[0][field][0];  // Actuele nautical mile
            var[1] = scr_fld[0][field][1];
#if defined(CONFIG_LOGGER_SPEED_SCREEN_VARIANT)
        } else {
            var[0] = scr_fld[1][field][0];  // Actuele nautical mile
            var[1] = scr_fld[1][field][1];
        }
#endif
    }
    else if (field == cfg_fld_spd_dist_time) { // total distance
        if(has_moved) {
            s[0] = avail_fields[fld_distance].value.num(); // total dist km
            if (NUM_LT_3_DIG(s[0])) 
                f2_to_char(s[0], val[0]);
            else
                f1_to_char(s[0], val[0]);
        }
        avail_fields[fld_total_time_hm].value.timestr(val[1]);
        goto setvar;
    }
    else if (field == cfg_fld_spd_500_m) { // total distance
        if(has_moved) {
            s[0] = avail_fields[fld_m500_cur_run_max].value.num(); // m500 current run max speed
            s[1] = avail_fields[fld_m500_display_max].value.num(); // m500 max speed
            if (NUM_LT_3_DIG(s[0])) 
                f2_to_char(s[0], val[0]);
            else
                f1_to_char(s[0], val[0]);
            if (NUM_LT_3_DIG(s[1]))
                f2_to_char(s[1], val[1]);
            else
                f1_to_char(s[1], val[1]);
        }
        goto setvar;
    }
    else if (field == cfg_fld_spd_max_2s_10s) { // 2 and 10 seconds stats
        if(has_moved) {
            s[0] = avail_fields[fld_s2_display_max].value.num(); // s2 max speed
            s[1] = avail_fields[fld_s10_display_max].value.num(); // s10 max speed
            if (NUM_LT_3_DIG(s[0])) 
                f2_to_char(s[0], val[0]);
            else
                f1_to_char(s[0], val[0]);
            if (NUM_LT_3_DIG(s[1]))
                f2_to_char(s[1], val[1]);
            else
                f1_to_char(s[1], val[1]);
        }
        goto setvar;
    }
    else if (field == cfg_fld_spd_half_hour) { // 30 minutes stats
        if(has_moved) {
            s[0] = avail_fields[fld_s1800_cur_run_max].value.num();
            s[1] = avail_fields[fld_s1800_display_max].value.num(); // s1800 max speed
            if (NUM_LT_3_DIG(s[0])) 
                f2_to_char(s[0], val[0]);
            else
                f1_to_char(s[0], val[0]);
            if (NUM_LT_3_DIG(s[1]))
                f2_to_char(s[1], val[1]);
            else
                f1_to_char(s[1], val[1]);
        }
        goto setvar;
    }
    else if (field == cfg_fld_spd_1_hour) { // 60 minutes stats
        if(has_moved) {
            s[0] = avail_fields[fld_s3600_cur_run_max].value.num();
            s[1] = avail_fields[fld_s3600_display_max].value.num(); // 1h max speed
            if (NUM_LT_3_DIG(s[0])) 
                f2_to_char(s[0], val[0]);
            else
                f1_to_char(s[0], val[0]);
            if (NUM_LT_3_DIG(s[1]))
                f2_to_char(s[1], val[1]);
            else
                f1_to_char(s[1], val[1]);
        }
        setvar:
        var[0] = scr_fld[0][field][0];
        var[1] = scr_fld[0][field][1];
    }

    // col 1
    lv_obj_t *panel;
    if ((panel = ui_speed_screen.cells[0][0].info)) {
        set_label_text_safe(panel, var[0], 0);
    }
    if ((panel = ui_speed_screen.cells[0][0].title)) {
        set_label_text_safe(panel, val[0], 0);
    }
    // col 2
    if ((panel = ui_speed_screen.cells[0][1].info)) {
        set_label_text_safe(panel, var[1], 0);
#if defined(CONFIG_LCD_IS_EPD)
        if (field == cfg_fld_spd_500_m || field == cfg_fld_spd_half_hour) {
            lv_obj_set_x(lv_obj_get_parent(panel), lv_pct(-2));
        }
#endif
    }
    if ((panel = ui_speed_screen.cells[0][1].title)) {
        set_label_text_safe(panel, val[1], 0);
        // if (field == cfg_fld_spd_dist_time) {
        //     lv_obj_set_width(lv_obj_get_parent(panel), lv_pct(87));
        //     lv_obj_set_style_text_font(panel, ui_speed_screen.font.title_tms, LV_PART_MAIN | LV_STATE_DEFAULT);
        // }
        // else {
        // }
    }
    if (has_moved && (panel = ui_speed_screen.bar)) {
        uint32_t run_rectangle_length = 0;
#if defined(CONFIG_GPS_LOG_ENABLED)
        uint32_t millis = get_millis();
        uint32_t log_seconds = display_state.start_logging_millis ? MS_TO_SEC(millis - display_state.start_logging_millis) : 0;  // number of seconds since logging started
        if (field == cfg_fld_spd_half_hour) {
            run_rectangle_length = log_seconds * bar_max / (HALF_H_IN_SECS);
            if (log_seconds > (HALF_H_IN_SECS)) {
                display_state.start_logging_millis = millis;
            }
        }  // 30 minutes = full bar
        else if (field == cfg_fld_spd_1_hour) {
            run_rectangle_length = log_seconds * bar_max / ONE_H_IN_SECS;
            if (log_seconds > ONE_H_IN_SECS) {
                display_state.start_logging_millis = millis;
            }
        }  // 60 minutes = full bar
        else if (field == cfg_fld_spd_alpha && gps_data->run_distance_after_turn <= bar_length) {
            run_rectangle_length = gps_data->run_distance_after_turn / bar_length;
        }
        else {
            run_rectangle_length = gps_data->run_distance / bar_length;
        }
#endif
        lv_bar_set_value(panel, run_rectangle_length, 0);
    }
    return ESP_OK;
}

/**
 * @brief Updates the low speed seconds counter.
 *        This function checks the average speed over 10 seconds and
 *        updates the low speed seconds counter accordingly.
 */
static void speed_info_bar_update_low_speed_seconds(void) {
    struct gps_context_s *gps = &m_app_ctx.ctx->gps;
    if (time_cur_speed(time_10s) > SEC_TO_MS(2)) {
        // if the speed is higher then 2 m/s, reset the counter
        display_state.low_speed_seconds = 0;
        if(!display_state.start_logging_millis)
            goto start_logging;
    }
    else {
        if (display_state.low_speed_seconds > BAR_RESET_IN_STILL_SECS) { 
            // bar will be reset if the 10s speed drops under 2m/s for more then 120 s !!!!
        start_logging:
            display_state.start_logging_millis = get_millis();
        }
        else
            ++display_state.low_speed_seconds;
    }
}

/**
 * @brief Updates the speed information on the screen.
 *        This function retrieves the current speed from the GPS context,
 *        formats it, and updates the corresponding label in the UI.
 *
 * @param timer Pointer to the timer triggering this callback.
 */
static void speed_cb(lv_timer_t *timer) {
    FUNC_ENTRY(TAG);
    const struct gps_context_s *gps = &m_app_ctx.ctx->gps;
    const struct ubx_config_s *ubx_dev = gps->ubx_device;
    char str[8] = {0}, *p = str;
    size_t len = 4;
    float gpsspd;
    if (ui_speed_screen.speed) {
#if defined(CONFIG_GPS_LOG_ENABLED)
#if defined(CONFIG_UBLOX_ENABLED)
      if (!ubx_dev || !ubx_dev->ready || !gps->signal_ok) {
          memcpy(p, scr_fld_2[0], SCR_FLD_2_1_LEN);
          *(p+SCR_FLD_2_1_LEN) = 0;
      }
#endif
      else if(time_cur_speed(time_2s) < ONE_M_S_IN_MM_S) {
          memcpy(p, scr_fld_2[1], len);
          *(p+len) = 0;
      }
      else 
      {
          gpsspd = convert_speed((float)gps_last_speed_smoothed(2), c_gps_cfg.speed_unit);
          len = (NUM_GE_3_DIG(gpsspd)) ? f1_to_char(gpsspd, p) : f2_to_char(gpsspd, p);
      }
#else
      memcpy(p, "_.__", len);
      *(p+len) = 0;
#endif
        set_label_text_safe(ui_speed_screen.speed, p, 0);
    }
}

/**
 * @brief Updates the GPS information on the screen.
 *        This function retrieves the current GPS status, formats it,
 *        and updates the corresponding labels in the UI.
 *
 * @param timer Pointer to the timer triggering this callback.
 */
static void gps_info_cb(lv_timer_t *timer) {
    FUNC_ENTRY(TAG);
    char str[64] = {0}, *p = str;
    const struct gps_context_s *gps = &m_app_ctx.ctx->gps;
    const struct ubx_config_s *ubx_dev = gps->ubx_device;
    const char * gpsstr = ubx_dev ? ubx_chip_str(ubx_dev) : 0;
    const lv_img_dsc_t * gps_img = &near_me_bold_48px;
    if(!gpsstr) {
        p=str;
        memcpy(p, "NO GPS YET", 10), p += 10;
    }
    else if(!strcmp(gpsstr, "UNKNOWN")) {
        p=str;
        memcpy(p, "GPS", 3), p += 3;
        if(ubx_dev->ready && rtc_config.hw_type == UBX_HW_TYPE_DEFAULT) {
            memcpy(p, " -", 2), p += 2;
            gps_img = &near_me_disabled_bold_48px;
        }
    }
    else {
        p += strlen(gpsstr);
        memcpy(&str[0], gpsstr, p-&str[0]);
        *p++ = '@';
        p += xltoa(rtc_config.output_rate, p);
        memcpy(p, "Hz", 2), p += 2;
    }
    *p = 0;
    
    set_screen_img(ui_info_screen.info_img, gps_img, 0);
    set_label_text_safe(ui_info_screen.info_rows[UI_INFO_SCREEN_TITLE_LBL], &str[0], 0);
    update_gps_info_row_str(ubx_dev, &str[0]);
    set_label_text_safe(ui_info_screen.info_rows[UI_INFO_SCREEN_ROW_2_LBL], &str[0], 1);
    update_gps_desc_row_str(gps, &str[0]);
    set_label_text_safe(ui_info_screen.info_rows[UI_INFO_SCREEN_ROW_3_LBL], &str[0], 1);
    vfs_space_str(&str[0], sizeof(str));
    set_label_text_safe(ui_info_screen.info_rows[UI_INFO_SCREEN_ROW_4_LBL], &str[0], 1);
}

/**
 * @brief Updates the Wi-Fi information on the screen.
 *        This function retrieves the current Wi-Fi connection status,
 *        formats it, and updates the corresponding labels in the UI.
 *
 * @param timer Pointer to the timer triggering this callback.
 */
static void wifi_info_cb(lv_timer_t *timer) {
    FUNC_ENTRY(TAG);
#if defined(CONFIG_LOGGER_WIFI_ENABLED)
    char str[64] = {0}, *p = str;
    size_t len = 0;
    struct m_wifi_context *wctx = m_app_ctx.wifi_ctx;
    uint8_t cur_label = 0;
#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ENABLE_WIFI_AP_STA)
    bool is_ap_or_apsta  = (wctx->s_wifi_mode == wifi_mode_apsta || wctx->s_wifi_mode == wifi_mode_ap);
    bool is_sta_or_apsta = (wctx->s_wifi_mode == wifi_mode_apsta || wctx->s_wifi_mode == wifi_mode_sta);
#else
    bool is_ap_or_apsta = (wctx->s_wifi_mode == wifi_mode_ap);
    bool is_sta_or_apsta = (wctx->s_wifi_mode == wifi_mode_sta);
#endif
    /// first row
    p = str;
    if(is_ap_or_apsta) {
        memcpy(p, "AP: ", 4), p+= 4;
        len = strlen(wctx->ap.ssid);
        memcpy(p, wctx->ap.ssid, len), p += len;
        if(!wctx->s_ap_connection) {
            memcpy(p, " ...", 4), p+= 4;
        }
    }
#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ENABLE_WIFI_AP_STA)
    if(wctx->s_wifi_mode == wifi_mode_apsta) {
        *p = 0;
        set_label_text_safe(ui_info_screen.info_rows[cur_label++], &str[0], 0);
        p = str;
    }
#endif
    if(is_sta_or_apsta) {
        memcpy(p, "STA: ", 5), p+= 5;
        if(!wctx->s_sta_connection) {
            memcpy(p, "scan ...", 8), p+= 8;
        }
        else if (wctx->s_sta_num_connect < M_WIFI_STA_MAX) {
            len = strlen(wctx->stas[wctx->s_sta_num_connect].ssid);
            memcpy(p, wctx->stas[wctx->s_sta_num_connect].ssid, len), p += len;
        } else {
            memcpy(p, "not found", 9), p+= 9;
        }
    }
    *p = 0;
    set_label_text_safe(ui_info_screen.info_rows[cur_label++], &str[0], 0);
    /// second row
    p = str;
    memcpy(p, "addr: ", 6), p += 6;
    if((is_sta_or_apsta) && wctx->s_sta_connection) {
        if(!wctx->s_sta_connection) {
            memcpy(p, "...", 3), p+= 3;
        }
        else if( wctx->s_sta_num_connect < M_WIFI_STA_MAX) {
            len = strlen(wctx->hostname);
            memcpy(p, wctx->hostname, len), p += len;
            if(len>1) {
                memcpy(p, ".local", 6), p += 6;
            }
        }
        else {
            memcpy(p, "failed", 6), p += 6;
        }
    }
    else if((is_ap_or_apsta) && wctx->s_ap_connection) {
        len = sprintf(p, "%hhu.%hhu.%hhu.%hhu",
            wctx->ap.ipv4_address[0],
            wctx->ap.ipv4_address[1],
            wctx->ap.ipv4_address[2],
            wctx->ap.ipv4_address[3]);
        p += len;
    }
    else {
        memcpy(p, "...", 3), p += 3;
    }
    *p = 0;
    set_label_text_safe(ui_info_screen.info_rows[cur_label++], &str[0], 1);
    /// third row
    p = str;
    if((is_ap_or_apsta) && wctx->s_ap_connection) {
        memcpy(p, "pwd: password", 13), p += 13;
    }
    *p = 0;
    set_label_text_safe(ui_info_screen.info_rows[cur_label++], &str[0], 1);
    if(cur_label<=3) {
        p = str;
        memcpy(p, " ", 1), p += 1;
        *p=0;
        set_label_text_safe(ui_info_screen.info_rows[UI_INFO_SCREEN_ROW_4_LBL], &str[0], 1);
    }
#endif
}

/**
 * @brief Updates the status bar with the current time and date.
 *        This function retrieves the current time from the RTC context,
 *        formats it, and updates the corresponding label in the status bar.
 *
 * @param timer Pointer to the timer triggering this callback.
 */
static void statusbar_time_cb(lv_timer_t *timer) {
    FUNC_ENTRY(TAG);
#if defined(STATUS_PANEL_V1)
    ui_status_panel_t * statusbar = &ui_status_panel;
    if(!statusbar->parent) {
        return;
    }
#else
    lv_statusbar_t * statusbar = (lv_statusbar_t *)ui_StatusPanel;
#endif
    const struct main_ctx_s *ctx = 0;
    if (timer) {
#if (LVGL_VERSION_MAJOR < 9)
        ctx = timer->user_data;
#else        
        ctx = (const struct main_ctx_s *)lv_timer_get_user_data(timer);
#endif
        if (ctx->app_mode == APP_MODE_WIFI && !m_app_ctx.ctx->firmware_update_started) {
            wifi_info_cb(timer);
        }
    }
    struct tm tm = {0};
    char tmp[24]={0}, *p = tmp;
    lv_obj_t *panel;
    if ((panel = statusbar->time_label)) {
        if (statusbar->viewmode == STATUS_VIEWMODE_DEFAULT || statusbar->viewmode == STATUS_VIEWMODE_SPEED) {
            get_local_time(&tm);
        } else if (statusbar->viewmode == STATUS_VIEWMODE_SLEEP) {
            tm.tm_year = m_context_rtc.rtc_tm.tm_year;
            tm.tm_mon = m_context_rtc.rtc_tm.tm_mon;
            tm.tm_mday = m_context_rtc.rtc_tm.tm_mday;
            tm.tm_hour = m_context_rtc.rtc_tm.tm_hour;
            tm.tm_min = m_context_rtc.rtc_tm.tm_min;
            tm.tm_sec = m_context_rtc.rtc_tm.tm_sec;
        }
        if (tm.tm_year > 120) {
            p += time_to_char_hm(tm.tm_hour, tm.tm_min, p);
            if (statusbar->viewmode == STATUS_VIEWMODE_SLEEP) {
                *p++ = ' ';
                p += date_to_char(tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900, 0, p), *p = 0;
            }
        }
#if (C_LOG_LEVEL < 2)
            DLOG(TAG, "** [%s] (date)time: %s {d: %d, m:%d, y:%d} ** ", __func__, tmp, tm.tm_mday, tm.tm_mon, tm.tm_year);
#endif
        set_label_text_safe(panel, &tmp[0], 0);
    }
}

#if defined(CONFIG_BMX_ENABLE)
/**
 * @brief Updates the temperature status bar with the latest temperature data.
 *        This function retrieves the temperature reading, formats it, and updates
 *        the corresponding label in the status bar.
 *
 * @param timer Pointer to the timer triggering this callback.
 */
static void statusbar_temp_cb(lv_timer_t *timer) {
#if (C_LOG_LEVEL < 2)
   ILOG(TAG, "[%s]", __func__);
#endif
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
    if ((panel = statusbar->sat_info_label)) {
#if defined(CONFIG_BMX_ENABLE)
        size_t len = temp_to_char(p);
#if (C_LOG_LEVEL > 1)
        if(!len || (len == 3 && *p == '-' && *p+2 == '-')) {
#else
        if (!len) {
#endif
            p = 0;
        }
        set_label_text_safe(panel, p, 1);
#endif
    }
}
#endif // CONFIG_BMX_ENABLE

/**
 * @brief Updates the battery status bar with the current battery voltage.
 *        This function retrieves the battery voltage, formats it, and updates
 *        the corresponding label in the status bar.
 *
 * @param timer Pointer to the timer triggering this callback.
 */
static void statusbar_bat_cb(lv_timer_t *timer) {
#if (C_LOG_LEVEL < 2)
    ILOG(TAG, "[%s]", __func__);
#endif
#if defined(STATUS_PANEL_V1)
    ui_status_panel_t * statusbar = &ui_status_panel;
    if(!statusbar->parent) {
        return;
    }
#else
    lv_statusbar_t * statusbar = (lv_statusbar_t *)ui_StatusPanel;
#endif
    char tmp[24];
    lv_obj_t *panel;
    const char * s = bat_to_char(&tmp[0], m_context_rtc.bat_view);
    if ((panel = statusbar->bat_label)) {
        set_label_text_safe(panel, &tmp[0], 0);
    }
    if ((panel = statusbar->bat_image)) {
        set_label_text_safe(panel,s, 0);
// #if !defined(CONFIG_LCD_IS_EPD)
//         lv_obj_set_style_text_color(panel, full>20 ? lv_color_hex(0xFFFFFF) : full>10 ? lv_color_hex(0xEECE44) : lv_color_hex(0xE32424), LV_PART_MAIN | LV_STATE_DEFAULT );
//         lv_obj_set_style_text_opa(panel, 255, LV_PART_MAIN| LV_STATE_DEFAULT);
// #endif
    }
}

/**
 * @brief Updates the GPS status bar with the current GPS status.
 *        This function retrieves the GPS status, formats it, and updates
 *        the corresponding label in the status bar.
 *
 * @param timer Pointer to the timer triggering this callback.
 */
static void statusbar_gps_cb(lv_timer_t *timer) {
    FUNC_ENTRY(TAG);
#if defined(STATUS_PANEL_V1)
    ui_status_panel_t * statusbar = &ui_status_panel;
    if(!statusbar->parent) {
        return;
    }
#else
    lv_statusbar_t * statusbar = (lv_statusbar_t *)ui_StatusPanel;
#endif
    const struct main_ctx_s *ctx = 0;
    if(timer){
#if (LVGL_VERSION_MAJOR < 9)
        ctx = timer->user_data;
#else
        ctx  = lv_timer_get_user_data(timer);
#endif
        speed_info_bar_update_low_speed_seconds();
        if(ctx->cur_screen == CUR_SCREEN_GPS_SPEED) {
            speed_info_bar_update();
        }
        else if(ctx->cur_screen == CUR_SCREEN_GPS_INFO) {
            gps_info_cb(timer);
        }
    }
    else
        ctx = &m_app_ctx;
    if(!ctx || ! ctx->ctx || !ctx->ctx->gps.ubx_device) return;
    const ubx_config_t *ubx_dev = ctx->ctx->gps.ubx_device;
    lv_obj_t *panel;
    if((panel = statusbar->recoding_image)) {
        if (ctx->app_mode == APP_MODE_GPS && ctx->ctx->gps.files_opened) {
            if (lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_clear_flag(panel, LV_OBJ_FLAG_HIDDEN);
            }
        } else {
            if (!lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_add_flag(panel, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
    char tmp[24], *p;
    if ((panel = statusbar->sat_info_label)) {
        if (ctx->app_mode == APP_MODE_GPS && ubx_dev->is_on) {
            p = tmp;
            *p = 0;
            if (lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_clear_flag(panel, LV_OBJ_FLAG_HIDDEN);
            }
            set_label_text_safe(panel, speed_units[c_gps_cfg.speed_unit], 1);
            // if(ubx_dev->ubx_msg.navDOP.hDOP > 0 && ubx_dev->ubx_msg.navDOP.hDOP < 1000){
            //     *(p++) = 'h';
            //     p += f2_to_char(ubx_dev->ubx_msg.navDOP.hDOP/100.0f, p);
            // }
            // if(ubx_dev->ubx_msg.navPvt.sAcc > 0 && ubx_dev->ubx_msg.navPvt.sAcc < 1000) {
            //     *(p++) = 'a';
            //     p += f1_to_char(MM_TO_M(ubx_dev->ubx_msg.navPvt.sAcc), p);
            // }
            // *p=0;
            // set_label_text_safe(panel, &tmp[0], 0);
        }
        else {
            if (!lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_add_flag(panel, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
    if ((panel = statusbar->gps_image)) {
        p=tmp;
        *p = 0;
        uint8_t numsat = 0;    
        if (ctx->app_mode == APP_MODE_GPS && ubx_dev->is_on) {
            if (lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_clear_flag(panel, LV_OBJ_FLAG_HIDDEN);
            }
            numsat = ubx_dev->ubx_msg.navPvt.numSV;
            if(numsat) {
                p += xltoa(numsat, p);
                int nav_mode = get_cur_nav_mode(ubx_dev->rtc_conf->nav_mode);
                *(p++) =    nav_mode == UBX_MODE_AUTOMOTIVE ? 'A' :
                            nav_mode == UBX_MODE_PORTABLE ? 'O' :
                            nav_mode == UBX_MODE_PEDESTRIAN ? 'P' : 
                            nav_mode == UBX_MODE_SEA ? 'S' : '-';
                *p = 0;
            }
        } else {
            if (!lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_add_flag(panel, LV_OBJ_FLAG_HIDDEN);
            }
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
        set_label_text_safe(panel, &tmp[0], 0);
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

static lv_timer_t * gps_timer = 0;
static lv_timer_t * time_timer = 0;
static lv_timer_t * bat_timer = 0;
static lv_timer_t * temp_timer = 0;
static lv_timer_t * gps_speed_timer = 0;

void update_lv_timers() {
    FUNC_ENTRY(TAG);
    if(m_app_ctx.app_mode == APP_MODE_SLEEP) {
        statusbar_time_cb(0);
        statusbar_bat_cb(0);
    }
    else {
        if(m_app_ctx.app_mode == APP_MODE_GPS) {
            if(m_app_ctx.cur_screen == CUR_SCREEN_GPS_SPEED) {
                if(!gps_speed_timer) {
                    gps_speed_timer = lv_timer_create(speed_cb, QUATER_SEC_IN_MS, 0);
                }
                else if(!display_state.speed_timer_status) {
                    lv_timer_resume(gps_speed_timer);
                }
                if(!display_state.speed_timer_status) {
                    display_state.speed_timer_status = 1;
                }
            }
            else {
                if(gps_speed_timer && display_state.speed_timer_status) {
                    lv_timer_pause(gps_speed_timer);
                    display_state.speed_timer_status = 0;
                }
            }
            if(!gps_timer) {
                gps_timer = lv_timer_create(statusbar_gps_cb, SEC_TO_MS(1), &m_app_ctx);
            }
        } else { 
            if(gps_timer) {
                lv_timer_del(gps_timer);
                gps_timer = 0;
                statusbar_gps_cb(0);
            }
            if(gps_speed_timer) {
                lv_timer_del(gps_speed_timer);
                gps_speed_timer = 0;    
            }
        }
        if(!time_timer) {
            statusbar_time_cb(0);
            time_timer = lv_timer_create(statusbar_time_cb, SEC_TO_MS(1), &m_app_ctx);
        }
        if(!bat_timer) {
            statusbar_bat_cb(0);
            bat_timer = lv_timer_create(statusbar_bat_cb, SEC_TO_MS(1), &m_app_ctx);
        }
#if defined(CONFIG_BMX_ENABLE)
        if(!temp_timer) temp_timer = lv_timer_create(statusbar_temp_cb, SEC_TO_MS(1), 0);
#endif
    }
}

void stop_lv_timers() {
    FUNC_ENTRY(TAG);
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
    if(gps_speed_timer) {
        lv_timer_del(gps_speed_timer);
        gps_speed_timer = 0;
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
#if (C_LOG_LEVEL < 1)
        DLOG(TAG, "sat[%hhu]: %hhu, %hhu, %hhu, %hhu, %hu, %lu %lu %lu", i, sat->gnssId, sat->svId, sat->cno, sat->elev, sat->azim, sat->flags, (sat->flags & 0x08), (sat->flags & 0x07));
#endif
        if((sat->flags & 0x08u) == 0 || (sat->flags & 0x07u) < 4) {
            continue;
        }
        switch(sat->gnssId) {
            case UBX_GNSS_GPS:
                ++display_state.sat_count.gps;
                break;
            case UBX_GNSS_SBAS:
                ++display_state.sat_count.sbas;
                break;
            case UBX_GNSS_GALILEO:
                ++display_state.sat_count.galileo;
                break;
            case UBX_GNSS_BEIDOU:
                ++display_state.sat_count.beidou;
                break;
            case UBX_GNSS_QZSS:
                ++display_state.sat_count.qzss;
                break;
            case UBX_GNSS_GLONASS:
                ++display_state.sat_count.glonass;
                break;
            case UBX_GNSS_NAVIC:
                ++display_state.sat_count.navic;
                break;
            default:
                break;
        }
    }
    ILOG(TAG, "gnss: %hhu, count: %hhu, G:%hhu, S:%hhu, E:%hhu, B:%hhu, Q:%hhu, R:%hhu, N:%hhu", rtc_config.gnss, nav_sat->numSvs, display_state.sat_count.gps, display_state.sat_count.sbas, display_state.sat_count.galileo, display_state.sat_count.beidou, display_state.sat_count.qzss, display_state.sat_count.glonass, display_state.sat_count.navic);
}

static size_t update_gps_info_row_str(const struct ubx_config_s *ubx_dev, char * p) {
    if(!ubx_dev) return 0;
    char * pc = p;
    if(ubx_dev->setup_progress) {
        memcpy(pc, "initializing", 12), pc += 12;
    } else if(ubx_dev->ready) {
        if(rtc_config.hw_type == UBX_HW_TYPE_DEFAULT) {
            memcpy(pc, "gps not found", 13), pc += 13;
            goto end;
        }
        update_sat_count(ubx_dev);
        uint8_t gnss = rtc_config.gnss;
        //pc += xultoa(ubx_dev->ubx_msg.navPvt.numSV, pc);
        memcpy(pc, "sat:", 4), pc += 4;
        if(BIT_GET(gnss, UBX_GNSS_GPS)!=0) {
            *pc++ = ' ';
            *pc++ = 'G';
            pc += xultoa(display_state.sat_count.gps, pc);
        }
        if(BIT_GET(gnss, UBX_GNSS_GALILEO)!=0) {
            *pc++ = ' ';
            *pc++ = 'E';
            pc += xultoa(display_state.sat_count.galileo, pc);
        }
        if(BIT_GET(gnss, UBX_GNSS_BEIDOU)!=0) {
            *pc++ = ' ';
            *pc++ = 'B';
            pc += xultoa(display_state.sat_count.beidou, pc);
        }
        if(BIT_GET(gnss, UBX_GNSS_GLONASS)!=0) {
            *pc++ = ' ';
            *pc++ = 'R';
            pc += xultoa(display_state.sat_count.glonass, pc);
        }
        memcpy(pc, " h", 2), pc += 2;
        pc += f2_to_char(ubx_dev->ubx_msg.navDOP.hDOP/100.0f, pc);
        memcpy(pc, " a", 2), pc += 2;
        // pc += xultoa(ubx_dev->ubx_msg.navPvt.sAcc/10, pc); // speed accuracy in cm/s
        pc += f1_to_char(ubx_dev->ubx_msg.navPvt.sAcc/1000.0f, pc);
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
    memcpy(pb, "bat: ", 5), pb += 5;
#if defined(CONFIG_LOGGER_ADC_ENABLED)
    float bat_voltage;
    get_battery_voltage_for_display(0, m_context_rtc.RTC_voltage_bat, &bat_voltage);
    pb += f2_to_char(bat_voltage, pb);
#else
    pb += f2_to_char(m_context_rtc.RTC_voltage_bat, pb);
#endif
    memcpy(pb, "V ", 2), pb += 2;
    if(gps->first_fix){
        memcpy(pb, " fx: ", 5), pb += 5;
        pb += xultoa(MS_TO_SEC(gps->first_fix), pb);
        *pb++ = 's';
    }
    if(gps->lost_frames) {
        memcpy(pb, " l: ", 4), pb += 4;
        pb += xultoa(gps->lost_frames, pb);
    }
    *pb = 0;
    return pb - p;
}

static uint32_t _update_screen(const struct display_s *me, const screen_mode_t screen_mode, void *arg) {
    uint32_t buf_update_count = display_get_buf_update_count();
    FUNC_ENTRY_ARGS(TAG, " buf_update_count:%ld mode: %d", buf_update_count, screen_mode);
    uint32_t ret = 0;
    UNUSED_PARAMETER(ret);
    if(display_refresh_lock(portMAX_DELAY) == pdTRUE) {
        logger_config_t *config = m_app_ctx.config;
        char str[24] = {0}, *p = str, str1[32]={0}, *pb = str1, str2[32]={0}, *pc = str2;
        bool is_gps_stat_screen = (screen_mode >= SCREEN_MODE_SPEED_STATS_1 && screen_mode <= SCREEN_MODE_SPEED_STATS_10);
        display_state.update_delay = 50;
        // ESP_LOGI(TAG, "update screen: mode:%" PRIu8 ", update nr:%lu", screen_mode, buf_update_count);
        int state = (arg ? *(int*)arg : 0);
        printf("_update screen: mode:%d, state:%d\n", screen_mode, state);
        int isnew = 0;
        lv_obj_t *panel = 0;
        stat_screen_t *sc_data = 0;
        const struct gps_context_s *gps = &m_app_ctx.ctx->gps;
        const struct ubx_config_s *ubx_dev = gps->ubx_device;
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
                
#if defined(CONFIG_LCD_IS_EPD)
                ui_set_main_cnt_offset(&ui_info_screen.screen, offset);
#endif
                isnew = showGpsScreen(display_state.gps_image_angle);
                if(isnew)
                    gps_info_cb(0);
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
            case SCREEN_MODE_CHARGE:
                if(state == ADC_BATTERY_CHARGING_STARTED)
                    memcpy(pb, "Charging ", 9), pb += 9;
                else if(state == ADC_BATTERY_CHARGING_STOPPED)
                    memcpy(pb, "Stopped ", 8), pb += 8;
                else if(state == ADC_BATTERY_NORMAL)
                    memcpy(pb, "Good ", 5), pb += 5;
                else if(state == ADC_BATTERY_HIGH)
                    memcpy(pb, "Full ", 5), pb += 5;
                pb += f2_to_char(m_context_rtc.RTC_voltage_bat, pb);
                showChargeScreen(state, &str1[0]);
                display_state.update_delay = 1000;
                break;
            case SCREEN_MODE_SD_TROUBLE:
                showSdTroubleScreen();
                break;
            case SCREEN_MODE_LOW_BAT:
                link_for_low_bat:
                memcpy(pb, "Battery low ", 12), pb += 12;
                pb += f2_to_char(m_context_rtc.RTC_voltage_bat, pb);
                showLowBatScreen(&str1[0]);
                break;
            case SCREEN_MODE_FW_UPDATE:
                const v_settings_t *s = arg;
                const struct m_config_item_s *i = (const struct m_config_item_s *)s->settings_data;
#if defined(CONFIG_LCD_IS_EPD)
                ui_set_main_cnt_offset(&ui_info_screen.screen, offset);
#endif
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
                        memcpy(pb, "Time: ", 6), pb += 6;
                        avail_fields[fld_total_time_hms].value.timestr(pb);
                        memcpy(pc, "Distance: ", 10), pc += 10;
                        pc += f2_to_char(avail_fields[fld_distance].value.num(), pc);
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
                display_state.update_delay = 350;
                break;
            case SCREEN_MODE_SPEED_1:
#if defined(CONFIG_LOGGER_SPEED_SCREEN_VARIANT)
            link_for_screen_mode_speed_2:
#endif
#if defined(CONFIG_LCD_IS_EPD)
                ui_set_main_cnt_offset(&ui_speed_screen.screen, offset);
#endif
                isnew = showSpeedScreen();
                if(isnew) {
                    speed_cb(0);
                    speed_info_bar_update();
                }

                break;
#if defined(CONFIG_LOGGER_SPEED_SCREEN_VARIANT)
            case SCREEN_MODE_SPEED_2:
                goto link_for_screen_mode_speed_2;
                break;
#endif
            case SCREEN_MODE_SPEED_STATS_1:
            case SCREEN_MODE_SPEED_STATS_2:
            case SCREEN_MODE_SPEED_STATS_3:
            case SCREEN_MODE_SPEED_STATS_4:
            case SCREEN_MODE_SPEED_STATS_5:
            case SCREEN_MODE_SPEED_STATS_6:
            case SCREEN_MODE_SPEED_STATS_7:
            case SCREEN_MODE_SPEED_STATS_8:
            case SCREEN_MODE_SPEED_STATS_9:
            case SCREEN_MODE_SPEED_STATS_10:
                sc_data = &sc_screens[screen_mode-SCREEN_MODE_SPEED_STATS_1];
                break;
            case SCREEN_MODE_WIFI_START:
            case SCREEN_MODE_WIFI_AP:
            case SCREEN_MODE_WIFI_STATION:
                DLOG(TAG, "[%s] %s, wifi %d", __func__, scr_mode_str, screen_mode);
                // display_state.update_delay = 100;
#if defined(CONFIG_LCD_IS_EPD)
                ui_set_main_cnt_offset(&ui_info_screen.screen, offset);
#endif
                isnew = showWifiScreen();
                if (isnew)
                    wifi_info_cb(0);
                // statusbar_update();
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
                display_state.update_delay = 800;
                break;
            case SCREEN_MODE_SETTINGS:
                if(arg) {
                    const v_settings_t *s = arg;
                    const struct m_config_item_s *i = (const struct m_config_item_s *)s->settings_data;
                    if(!i) goto plain_setting;
#if defined(CONFIG_LCD_IS_EPD)
                    ui_set_main_cnt_offset(&ui_info_screen.screen, offset);
#endif
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
                DLOG(TAG, "[%s] stats panel: 6Row x 1Slot", __func__);
                rows = 6, cols = 1;
#else
                DLOG(TAG, "[%s] stats panel: 3Row x 2Slot", __func__);
                rows = 3, cols = 2;
#endif
            } 
            else if (sc_data->num_fields == 4) {
#if defined(CONFIG_SSD168X_PANEL_SSD1681)
                DLOG(TAG, "[%s] stats panel: 4Row x 1Slot", __func__);
                rows = 4, cols = 1;
#else
                DLOG(TAG, "[%s] stats panel: 2Row x 2Slot", __func__);
                rows = 2, cols = 2;
#endif
            } else if (sc_data->num_fields == 2) {
                DLOG(TAG, "[%s] stats panel: 2Row x 1Slot", __func__);
                rows = 2, cols = 1;
            } else {
                DLOG(TAG, "[%s] stats panel: 3Row x 1Slot", __func__);
                rows = 3, cols = 1;
            }
#if defined(CONFIG_LCD_IS_EPD)
            ui_set_main_cnt_offset(&ui_stats_screen.screen, offset);
#endif
            loadStatsScreen(rows, cols);
            for(c=0; c < cols; c++) {
                for(r = 0; r < rows; r++) {
                    n = c*rows+r;
                    panel = ui_stats_screen.cells[r][c].title;
                    if(sc_data->fields[n].field->type == SCR_TYPE_TIME_HMS) {
                        lv_obj_set_width(lv_obj_get_parent(panel), lv_pct(90));
                        lv_obj_set_style_text_font(panel, ui_stats_screen.font.title_small, LV_PART_MAIN | LV_STATE_DEFAULT);
                    }
#if defined(TEST_NUM)
                    get_display_fld_str(sc_data->fields[n].field, p, fake_f2_to_char);
#else
                    get_display_fld_str(sc_data->fields[n].field, p, f2_to_char);
#endif
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
        if(display_state.screen_mode_counter && display_state.screen_mode_counter%5 == 0) {
            if(display_state.offset_mark > 4) {
                display_state.offset_mark_dir = 1;
            }
            else if(display_state.offset_mark == 0) {
                display_state.offset_mark_dir = 0;
            }
            if(!display_state.offset_mark_dir) ++display_state.offset_mark;
            else --display_state.offset_mark;
        }
        // if(display_state.display->task_resumed_for_times) --display_state.display->task_resumed_for_times;
        if(buf_update_count > 0 && buf_update_count%100==0){
            display_request_full_refresh(0);
        }
#endif
        // display_incr_buf_update_count();
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
    FUNC_ENTRY_ARGS(TAG, "%ld app_mode: %s, cur_screen: %s, next_screen: %s", buf_update_count, app_mode_str[ctx->app_mode], cur_screen_str[ctx->cur_screen], cur_screen_str[ctx->next_screen]);
    DMEAS_START();
#endif
    // struct display_s *dspl = &display;
    uint32_t delay=0;
    if(!display_state.display || !display_state.display->op) {
        goto end;
    }
    // bool run_is_active = false;
    struct gps_context_s *gps = &m_app_ctx.ctx->gps;
#ifdef CONFIG_UBLOX_ENABLED
    const struct ubx_config_s *ubx_dev = gps->ubx_device;
#endif
    if(ctx->app_mode == APP_MODE_GPS) {
        // run_is_active = (ctx->config && gps && gps->signal_ok && MM_S_TO_M_S(gps->gps_speed) >= m_app_ctx.config->screen.stat_speed);
        if (gps->gps_is_moving && ctx->next_screen != CUR_SCREEN_NONE){
            ctx->next_screen = CUR_SCREEN_NONE;
        }
        // ctx->stat_screen_count = m_app_ctx.ctx->stat_screen_count;
        // if (ctx->stat_screen_count > gps_stat_screen_item_count)
        //     ctx->stat_screen_count = gps_stat_screen_item_count;
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

    // Check if ADC has notified us of a charge event or if we're in charge mode
    if(adc_check_and_clear_lcd_charge_flag() || ctx->app_mode == APP_MODE_CHARGE) {
        int32_t adc_state = get_adc_state();
        delay = _update_screen(display_state.display, SCREEN_MODE_CHARGE, (void*)(&adc_state));
        // Flag is automatically cleared by adc_check_and_clear_lcd_charge_flag()
        ctx->cur_screen = CUR_SCREEN_CHARGE;
#if defined(CONFIG_LCD_IS_EPD)
        if(ctx->app_mode == APP_MODE_CHARGE) goto fasttimer;
#endif
        goto end;
    }

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
        // display_incr_buf_update_count();
        goto end;
    }
    else if ((!m_app_ctx.screen_auto_refresh && display_get_buf_update_count() < 2) || ctx->app_mode == APP_MODE_BOOT) {
        DLOG(TAG, "[%s] Boot screen requested lcd_count: %lu, app_mode: %s", __func__, buf_update_count, app_mode_str[ctx->app_mode]);
        delay = _update_screen(display_state.display, SCREEN_MODE_BOOT, 0);
        ctx->cur_screen = CUR_SCREEN_BOOT;
        goto end;
    } 
    
    else if (ctx->app_mode == APP_MODE_SHUT_DOWN || ctx->app_mode == APP_MODE_RESTART || m_app_ctx.ctx->request_shutdown || m_app_ctx.ctx->request_restart) {
        delay = _update_screen(display_state.display, SCREEN_MODE_SHUT_DOWN, 0);
        ctx->cur_screen = CUR_SCREEN_OFF_SCREEN;
        goto end;
    }

    else if(get_adc_state() == ADC_BATTERY_CRITICAL_LOW) {
        delay=_update_screen(display_state.display, SCREEN_MODE_LOW_BAT, 0);
        ctx->cur_screen = CUR_SCREEN_LOW_BAT;
        goto end;
    }
    else if(!m_app_ctx.ctx->sdOK) {
        delay=_update_screen(display_state.display, SCREEN_MODE_SD_TROUBLE, 0);
        ctx->cur_screen = CUR_SCREEN_SD_TROUBLE;
        goto end;
    }
    
    if (ctx->next_screen == CUR_SCREEN_SETTINGS) {
        struct m_config_item_s item = {0};
        if(ctx->cfg_screen == CFG_GROUP_GPS) {
            get_gps_cfg_item(ctx->gps_cfg_item, &item);
            const v_settings_t s = { CFG_GROUP_GPS, "GPS", &item };
            delay = _update_screen(display_state.display, SCREEN_MODE_SETTINGS, (void*)&s);
        } else if(ctx->cfg_screen == CFG_GROUP_STAT_SCREENS) {
            get_stat_screen_cfg_item(ctx->stat_screen_cfg_item, &item);
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
        if (ubx_dev->ubx_msg.count_nav_pvt > 10 && gps_read_msg_timeout(5) && ctx->next_screen == CUR_SCREEN_NONE) {
#if C_LOG_LEVEL < 2
            WLOG(TAG, "[%s] gps read msg timeout!, show trouble screen", __func__);
#endif
            gpstrblscr:
            delay=_update_screen(display_state.display, SCREEN_MODE_GPS_TROUBLE, 0);  // gps signal lost !!!
            ctx->cur_screen = CUR_SCREEN_GPS_TROUBLE;
        } else if ((ctx->next_screen != CUR_SCREEN_GPS_STATS && (!ubx_dev || !gps_has_version_set())) || (!gps->gps_is_moving && ctx->next_screen == CUR_SCREEN_GPS_INFO)) {
            // if(!ubx_dev->ubx_msg.mon_ver.hwVersion[0]) goto bootscreen;
            delay=_update_screen(display_state.display, SCREEN_MODE_GPS_INIT, 0);
            ctx->cur_screen = CUR_SCREEN_GPS_INFO;
        }
        else if (!gps->gps_is_moving && (time_display_max_speed(time_2s) > ONE_M_S_IN_MM_S || ctx->next_screen == CUR_SCREEN_GPS_STATS)) {
            if (gps->record && ctx->record_done == RECORD_DONE_END) {
                if(time_display_max_speed(time_2s) > 10000) // when more than 32k/h show records
                    ctx->record_done=RECORD_DONE_START;
                gps->record = 0;
            }
            if (time_display_record(time_10s) && ctx->record_done < RECORD_DONE_2) { // 10sec max record
                struct record_forwarder_s r = { &avail_fields[fld_s10_r1_display], &avail_fields[fld_s10_r2_display], ctx->record_done==RECORD_DONE_START};
                delay=_update_screen(display_state.display, SCREEN_MODE_RECORD, &r);
                ++ctx->record_done;
                goto end;
            }
            else if (time_display_record(time_2s) && ctx->record_done < RECORD_DONE_4) { // 2sec max record
                if(ctx->record_done<2) ctx->record_done = RECORD_DONE_2;
                struct record_forwarder_s r = { &avail_fields[fld_s2_r1_display], &avail_fields[fld_s2_r2_display] , ctx->record_done== RECORD_DONE_2};
                delay=_update_screen(display_state.display, SCREEN_MODE_RECORD, &r);
                ++ctx->record_done;
                goto end;
            }
            else if (dist_display_record(dist_250m) && ctx->record_done < RECORD_DONE_6) { // 250m max record
                if(ctx->record_done<4) ctx->record_done = RECORD_DONE_4;
                struct record_forwarder_s r = { &avail_fields[fld_m250_r1_display], &avail_fields[fld_m250_r2_display] , ctx->record_done==4};
                delay=_update_screen(display_state.display, SCREEN_MODE_RECORD, &r);
                ++ctx->record_done;
                goto end;
            }
            else if (dist_display_record(dist_500m) && ctx->record_done < RECORD_DONE_6) { // 500m max record
                if(ctx->record_done<6) ctx->record_done = RECORD_DONE_6;
                struct record_forwarder_s r = { &avail_fields[fld_m500_r1_display], &avail_fields[fld_m500_r2_display] , ctx->record_done== RECORD_DONE_6};
                delay=_update_screen(display_state.display, SCREEN_MODE_RECORD, &r);
                ++ctx->record_done;
                goto end;
            }
            else if (dist_display_record(dist_1852m) && ctx->record_done < RECORD_DONE_6) { // 1852m max record
                if(ctx->record_done<8) ctx->record_done = RECORD_DONE_8;
                struct record_forwarder_s r = { &avail_fields[fld_m1852_r1_display], &avail_fields[fld_m1852_r2_display] , ctx->record_done== RECORD_DONE_8};
                delay=_update_screen(display_state.display, SCREEN_MODE_RECORD, &r);
                ++ctx->record_done;
                goto end;
            }
             else if (alfa_display_record(alfa_500m) && ctx->record_done < RECORD_DONE_12) { // 500m alfa max record
                if(ctx->record_done<10) ctx->record_done = RECORD_DONE_10;
                struct record_forwarder_s r = { &avail_fields[fld_a500_r1_display], &avail_fields[fld_a500_r2_display] , ctx->record_done== RECORD_DONE_10};
                delay=_update_screen(display_state.display, SCREEN_MODE_RECORD, &r);
                ++ctx->record_done;
                goto end;
            }
            else if(ctx->record_done <  RECORD_DONE_OK) {
                ctx->record_done =  RECORD_DONE_OK;
            }
            int i = 0;
            while(BIT_GET(c_gps_cfg.stat_screens, m_app_ctx.ctx->stat_screen_cur) == 0) {
                if(++m_app_ctx.ctx->stat_screen_cur >= gps_stat_screen_item_count) { // 16 fields used eq uint16_t
                    m_app_ctx.ctx->stat_screen_cur = 0;
                }
                if(i++ >= gps_stat_screen_item_count) { // no active screens, show first one.
                    m_app_ctx.ctx->stat_screen_cur = 0;
                    break;
                }
            }
            delay=_update_screen(display_state.display, SCREEN_MODE_SPEED_STATS_1 + m_app_ctx.ctx->stat_screen_cur, 0);
            ctx->cur_screen = CUR_SCREEN_GPS_STATS;
#if defined(CONFIG_LCD_IS_EPD)
            if (!gps->gps_is_moving && !display_task_is_paused()) {
                display_start_task_pause_seq(); // should be 3 for pause loop
            }
#endif
        } else {
            if(ubx_dev && !gps_has_version_set()) {
#if (C_LOG_LEVEL < 2)
                WLOG(TAG, "[%s] gps not ready or not ver set, show trouble screen", __func__);
#endif
                goto gpstrblscr;
            }
#if defined(CONFIG_LOGGER_SPEED_SCREEN_VARIANT)
            if (ctx->config && ctx->config->screen.speed_large_font == 2)
                delay=_update_screen(display_state.display, SCREEN_MODE_SPEED_2, 0);
            else
#endif
                delay=_update_screen(display_state.display, SCREEN_MODE_SPEED_1, 0);
            m_app_ctx.ctx->stat_screen_cur = 0;
            // ctx->stat_screen_count = m_app_ctx.ctx->stat_screen_count;
            ctx->cur_screen = CUR_SCREEN_GPS_SPEED;
            ctx->record_done = 255;
            ctx->gps_cfg_item = CFG_GPS_ITEM_BASE;
        }
#else
        delay=_update_screen(display_state.display, SCREEN_MODE_GPS_INIT, 0);
        cur_screen = CUR_SCREEN_GPS_INFO;
#endif
#if defined(CONFIG_LCD_IS_EPD)
        if (!gps->gps_is_moving && !display_task_is_paused()) {
            display_start_task_pause_seq(); // should be 3 for pause loop
        }
#endif
    }
    end:
    if(!m_app_ctx.screen_auto_refresh && ctx->cur_screen == CUR_SCREEN_BOOT) {
        delay_ms(250);
    }
#if defined(CONFIG_LCD_IS_EPD)
    if(ctx->cur_screen == CUR_SCREEN_WIFI && ((!m_app_ctx.wifi_ctx->s_sta_connection && !m_app_ctx.wifi_ctx->s_ap_connection) || (m_app_ctx.wifi_ctx->s_sta_connection && !m_app_ctx.wifi_ctx->s_sta_connected)) ) {
        goto fasttimer;
    }
    if((ctx->cur_screen == CUR_SCREEN_GPS_TROUBLE 
    || ctx->cur_screen == CUR_SCREEN_SD_TROUBLE 
    || ctx->cur_screen == CUR_SCREEN_LOW_BAT)) {
        fasttimer:
        if(!get_display_timer_period()){
            display_timer_set_period(5);
        }
    }
    else if(get_display_timer_period()) {
        display_timer_set_period(0);
    }
#endif
    update_lv_timers();
#if (C_LOG_LEVEL < 2)
    DMEAS_END(TAG, "[%s] took: %llu us",  __FUNCTION__);
#endif
    return delay;
}

display_op_t screen_ops = {
    .screen_cb = screen_cb,
    .ui_init = ui_init,
    .ui_deinit = ui_deinit,
};

static bool lcd_initialized = false;

struct display_s *lcd_init() {
    FUNC_ENTRY(TAG);
    if(lcd_initialized) return display_state.display;
    display_init(display_state.display, &screen_ops);
    lcd_initialized = true;
    return display_state.display;
}

void lcd_deinit() {
    FUNC_ENTRY(TAG);
    if(!lcd_initialized) return;
    if(lcd_initialized) {
        lcd_initialized = false;
    }
    stop_lv_timers();
    display_uninit(display_state.display);
    display_state.display = 0;
}

#endif
