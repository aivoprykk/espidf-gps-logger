

#include <string.h> 

#include "esp_log.h"

#include "adc.h"

#include "display.h"
#include "private.h"
#include <epdg.h>
#include <gfxfont.h>

#if defined(CONFIG_BMX_ENABLE)
#include <bmx280.h>
#include "bmx.h"
#endif
#include "button.h"
#include "context.h"
#include "dstat_screens.h"
#include "logger_config.h"
#include "str.h"
#include "ubx.h"
#include "wifi.h"

//#include <epdg.h>
//#include <gfxfont.h>

#define DELAY_TIME 1000
#define BG_COLOR 0xffff
#define FG_COLOR 0x0000

#define ROW_SPACING 2
#define INFO_BAR_HEIGHT 20
#define DISPLAY_HEIGHT (screen._height == 128 ? 122 : screen._height)
#define DISPLAY_TOP_PAD (screen._height == 128 ? 6 : 0)
#define INFO_BAR_TOP (screen._height - INFO_BAR_HEIGHT)
#define INFO_BAR_ROW (screen._height - 2)

extern struct context_s m_context;
extern struct context_rtc_s m_context_rtc;
//extern struct UBXMessage ubxMessage;
extern struct m_wifi_context wifi_context;
extern stat_screen_t sc_screens[];

static const char *TAG = "epaper";
static epd_g_t screen;
static uint8_t offset = 0;
TIMER_INIT

#ifdef CONFIG_HAS_SCREEN_DEPG0213BN
#include "depg0213bn.h"
depg0213bn_t screen_driver;
#endif
#ifdef CONFIG_HAS_SCREEN_GDEY0213B74
#include "gdey0213b74.h"
gdey0213b74_t screen_driver;
#endif
#ifdef CONFIG_HAS_SCREEN_GDEY0154D67
#include "gdey0154d67.h"
gdey0154d67_t screen_driver;
#endif


typedef struct {
    const char name[32];
    const GFXfont *font;
    uint16_t w, h, w1, h1;
    uint32_t len;
} font_type_t;
#define FONT_TYPE_DEFAULT {0, 0, 0, 0, 0, 0}

typedef struct {
    // const char *ipart;
    // const char *fpart;
    uint32_t ilen;
    uint16_t iwidth;
    uint32_t flen;
    uint16_t fwidth;
    int16_t xpos;
    uint16_t maxh;
    font_type_t *ffont;
    font_type_t *ifont;
    const GFXfont *fontbank[8];  // max 4 fonts to choose
    uint8_t use_i_font_for_f, have_f_font, have_i_font, num_fonts_in_bank;
    void *display;
} speed_num_font_t;
#define SPEED_NUM_FONT_DEFAULT { \
    0, 0, 0, 0, 0, 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0, 0, 0, 0, 0 \
}
struct display_state_s {
    speed_num_font_t speed_1_fonts;
    speed_num_font_t speed_2_fonts;
    uint8_t update_epaper;
    uint32_t update_delay;
    bool initialized;
};
#define DISPLAY_STATE_DEFAULT {SPEED_NUM_FONT_DEFAULT, SPEED_NUM_FONT_DEFAULT, 0, 0, false}

static esp_err_t reset_display_state(struct display_state_s *display_state) {
    LOGR
    display_state->initialized = 0;
    memset(&display_state->speed_1_fonts, 0, sizeof(speed_num_font_t));
    memset(&display_state->speed_2_fonts, 0, sizeof(speed_num_font_t));
    return ESP_OK;
}

struct display_priv_s {
    struct display_state_s state;
    int16_t displayHeight;
    int16_t displayWidth;
    uint8_t test_field, test_font;
    bool displayOK;
};

struct display_priv_s display_priv = {DISPLAY_STATE_DEFAULT, 0, 0, 0, 0, false};

#include "Fonts/BitmapSurfbuddies.h"

#include "Fonts/FreeMonoBold9pt7b.h"  //gebruikt
/* #include "Fonts/FreeMonoBold12pt7b.h"  //gebruikt
 */
#include "Fonts/FreeSansBold12pt7b.h"  //gebruikt
#include "Fonts/FreeSansBold18pt7b.h"  //gebruikt
#include "Fonts/FreeSansBold24pt7b.h"  //gebruikt
#include "Fonts/FreeSansBold30pt7b.h"  //gebruikt
#include "Fonts/FreeSansBold60pt7b.h"
#include "Fonts/FreeSansBold6pt7b.h"   //gebruikt
#include "Fonts/FreeSansBold75pt7b.h"  //gebruikt
#include "Fonts/FreeSansBold9pt7b.h"   //gebruikt

#include "Fonts/SF_Distant_Galaxy9pt7b.h"  //surfbuddies

#include "Fonts/SansSerif_bold_26.h"     //bijgevoegd in lib
#include "Fonts/SansSerif_bold_46_nr.h"  //bijgevoegd in lib
#include "Fonts/SansSerif_bold_84_nr.h"  //bijgevoegd in lib
#include "Fonts/SansSerif_bold_96_nr.h"  //bijgevoegd in lib

/* #include "Fonts/ubuntu/Ubuntu_Bold8pt7b.h" */
/* #include "Fonts/ubuntu/Ubuntu_Medium8pt7b.h"
#include "Fonts/ubuntu/Ubuntu_Medium12pt7b.h"

#include "Fonts/roboto/RobotoCondensed_Bold12pt7b.h"
#include "Fonts/roboto/RobotoCondensed_Bold18pt7b.h"
#include "Fonts/roboto/RobotoCondensed_Bold24pt7b.h"
#include "Fonts/roboto/RobotoCondensed_Bold30pt7b.h"
#include "Fonts/roboto/RobotoCondensed_Bold42pt7b.h"
#include "Fonts/roboto/RobotoCondensed_Bold60pt7b.h"
#include "Fonts/roboto/RobotoCondensed_Bold75pt7b.h"
#include "Fonts/roboto/RobotoCondensed_Bold84pt7b.h"
#include "Fonts/roboto/RobotoCondensed_Bold8pt7b.h"
#include "Fonts/roboto/RobotoCondensed_Bold96pt7b.h" */

font_type_t fonts[] = {
    {.name = {"FreeSansBold6pt7b"}, .font = &FreeSansBold6pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},            // 2
    {.name = {"FreeMonoBold9pt7b"}, .font = &FreeMonoBold9pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},            // 0
    {.name = {"FreeSansBold9pt7b"}, .font = &FreeSansBold9pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},            // 3
    {.name = {"SF_Distant_Galaxy9pt7b"}, .font = &SF_Distant_Galaxy9pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},  // 8
    /* {.name = {"FreeMonoBold12pt7b"}, .font = &FreeMonoBold12pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},                  // 1
     */
    {.name = {"FreeSansBold12pt7b"}, .font = &FreeSansBold12pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},  // 4
    {.name = {"FreeSansBold18pt7b"}, .font = &FreeSansBold18pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},  // 5
    {.name = {"FreeSansBold24pt7b"}, .font = &FreeSansBold24pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},  // 6
    {.name = {"FreeSansBold30pt7b"}, .font = &FreeSansBold30pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},  // 6
    {.name = {"FreeSansBold60pt7b"}, .font = &FreeSansBold60pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},
    {.name = {"FreeSansBold75pt7b"}, .font = &FreeSansBold75pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},  // 7

    {.name = {"SansSerif_bold_26"}, .font = &SansSerif_bold_26, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},        // 9
    {.name = {"SansSerif_bold_46_nr"}, .font = &SansSerif_bold_46_nr, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},  // 9
    {.name = {"SansSerif_bold_84_nr"}, .font = &SansSerif_bold_84_nr, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},  // 10
    {.name = {"SansSerif_bold_96_nr"}, .font = &SansSerif_bold_96_nr, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},  // 11 */
    /* {.name = {"Ubuntu_Bold8pt7b"}, .font = &Ubuntu_Bold8pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},                      // 12
    {.name = {"Ubuntu_Medium8pt7b"}, .font = &Ubuntu_Medium8pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},                  // 13
    {.name = {"Ubuntu_Medium12pt7b"}, .font = &Ubuntu_Medium12pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},                // 14
    {.name = {"RobotoCondensed_Bold8pt7b"}, .font = &RobotoCondensed_Bold8pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},    // 15
    {.name = {"RobotoCondensed_Bold12pt7b"}, .font = &RobotoCondensed_Bold12pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},  // 16
    {.name = {"RobotoCondensed_Bold18pt7b"}, .font = &RobotoCondensed_Bold18pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},  // 17
    {.name = {"RobotoCondensed_Bold30pt7b"}, .font = &RobotoCondensed_Bold30pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},  // 18
    {.name = {"RobotoCondensed_Bold42pt7b"}, .font = &RobotoCondensed_Bold42pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},  // 18
    {.name = {"RobotoCondensed_Bold75pt7b"}, .font = &RobotoCondensed_Bold75pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},  // 18
    {.name = {"RobotoCondensed_Bold84pt7b"}, .font = &RobotoCondensed_Bold84pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},  // 18
    {.name = {"RobotoCondensed_Bold96pt7b"}, .font = &RobotoCondensed_Bold96pt7b, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0},  // 18 */
    {.name = {0}, .font = 0, .w = 0, .h = 0, .w1 = 0, .h1 = 0, .len = 0}  // 19
};

#define DISPLAY_PRINT(str) screen.op->print(&screen, (str));
#define DISPLAY_SET_TEXT_COLOR(c) screen.op->setTextColor(&screen, (c), (c));
#define DISPLAY_SET_FONT(f) screen.op->setFont(&screen, (f));
#define DISPLAY_SET_CURSOR(x, y) screen.op->setCursor(&screen, (x), (y));
#define DISPLAY_FILL_SCREEN(x) epd_fillScreen(&screen, (x));

static void init_font(font_type_t *ft) {
    TIMER_S
    if (!ft || !*ft->name)
        return;
    int16_t x = 0, y = 0, cursor_x = 0, y1 = 0;
    uint16_t w = 0, h = 0, i = 0, j = 0;
    ft->len = strlen(ft->name);
    DISPLAY_SET_FONT(ft->font);
    DISPLAY_SET_CURSOR(5, 5);
    const char *numbers[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
    for (i = 0, j = 10; i < j; ++i) {
        if (!numbers[i] || !*numbers[i])
            continue;
        screen.op->getTextBounds(&screen, numbers[i], x, y, &cursor_x, &y1, &w, &h);
#ifdef DEBUG
        ESP_LOGD(TAG, "getTextBounds for '%s' font:%s x:%" PRId16 " y:%" PRId16 " w:%" PRIu16 " h:%" PRIu16, numbers[i], ft->name, cursor_x, y1, w, h);
#endif
        if (w > ft->w)
            ft->w = w;
        if (h > ft->h)
            ft->h = h;
    }
    ESP_LOGD(TAG, "init font:%s nw: %" PRIu16 " nh: %" PRIu16, ft->name, ft->w, ft->h);
    TIMER_E
}

static esp_err_t init_fonts(font_type_t *_fonts, uint16_t len) {
    for (uint16_t i = 0, j = len; i < j; ++i) {
        init_font(&_fonts[i]);
    }
    return ESP_OK;
}

static font_type_t *get_font(const GFXfont *f, font_type_t **ft) {
    assert(f);
    for (uint16_t i = 0, j = sizeof(fonts)/sizeof(fonts[0]); i < j; ++i) {
        if (!fonts[i].font)
            break;
        if (fonts[i].font == f) {
            *ft = &fonts[i];
            ESP_LOGD(TAG, "[get_font] %s w: %" PRIu16 " h: %" PRIu16 " w1: %" PRIu16 " h1: %" PRIu16, (*ft)->name, (*ft)->w, (*ft)->h, (*ft)->w1, (*ft)->h1);
            break;
        }
    }
    return *ft;
}

static int16_t get_field_width(uint16_t strlen, font_type_t *f, uint8_t add_comma) {
    uint16_t width = (strlen * (f)->w) + (add_comma ? f->w : 0);
    return width;
}

static int16_t pass_font(speed_num_font_t *ft, uint8_t f_depth) {
    LOGR
        uint16_t i,
        j, k, l, width = 0;
    font_type_t *ipart = 0, *fpart = 0;
    for (i = 0, j = ft->num_fonts_in_bank; i < j; ++i) {
        if (ft->fontbank[i])
            get_font(ft->fontbank[i], &ipart);
        else
            continue;
        if (ft->ifont != ipart)
            ft->ifont = ipart;
        else
            continue;

        ft->iwidth = get_field_width(ft->ilen, ft->ifont, 0);
        if ((ft->ifont->h + ROW_SPACING) > ft->maxh)
            continue;
        for (k = i, l = k + f_depth, j = ft->num_fonts_in_bank; k < j; ++k) {
            if (f_depth && l == k)
                break;
            if (ft->fontbank[k]) {
                if (ft->fontbank[k] != ipart->font)
                    get_font(ft->fontbank[k], &fpart);
                else
                    fpart = ipart;
            } else
                continue;
            if (fpart->w > ipart->w && k + 1 == l)
                ++l;
            if (ft->ffont != fpart)
                ft->ffont = fpart;
            else
                continue;
            ft->fwidth = get_field_width(ft->flen, ft->ffont, 1);
            width = ft->iwidth + ft->fwidth;
            ESP_LOGD(TAG, "[%s] ipart:%s fpart:%s iwidth: %" PRIu16 " fwidth: %" PRIu16 " width: %" PRIu16, __FUNCTION__, (ft->ifont)->name, (ft->ffont)->name, ft->iwidth, ft->fwidth, width);
            if (width < screen._width) {
                ft->xpos = (screen._width - width) / 2;
                goto done;
            }
        }
    }
done:
    ft->have_i_font = 1;
    ft->have_f_font = 1;
    ESP_LOGD(TAG, "[pass_font] passed fonts: ipart:%s fpart:%s xpos: %" PRId16, (ft->ifont)->name, (ft->ffont)->name, ft->xpos);
    return ft->xpos;
}

#define VCENTER_V(vh, h) (((vh) / 2) + ((h) / 2))
#define VCENTER(h) VCENTER_V(screen._height, h)
#define VCENTER_AT_V(vh, h1, h) VCENTER_V(vh - (h1), h)
#define VCENTER_AT(h1, h) VCENTER_V(screen._height - (h1), h)

#define SET_FONT(_f, _s) \
    get_font(_s, &_f);   \
    assert(_f);          \
    DISPLAY_SET_FONT(_f->font);

#define HCENTER_AT(_w, _f, _len) \
    cursor_x = offset + ((_w) < ((_len) * f->w) ? 0 : (((_w) - ((_len) * f->w)) / 2))

#define SET_FONT_CENTER(_f, _s, _len) \
    SET_FONT(_f, _s)                  \
    HCENTER_AT(screen._width, _f, _len)

static void Speed_in_Unit(int offset) {
    font_type_t *f = 0;
    SET_FONT(f, &FreeSansBold6pt7b);
    uint8_t cursor_x = screen._width - 3 - offset,
            cursor_y = (DISPLAY_HEIGHT / 2 - (f->w * 14 / 2)) - DISPLAY_TOP_PAD + INFO_BAR_TOP;
    uint8_t rotation = screen.rotation;
    screen.op->setRotation(&screen, !rotation);
    DISPLAY_SET_CURSOR(cursor_y, cursor_x);  // was 30, 249
    if ((int)(m_context_rtc.RTC_calibration_speed * 100000) == 194)
        DISPLAY_PRINT("speed in knots");  // 1.94384449 m/s to knots !!!
    if ((int)(m_context_rtc.RTC_calibration_speed * 1000000) == 3600)
        DISPLAY_PRINT("speed in km/h");
    screen.op->setRotation(&screen, rotation);
}

static void sdCardInfo(int offset, int skip_if_not_ok) {
    TIMER_S
    if (m_context.sdOK != true && skip_if_not_ok)
        return;
    DISPLAY_PRINT("SD ");
    if (m_context.sdOK != true) {
        DISPLAY_PRINT("fail!!!");
    } else if (m_context.freeSpace > 0) {
        char tmp[24] = {0};
        sprintf(tmp, "free %"PRIu32"Mb", m_context.freeSpace);
        DISPLAY_PRINT(tmp);
    } else {
        DISPLAY_PRINT("OK");
    }
    TIMER_E
}

static int device_boot_log_1_row(int sdcard_void, int16_t cursor_x, int16_t cursor_y, uint16_t row_height, uint8_t row) {
    TIMER_S
    offset = m_context_rtc.RTC_offset;
    int ret = 500;
    char tmp[16] = {0}, *p=tmp;
    cursor_y += (row * row_height);
    offset += cursor_x;
    // printf("[%s] r:%" PRIu8 " y1:%" PRIu16 " y2:%" PRIu16 "\n", __FUNCTION__, row, cursor_y - row_height, cursor_y);
    //  screen.op->fillRect(&screen, 1, cursor_y-row_height+3, screen._width, row_height, BG_COLOR);
    DISPLAY_SET_CURSOR(offset, cursor_y);
    if (row == 1) {
        DISPLAY_PRINT("Version ");
        DISPLAY_PRINT(VERSION_STRING);
    } else if (row == 2) {
        DISPLAY_PRINT("Display size ");
        sprintf(tmp, "%dx%d", screen.screenop->get_height(), screen.screenop->get_width());
        DISPLAY_PRINT(tmp);
    } else if (row == 3) {
        sdCardInfo(offset, sdcard_void);
    } else if (row == 4) {
        DISPLAY_PRINT("Gps ");
        DISPLAY_PRINT(ubx_chip_str(m_context.gps.ublox_config));
        // sprintf(tmp, " %d Hz", config->sample_rate);
        DISPLAY_PRINT(tmp);
    } else if (row == 5) {
        if (m_context_rtc.RTC_voltage_bat > 0) {
            DISPLAY_PRINT("Battery: ");
            size_t len = f_to_char(m_context_rtc.RTC_voltage_bat, p, 3);
            p+=len;
            memcpy(p," V", 2);
            *(p+2)=0;
            DISPLAY_PRINT(tmp);
        }
    } else
        ret = 100;
    TIMER_E
    return ret;
}

static int DateTimeRtc(int offset) {
    LOGR if (m_context_rtc.RTC_year < 2023) return 0;
    // DISPLAY_SET_FONT(&RobotoCondensed_Bold8pt7b);
    DISPLAY_SET_FONT(&FreeSansBold9pt7b);
    DISPLAY_SET_CURSOR(screen._width>200?offset:1, INFO_BAR_ROW);
    char tmp[24] = {0};
    sprintf(tmp, "%02hu:%02hu %02hu-%02hu-%04hu", m_context_rtc.RTC_hour, m_context_rtc.RTC_min, m_context_rtc.RTC_day, m_context_rtc.RTC_month, m_context_rtc.RTC_year);
    DISPLAY_PRINT(tmp);
    return 0;
}

static uint32_t Bat_level_Simon(int offset) {
    // if (!m_context_rtc.rtc_initialized)
    //     return 0;
    uint8_t bat_perc = calc_bat_perc_v(m_context_rtc.RTC_voltage_bat);
    ESP_LOGD(TAG, "[%s] battery level: %d", __FUNCTION__, bat_perc);

    int batW = 8;
    int batL = 15;
    int posX = screen._width - batW - 7;
    int posY = screen._height - batL;
    int line = 2;
    int seg = 3;
    int segW = batW - 2 * line;
    int segL = (batL - 0.25 * batW - 2 * line - (seg - 1)) / seg;
    screen.op->fillRect(&screen, offset + posX, posY, 0.5 * batW, 0.25 * batW, FG_COLOR);                 // battery top
    screen.op->fillRect(&screen, offset + posX - 0.25 * batW, posY + 0.25 * batW, batW, batL, FG_COLOR);  // battery body
    if (bat_perc < 67) {
        screen.op->fillRect(&screen, offset + posX - 0.25 * batW + line, posY + 0.25 * batW + line, segW, segL, BG_COLOR);
    }
    if (bat_perc < 33) {
        screen.op->fillRect(&screen, offset + posX - 0.25 * batW + line, posY + 0.25 * batW + line + 1 * (segL + 1), segW, segL, BG_COLOR);
    }
    if (bat_perc < 1) {
        screen.op->fillRect(&screen, offset + posX - 0.25 * batW + line, posY + 0.25 * batW + line + 2 * (segL + 1), segW, segL, BG_COLOR);
    }
    // DISPLAY_SET_FONT(&Ubuntu_Medium8pt7b);
    font_type_t *f=0;
    SET_FONT(f, &FreeSansBold9pt7b);
    if (bat_perc < 100) {
        DISPLAY_SET_CURSOR(posX - (4 * f->w) + offset, (INFO_BAR_ROW));  // 193 - 232 = 39
    } else {
        DISPLAY_SET_CURSOR(posX - (5 * f->w) + offset, (INFO_BAR_ROW));  // 184 - 232 = 48
    }
    char tmp[4] = {0};
    sprintf(tmp, "%"PRIu8"%%", bat_perc);
    DISPLAY_PRINT(tmp);
    return 0;
}

static void M8_M10(int offset) {
    LOGR
    // DISPLAY_SET_FONT(&Ubuntu_M8pt8b);
    DISPLAY_SET_FONT(&FreeSansBold6pt7b);  // width 6pt
    int xpos = screen._width - 140 + offset - (m_context.config->sample_rate < 10 ? 15 : 21);
    DISPLAY_SET_CURSOR(xpos, INFO_BAR_ROW);  // 250-110=140
    char tmp[4] = {0};
    tmp[0] = '@';
    xltoa(m_context.config->sample_rate, &(tmp[1]));
    DISPLAY_PRINT(ubx_chip_str(m_context.gps.ublox_config));
    DISPLAY_PRINT(tmp);
}

static esp_err_t update_time(struct tm * tms) {
    // if (!m_context_rtc.rtc_initialized)
    //     return -1;
    LOGR
    esp_err_t ret = 0;
    /* int32_t millis = esp_timer_get_time() / 1000;
    if (!m_context.NTP_time_set) {
        if (m_context.gps.ublox_config->ready && (!m_context.gps.Gps_time_set || millis > next_time_sync)) {
            Set_GPS_Time(&m_context, config->timezone);
            next_time_sync = millis + (60 * 1000);
        }
    } else if (!m_context.gps.ublox_config->ready && m_context.gps.Gps_time_set) {
        m_context.gps.Gps_time_set = 0;
    }
    if (!m_context.gps.Gps_time_set && !m_context.NTP_time_set)
        return -1; */
    memset(tms, 0, sizeof(struct tm));
    getLocalTime(tms, 0);
    ESP_LOGD(TAG, "[%s] time: %02d-%02d-%d %02d:%02d:%02d source: %s", __FUNCTION__, tms->tm_mday, (tms->tm_mon) + 1, (tms->tm_year) + 1900, tms->tm_hour, tms->tm_min, tms->tm_sec, m_context.NTP_time_set ? "ntp" : m_context.gps.ublox_config->time_set ? "gps"
                                                                                                                                                                                                                                       : "none");
    return ret;
}

static esp_err_t Time(int offset) {
    struct tm tms;
    update_time(&tms);
    if (tms.tm_year > 122) {
        DISPLAY_SET_FONT(&FreeSansBold9pt7b);
        DISPLAY_SET_CURSOR(offset, INFO_BAR_ROW);
        char tmp[24] = {0};
        sprintf(tmp, "%02d:%02d:%02d", tms.tm_hour, tms.tm_min, tms.tm_sec);
        DISPLAY_PRINT(tmp);
    }
    return 0;
}

static void Sats_level(int offset) {
    const ubx_msg_t *ubxMessage = &m_context.gps.ublox_config->ubx_msg;
    if (!ubxMessage->mon_ver.swVersion[0]) return;
    int posX = screen._width - 85 + offset;               // 250-167 = 117;
    int posY = INFO_BAR_TOP + (INFO_BAR_HEIGHT / 2) - 6;  //-(circelL+2*circelS);
    if (wifi_context.Wifi_on) {
        screen.op->fillRect(&screen, posX - 21, posY, 21 + 15, 15, BG_COLOR);
        return;
    }
    int satnum = ubxMessage->navPvt.numSV;
    // display->drawGrayscaleBitmap(posX, posY, ESP_Sat_15, 15, 15);
    epd_drawBitmap(&screen, posX, posY, ESP_Sat_15, 15, 15, FG_COLOR, bm_default);
    // DISPLAY_SET_FONT(&Ubuntu_M8pt8b);
    DISPLAY_SET_FONT(&FreeSansBold9pt7b);
    DISPLAY_SET_CURSOR(posX - (satnum < 10 ? 12 : 21), INFO_BAR_ROW);
    char tmp[4] = {0};
    sprintf(tmp, "%d", satnum);
    DISPLAY_PRINT(tmp);
}

static void InfoBar(int offset) {
    // if (!m_context_rtc.rtc_initialized)
    //     return;
    LOGR
    const ubx_msg_t *ubxMessage = &m_context.gps.ublox_config->ubx_msg;
    screen.op->fillRect(&screen, 1, DISPLAY_HEIGHT - INFO_BAR_HEIGHT + DISPLAY_TOP_PAD, screen._width, INFO_BAR_HEIGHT, BG_COLOR);
    Bat_level_Simon(offset);
    Sats_level(offset);
    if (screen._width>200 && m_context.gps.ublox_config->ready && ubxMessage->navPvt.numSV > 4)
        M8_M10(offset);
    Time(offset);
}

static void InfoBarRtc(int offset) {
    // if (!m_context_rtc.rtc_initialized)
    //     return;
    
    screen.op->fillRect(&screen, 1, DISPLAY_HEIGHT - INFO_BAR_HEIGHT + DISPLAY_TOP_PAD, screen._width, INFO_BAR_HEIGHT, BG_COLOR);
    Bat_level_Simon(offset);
    DateTimeRtc(offset);
    
}

static uint32_t epaper_boot_screen(struct display_s *me) {
    TIMER_S
    if (!me->self->displayOK)
        return 100;
    offset = m_context_rtc.RTC_offset;
    uint32_t ret = 0;
    int16_t cursor_y = 0, fcursor_y = 0, cursor_x;
    font_type_t *f = 0;
    DISPLAY_SET_TEXT_COLOR(FG_COLOR);
    if (m_context.boot_screen_stage <= 1) {
        DISPLAY_FILL_SCREEN(BG_COLOR);
    }

    if (m_context_rtc.RTC_voltage_bat < MINIMUM_VOLTAGE) {
        SET_FONT_CENTER(f, &FreeSansBold12pt7b, 14);
        char tmp[24], *p = tmp;
        cursor_y = DISPLAY_TOP_PAD + VCENTER_V(DISPLAY_HEIGHT - INFO_BAR_HEIGHT, f->h);
        DISPLAY_PRINT("Going to sleep");
        SET_FONT(f, &FreeSansBold9pt7b);
        DISPLAY_SET_CURSOR(cursor_x, (cursor_y += (f->h + ROW_SPACING)));
        DISPLAY_PRINT("Voltage to low: ");
        f_to_char(m_context_rtc.RTC_voltage_bat, p, 3);
        DISPLAY_PRINT(p);
        DISPLAY_SET_CURSOR(cursor_x, (cursor_y += (f->h + ROW_SPACING)));
        DISPLAY_PRINT("Please charge battery!");
        ret = 1000;
    } else if (m_context.boot_screen_stage) {
        SET_FONT(f, screen._height > 128 ? &FreeSansBold9pt7b : &FreeSansBold6pt7b);
        int16_t row_height = f->h + 5;
        uint8_t block_height = (5 * row_height) + (screen._height > 128 ? 5 : 3);
        fcursor_y = (DISPLAY_HEIGHT - INFO_BAR_HEIGHT - block_height);
        if (screen._height > 128) {
            cursor_y = DISPLAY_TOP_PAD + (fcursor_y / 2 - 24);
            cursor_x = screen._width / 2 - 24;
        } else {
            cursor_y = (DISPLAY_HEIGHT / 2 - 24);
            cursor_x = 10;
        }
        epd_drawBitmap(&screen, cursor_x, cursor_y, ESP_GPS_logo, 48, 48, EPD_BLACK, bm_default);
        if (screen._height <= 128) {
            cursor_x = 68;
            fcursor_y /= 2;
        }
        else {
            cursor_x = 10;
        }
        cursor_y = fcursor_y + DISPLAY_TOP_PAD;
        ret = device_boot_log_1_row(1, cursor_x, cursor_y, row_height, m_context.boot_screen_stage);
    }
    if (m_context.boot_screen_stage < 1) {  // 9p font used for info bar
        InfoBarRtc(offset);
    } else {
        InfoBar(offset);
    }
    if (m_context.boot_screen_stage > 1) {
        epd_displayWindow(&screen, 0, cursor_y, screen._width, screen._height - cursor_y);
        //epd_displayWindow(&screen, 0, 0, screen._width, screen._width > screen._height && screen._height == 128 ? 122 : screen._height);
    } else {
        epd_display(&screen, false);
    }
    TIMER_E
    return ret;
}

static void update_offset() {
    //int8_t offset = offset;
    if (offset > 9)
        offset--;
    if (offset < 1)
        offset++;
}

static void update_offset_w_count(uint8_t count) {
    if (count % 20 < 10)
        offset++;
    else
        offset--;
    if (offset > 10)
        offset = 0;
    if (offset < 1)
        offset = 0;
}

static uint32_t epaper_off_screen(struct display_s *me, int choice) {
    TIMER_S
    if (!me->self->displayOK)
        return 500;
    uint32_t milli = get_millis();
    int ret = 0;
    float session_time = (milli - m_context.gps.start_logging_millis) / 1000;
    if (session_time > 0) {
        ESP_LOGD(TAG, "session time: %.2f s", session_time);
    }

    // display->setRotation(1);
    DISPLAY_FILL_SCREEN(BG_COLOR);
    DISPLAY_SET_TEXT_COLOR(FG_COLOR);
    int16_t cursor_x = 0, cursor_y = 0, len = 0;
    // float rounded = 0;
    char tmp[24], *p = tmp;
    uint8_t col1, col2, row;
    font_type_t *f = 0;
    const char *title = m_context.request_restart ? "Reboot device" : m_context.Shut_down_Save_session ? "Saving session"
                                                                                                       : "Going to sleep";
    len = strlen(title);
    SET_FONT_CENTER(f, &FreeSansBold12pt7b, len);
    col1 = cursor_x;
    col2 = cursor_x + 70;
    row = f->h + ROW_SPACING;
    cursor_y = VCENTER_AT(INFO_BAR_HEIGHT, row);
    DISPLAY_SET_CURSOR(cursor_x, cursor_y);
    DISPLAY_PRINT(title);
    if (choice == 0) {
        if (m_context.Shut_down_Save_session == true) {
            SET_FONT(f, &FreeSansBold9pt7b);
            DISPLAY_SET_CURSOR(col1, (cursor_y += (row)));
            DISPLAY_PRINT("Time:");
            f_to_char(session_time, p, 0);
            DISPLAY_SET_CURSOR(col2, cursor_y);
            DISPLAY_PRINT(p);
            DISPLAY_SET_CURSOR(col1, (cursor_y += (row)));
            DISPLAY_PRINT(" AVG:");
            f2_to_char(m_context_rtc.RTC_avg_10s, p);
            DISPLAY_SET_CURSOR(col2, cursor_y);
            DISPLAY_PRINT(p);
            DISPLAY_SET_CURSOR(col1, (cursor_y += (row)));
            DISPLAY_PRINT("Dist:");
            f2_to_char((m_context.gps.Ublox.total_distance / 1000), p);
            DISPLAY_SET_CURSOR(col2, cursor_y);
            DISPLAY_PRINT(p);
        }
    } else {
        ESP_LOGD(TAG, "Off_screen Simon");
        if (m_context.Shut_down_Save_session == true) {
            SET_FONT(f, &FreeSansBold9pt7b);
            DISPLAY_SET_CURSOR(col1, (cursor_y += (row)));
            DISPLAY_PRINT("Time:");
            size_t len = f_to_char(session_time, p, 0);
            p+=len;
            *p='s';
            ++p;
            *p='e';
            ++p;
            *p='c';
            ++p;
            *p=0;
            DISPLAY_SET_CURSOR(col2, cursor_y);
            DISPLAY_PRINT(tmp);
            DISPLAY_SET_CURSOR(col1, (cursor_y += (row)));
            DISPLAY_PRINT(" AVG:");
            f2_to_char((m_context_rtc.RTC_avg_10s), p);
            DISPLAY_SET_CURSOR(col2, cursor_y);
            DISPLAY_PRINT(tmp);
            DISPLAY_SET_CURSOR(col1, (cursor_y += (row)));
            DISPLAY_PRINT("Dist:");
            len = f2_to_char((m_context.gps.Ublox.total_distance / 1000), p);
            p+=len;
            *p='k';
            ++p;
            *p='m';
            ++p;
            *p=0;
            DISPLAY_SET_CURSOR(col2, cursor_y);
            DISPLAY_PRINT(tmp);
        }
    }
    InfoBar(offset);
    epd_displayWindow(&screen, 0, 0, screen._width - 1, screen._height - 1);
    ret += 4000;
    TIMER_E
    return ret;
}

static uint32_t epaper_sleep_screen(struct display_s *me, int choice) {
    TIMER_S
    if (!me->self->displayOK)
        return 500;
    update_offset();
    DISPLAY_FILL_SCREEN(BG_COLOR);
    DISPLAY_SET_TEXT_COLOR(FG_COLOR);
    InfoBarRtc(offset);
    char tmp[24], *p = tmp;
    uint8_t top_pad = DISPLAY_TOP_PAD, upper_h, row, col1, col2;
    if (screen._height > 128) {
        top_pad += 10;
    }
    int16_t cursor_y = top_pad;
    // int16_t cursor_x;
    font_type_t *f = 0;
    if (choice == 0) {
        SET_FONT(f, &FreeSansBold18pt7b);
        row = f->h + ROW_SPACING;
        upper_h = (4 * row);
        cursor_y += (DISPLAY_HEIGHT - INFO_BAR_HEIGHT - upper_h) / 2;
        col1 = 20 + offset;
        col2 = 100 + offset;
        DISPLAY_SET_CURSOR(offset + col1, (cursor_y += row));
        DISPLAY_PRINT("Dist:");
        DISPLAY_SET_CURSOR(offset + col2, cursor_y);
        f2_to_char(m_context_rtc.RTC_distance, p);
        DISPLAY_PRINT(p);  // 0
        DISPLAY_SET_CURSOR(offset + col1, (cursor_y += row));
        DISPLAY_PRINT(" AVG:");
        DISPLAY_SET_CURSOR(offset + col2, cursor_y);
        f2_to_char(m_context_rtc.RTC_avg_10s, p);
        DISPLAY_PRINT(p);  // 2
        DISPLAY_SET_CURSOR(offset, (cursor_y += row));
        DISPLAY_PRINT(" 2sF:");
        DISPLAY_SET_CURSOR(offset + col2, cursor_y);
        f2_to_char(m_context_rtc.RTC_max_2s, p);
        DISPLAY_PRINT(p);
        DISPLAY_SET_CURSOR(offset, (cursor_y += row));
        DISPLAY_PRINT("10sF:");
        DISPLAY_SET_CURSOR(offset + col2, cursor_y);
        f2_to_char(m_context_rtc.RTC_R1_10s, p);
        DISPLAY_PRINT(p);
        epd_display(&screen, true);
    } else {
        col1 = 0 + offset;
        col2 = 34 + offset;
        uint8_t col3 = 90 + offset;
        uint8_t col4 = 146 + offset;
        uint8_t pX, pY, pX2, pY2;
        SET_FONT(f, &FreeSansBold9pt7b);  // 13px
        row = f->h + ROW_SPACING;
        upper_h = (6 * row);
        uint8_t lower_h = DISPLAY_HEIGHT - INFO_BAR_HEIGHT - upper_h;
        SET_FONT(f, &SF_Distant_Galaxy9pt7b);
        if (screen._height > 128) {  // 200x200
            cursor_y += upper_h + (ROW_SPACING * 3 / 2) + f->h;
            pX = 20;
            pX2 = 80;
            pY = pY2 = cursor_y - f->h + (lower_h) / 2 - 24;
        } else {  // 250x128
            cursor_y += upper_h + (ROW_SPACING * 3 / 2) + VCENTER_V(lower_h, f->h);
            pX = pX2 = 196,
            pY = DISPLAY_TOP_PAD + 3;
            pY2 = DISPLAY_TOP_PAD + 52;
        }
        // ESP_LOGD(TAG, "[%s] upperh:%" PRIu8 " lowerh:%" PRIu8 " fonth:%" PRIu16 " '%s' cursor_y:%" PRId16, __FUNCTION__, upper_h, lower_h, f->h, m_context_rtc.RTC_Sleep_txt, cursor_y);
        if (m_context_rtc.RTC_Board_Logo == 1)  // Logo's Simon Dijkstra
            epd_drawBitmap(&screen, pX, pY, Starboard_logoS_zwart, 48, 48, FG_COLOR, bm_default);
        else if (m_context_rtc.RTC_Board_Logo == 2)
            epd_drawBitmap(&screen, pX, pY, Fanatic_logoS_zwart, 48, 48, FG_COLOR, bm_default);
        else if (m_context_rtc.RTC_Board_Logo == 3)
            epd_drawBitmap(&screen, pX, pY, JP_logoS_zwart, 48, 48, FG_COLOR, bm_default);
        else if (m_context_rtc.RTC_Board_Logo == 4)
            epd_drawBitmap(&screen, pX, pY, NoveNove_logoS_zwart, 48, 48, FG_COLOR, bm_default);
        else if (m_context_rtc.RTC_Board_Logo == 5)  // Logo's Jan Scholten
            epd_drawBitmap(&screen, pX, pY, Mistral_logoS_zwart, 48, 48, FG_COLOR, bm_default);
        else if (m_context_rtc.RTC_Board_Logo == 6)
            epd_drawBitmap(&screen, pX, pY, Goya_logoS_zwart, 48, 48, FG_COLOR, bm_default);
        else if (m_context_rtc.RTC_Board_Logo == 7)
            epd_drawBitmap(&screen, pX, pY, Patrik_logoS_zwart, 48, 48, FG_COLOR, bm_default);
        else if (m_context_rtc.RTC_Board_Logo == 8)
            epd_drawBitmap(&screen, pX, pY, Severne_logoS_zwart, 48, 48, FG_COLOR, bm_default);
        else if (m_context_rtc.RTC_Board_Logo == 9)
            epd_drawBitmap(&screen, pX, pY, Tabou_logoS_zwart, 48, 48, FG_COLOR, bm_default);
        else if (m_context_rtc.RTC_Board_Logo == 10)
            epd_drawBitmap(&screen, pX, pY, F2_logo_zwart, 48, 48, FG_COLOR, bm_default);
        else if (m_context_rtc.RTC_Board_Logo == 12)
            epd_drawBitmap(&screen, pX, pY, FutureFly_logo_zwart, 48, 48, FG_COLOR, bm_default);
        else if (m_context_rtc.RTC_Board_Logo == 13)
            epd_drawBitmap(&screen, pX, pY, ESP_GPS_logo, 48, 48, FG_COLOR, bm_default);
        // row 2 //
        // Zeil Logo's:
        if (m_context_rtc.RTC_Sail_Logo == 1)  // Logo's Simon Dijkstra
            epd_drawBitmap(&screen, pX2, pY2, GAsails_logoS_zwart, 48, 48, FG_COLOR, bm_default);
        else if (m_context_rtc.RTC_Sail_Logo == 2)
            epd_drawBitmap(&screen, pX2, pY2, DuoTone_logoS_zwart, 48, 48, FG_COLOR, bm_default);
        else if (m_context_rtc.RTC_Sail_Logo == 3)
            epd_drawBitmap(&screen, pX2, pY2, NP_logoS_zwart, 48, 48, FG_COLOR, bm_default);
        else if (m_context_rtc.RTC_Sail_Logo == 4)
            epd_drawBitmap(&screen, pX2, pY2, Pryde_logoS_zwart, 48, 48, FG_COLOR, bm_default);
        else if (m_context_rtc.RTC_Sail_Logo == 5)  // Logo's Jan Scholten
            epd_drawBitmap(&screen, pX2, pY2, Loftsails_logoS_zwart, 48, 48, FG_COLOR, bm_default);
        else if (m_context_rtc.RTC_Sail_Logo == 6)
            epd_drawBitmap(&screen, pX2, pY2, Gunsails_logoS_zwart, 48, 48, FG_COLOR, bm_default);
        else if (m_context_rtc.RTC_Sail_Logo == 7)
            epd_drawBitmap(&screen, pX2, pY2, Point7_logoS_zwart, 48, 48, FG_COLOR, bm_default);
        else if (m_context_rtc.RTC_Sail_Logo == 8)
            epd_drawBitmap(&screen, pX2, pY2, Simmer_logoS_zwart, 48, 48, FG_COLOR, bm_default);
        else if (m_context_rtc.RTC_Sail_Logo == 9)
            epd_drawBitmap(&screen, pX2, pY2, Naish_logoS_zwart, 48, 48, FG_COLOR, bm_default);
        else if (m_context_rtc.RTC_Sail_Logo == 10) {  // Severne as Sail logo !!!
            epd_drawBitmap(&screen, pX2, pY2, Severne_logoS_zwart, 48, 48, FG_COLOR, bm_default);
        }

        DISPLAY_SET_CURSOR(col1, cursor_y);  // was 121
        DISPLAY_PRINT(m_context_rtc.RTC_Sleep_txt);

        Speed_in_Unit(offset);

        // left column
        cursor_y = top_pad + row;
        DISPLAY_SET_FONT(&FreeMonoBold9pt7b);
        DISPLAY_SET_CURSOR(col1, cursor_y);
        DISPLAY_PRINT("AV:");
        DISPLAY_SET_CURSOR(col1, (cursor_y += row));
        DISPLAY_PRINT("R1:");
        DISPLAY_SET_CURSOR(col1, (cursor_y += row));
        DISPLAY_PRINT("R2:");
        DISPLAY_SET_CURSOR(col1, (cursor_y += row));
        DISPLAY_PRINT("R3:");
        DISPLAY_SET_CURSOR(col1, (cursor_y += row));
        DISPLAY_PRINT("R4:");
        DISPLAY_SET_CURSOR(col1, (cursor_y += row));
        DISPLAY_PRINT("R5:");

        cursor_y = top_pad + row;
        DISPLAY_SET_FONT(&FreeSansBold9pt7b);
        DISPLAY_SET_CURSOR(col2, cursor_y);
        f2_to_char(m_context_rtc.RTC_avg_10s, p);
        DISPLAY_PRINT(p);  // 2
        DISPLAY_SET_CURSOR(col2, (cursor_y += row));
        f2_to_char(m_context_rtc.RTC_R1_10s, p);
        DISPLAY_PRINT(p);  // 2
        DISPLAY_SET_CURSOR(col2, (cursor_y += row));
        f2_to_char(m_context_rtc.RTC_R2_10s, p);
        DISPLAY_PRINT(p);  // 2
        DISPLAY_SET_CURSOR(col2, (cursor_y += row));
        f2_to_char(m_context_rtc.RTC_R3_10s, p);
        DISPLAY_PRINT(p);  // 2
        DISPLAY_SET_CURSOR(col2, (cursor_y += row));
        f2_to_char(m_context_rtc.RTC_R4_10s, p);
        DISPLAY_PRINT(p);  // 2
        DISPLAY_SET_CURSOR(col2, (cursor_y += row));
        f2_to_char(m_context_rtc.RTC_R5_10s, p);
        DISPLAY_PRINT(p);  // 2

        // right column
        cursor_y = top_pad + row;
        DISPLAY_SET_FONT(&FreeMonoBold9pt7b);
        DISPLAY_SET_CURSOR(col3, cursor_y);
        DISPLAY_PRINT("2sec:");
        DISPLAY_SET_CURSOR(col3, (cursor_y += row));
        DISPLAY_PRINT("Dist:");
        DISPLAY_SET_CURSOR(col3, (cursor_y += row));
        DISPLAY_PRINT("Alph:");
        DISPLAY_SET_CURSOR(col3, (cursor_y += row));
        DISPLAY_PRINT("  1h:");  //
        DISPLAY_SET_CURSOR(col3, (cursor_y += row));
        DISPLAY_PRINT("  NM:");
        DISPLAY_SET_CURSOR(col3, (cursor_y += row));
        DISPLAY_PRINT("500m:");

        cursor_y = top_pad + row;
        DISPLAY_SET_FONT(&FreeSansBold9pt7b);
        DISPLAY_SET_CURSOR(col4, cursor_y);
        f2_to_char(m_context_rtc.RTC_max_2s, p);
        DISPLAY_PRINT(p);  // 2
        DISPLAY_SET_CURSOR(col4, (cursor_y += row));
        f2_to_char(m_context_rtc.RTC_distance, p);
        DISPLAY_PRINT(p);  // 2
        DISPLAY_SET_CURSOR(col4, (cursor_y += row));
        f2_to_char(m_context_rtc.RTC_alp, p);
        DISPLAY_PRINT(p);  // 2
        DISPLAY_SET_CURSOR(col4, (cursor_y += row));
        f2_to_char(m_context_rtc.RTC_1h, p);
        DISPLAY_PRINT(p);  // 2//
        DISPLAY_SET_CURSOR(col4, (cursor_y += row));
        f2_to_char(m_context_rtc.RTC_mile, p);
        DISPLAY_PRINT(p);  // 2
        DISPLAY_SET_CURSOR(col4, (cursor_y += row));
        f2_to_char(m_context_rtc.RTC_500m, p);
        DISPLAY_PRINT(p);  // 2

        epd_display(&screen, true);
    }
    TIMER_E
    return 0;
}

#define SPEED_INFO_BOTTOM_HEIGHT 8

static int count = 0;
static int old_screen = 0;
static int low_speed_seconds = 0;

static esp_err_t show_speed_info_bar(const float calspd) {  // info bar when config->speed_large_font is 1 or 0
    LOGR
        uint8_t speed_info_bar_height = DISPLAY_HEIGHT / 3;
    /// speed info row on 40pt top
    const logger_config_t *config = m_context.config;
    int run_rectangle_length;
    uint8_t field = m_context.config->speed_field;          // default is in config.txt
    uint16_t bar_length = config->bar_length * 1000 / 240;  // default 100% length = 1852 m
    uint8_t font_size = m_context.config->speed_large_font;
    if (screen._width > 200 && font_size!=1)
        update_offset_w_count(count);
    else
        offset = 1;
    if (config->speed_field == 1) {  // only switch if config.field==1 !!!
        if (((int)(m_context.gps.Ublox.total_distance / 1000000) % 10 == 0) && (m_context.gps.Ublox.alfa_distance / 1000 > 1000))
            field = 5;  // indien x*10km, totale afstand laten zien
        // if(m_context.gps.S10.s_max_speed<(m_context.gps.S10.display_speed[5]*0.95))field=8;//if
        // run slower dan 95% of slowest run, show 1h result
        if ((m_context.gps.Ublox.alfa_distance / 1000 < 350) && (m_context.gps.alfa_window < 100))
            field = 3;  // first 350 m after gibe  alfa screen !!
        if (m_context.gps.Ublox.alfa_distance / 1000 > config->bar_length)
            field = 4;  // run longer dan 1852 m, NM scherm !!
    } else if (config->speed_field == 2) {
        if (m_context.gps.Ublox.run_distance / 1000 > config->bar_length)
            field = 4;  // if run longer dan 1852 m, NM scherm !!
    } else if (config->speed_field == 7) {
        if ((m_context.gps.Ublox.alfa_distance / 1000 < 350) && (m_context.gps.alfa_window < 100))
            field = 3;  // first 350 m after gibe  alfa screen !!
        else
            field = 7;
    } else if (config->speed_field == 8) {
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

    const GFXfont *f1 = &FreeSansBold9pt7b, *f2;
    uint8_t speed_info_bar_col2_x = screen._width / 2 + 1;

    if (font_size == 1) {  // info row font is bigger when fontsize == 1
        // cursor_y = 36;
        // bar_position = cursor_y + 7;  // test voor bigger font, was 42
        // speed_info_bar_col2_x = 113;
        f2 = &SansSerif_bold_46_nr;
    } else {
        f2 = &FreeSansBold18pt7b;
    }
    size_t len;
    font_type_t *f=0;
    SET_FONT(f, f2);
    uint8_t cursor_y = DISPLAY_TOP_PAD + VCENTER_V(speed_info_bar_height - SPEED_INFO_BOTTOM_HEIGHT, f->h + ROW_SPACING) - ROW_SPACING / 2;
    uint8_t bar_position = speed_info_bar_height - SPEED_INFO_BOTTOM_HEIGHT - ROW_SPACING / 2 + DISPLAY_TOP_PAD;
    double s[] = {0, 0};
    const char *var[] = {0, 0};
    char val[][24] = {{0}, {0}}, *p;

    DISPLAY_SET_TEXT_COLOR(FG_COLOR);
    // double s1 = 0, s2 = 0;
    if (field <= 2 || display_priv.test_field == 2) {
        s[0] = m_context.gps.S10.s_max_speed * calspd;
        s[1] = m_context.gps.S10.avg_5runs * calspd;
        f1_to_char(s[0], val[0]);
        f1_to_char(s[1], val[1]);
        if (font_size == 0) {
            var[0] = "Run ";
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
        bar_length = 250 * 1000 / 240;  // full bar length with Alfa = 250 meter
        if ((m_context.gps.alfa_window < 99) && (m_context.gps.Ublox.alfa_distance / 1000 < 255)) {
            if (m_context.gps.alfa_exit > 99)
                m_context.gps.alfa_exit = 99;  // begrenzen alfa_exit...
            var[0] = "Gate ";
            len = f_to_char(m_context.gps.alfa_window, val[0], 0);
            var[1] = "Ex ";
            len = f_to_char(m_context.gps.alfa_exit, val[1], 0);
        } else {
            s[0] = m_context.gps.A500.display_max_speed * calspd;
            s[1] = m_context.gps.A500.alfa_speed_max * calspd;
            f1_to_char(s[0], val[0]);
            if (font_size == 0) {
                var[0] = "Alfa ";
                if (s[1] > 1) {        // laatste alfa was geldig !!!!
                    var[1] = "Amax ";  // nieuwe alfa laatste gijp or MISSED !!!!
                    f1_to_char(s[1], val[1]);
                } else {
                    var[1] = 0;
                    memcpy(val[1], "MISS", 4);
                    val[1][4]=0;
                }
            } else {
                // DISPLAY_PRINT(m_context.gps.A500.display_max_speed*calibration_speed);//1
                // //best Alfa from session on 500 m !!
                var[0] = "Alfa= ";
                if (s[1] > 1) {  // laatste alfa was geldig !!!!
                    f1_to_char(s[1], val[1]);
                } else {
                    memcpy(val[1], "MISS", 4);
                    val[1][4]=0;
                }
                var[1] = 0;
                *val[0] = 0;
            }
        }
    } else if (field == 4 || display_priv.test_field == 4) {
        s[0] = m_context.gps.M1852.m_max_speed * calspd;
        f1_to_char(s[0], val[0]);
        if (font_size == 0) {
            var[0] = "NMa ";  // Actuele nautical mile
            var[1] = "NM ";
            s[1] = m_context.gps.M1852.display_max_speed * calspd;
            f1_to_char(s[1], val[1]);
        } else {
            var[0] = "NM= ";  // Actuele nautical mile
            var[1] = 0;
            *val[1] = 0;
        }
    } else if (field == 5 || display_priv.test_field == 5) {
        var[0] = "Dist ";
        len = f1_to_char((m_context.gps.Ublox.total_distance / 1000000), val[0]);
        if (m_context.gps.Ublox.total_distance / 1000000 > 99.9){
            p=val[0]+len;
            memcpy(p, " km", 3);
            *(p+3)=0;
        }
    } else if (field == 6 || display_priv.test_field == 6) {
        s[0] = m_context.gps.S2.display_max_speed * calspd;
        s[1] = m_context.gps.S10.display_max_speed * calspd;
        var[0] = "2S ";
        f1_to_char(s[0], val[0]);
        var[1] = "10S ";
        f1_to_char(s[1], val[1]);
    } else if (field == 7 || display_priv.test_field == 7) {
        s[0] = m_context.gps.S1800.avg_s * calspd;
        s[1] = m_context.gps.S1800.display_max_speed * calspd;
        var[0] = ".5hA";
        f1_to_char(s[0], val[0]);
        var[1] = ".5hB";
        f1_to_char(s[1], val[1]);
    } else if (field == 8 || display_priv.test_field == 8) {
        s[0] = m_context.gps.S3600.avg_s * calspd;
        s[1] = m_context.gps.S3600.display_max_speed * calspd;
        var[0] = "1hA ";
        f1_to_char(s[0], val[0]);
        var[1] = "1hB ";
        f1_to_char(s[1], val[1]);
    }

    if (s[0] > 100 || s[1] > 100) {
        if (font_size == 1) {
            f2 = &SansSerif_bold_26;
        } else {
            f2 = &FreeSansBold12pt7b;
        }
    }

    // col1
    DISPLAY_SET_FONT(f1);
    DISPLAY_SET_CURSOR(offset, cursor_y);
    if (var[0]) {
        DISPLAY_PRINT(var[0]);
    }
    if (*val[0]) {
        DISPLAY_SET_FONT(f2);
        DISPLAY_PRINT(val[0]);  // 1 // last 10s max from run
    }
    // col 2
    DISPLAY_SET_FONT(f1);
    DISPLAY_SET_CURSOR(offset + speed_info_bar_col2_x, cursor_y);
    if (var[1]) {
        DISPLAY_PRINT(var[1]);
    }
    if (*val[1]) {
        DISPLAY_SET_FONT(f2);
        DISPLAY_PRINT(val[1]);  // 1
    }

    int32_t millis = get_millis();
    int log_seconds = (millis - m_context.gps.start_logging_millis) / 1000;  // aantal seconden sinds loggen is gestart
    if (m_context.gps.S10.avg_s > 2000) {
        low_speed_seconds = 0;
    }  // if the speed is higher then 2000 mm/s, reset the counter
    low_speed_seconds++;
    if (low_speed_seconds > 120) {
        m_context.gps.start_logging_millis = millis;
    }                                                                     // bar will be reset if the 10s speed drops under 2m/s for more then 120 s !!!!
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
    if (bar_length) {
        screen.op->fillRect(&screen, offset, DISPLAY_TOP_PAD + bar_position, run_rectangle_length, SPEED_INFO_BOTTOM_HEIGHT, FG_COLOR);  // balk voor run_distance weer te geven...
    }
    return ESP_OK;
}

static esp_err_t speed_screen_1(const float calspd) {
    LOGR if (!m_context.gps.ublox_config->signal_ok) return -1;
    int16_t cursor_y = 0, len;
    char tmp[24] = {0}, *p = tmp;
    uint8_t speed_info_row_height = DISPLAY_HEIGHT / 3;
    uint8_t speed_data_row_height = speed_info_row_height * 2;  // 2/3
    if (screen._height > 128)
        speed_data_row_height -= INFO_BAR_HEIGHT;
    font_type_t *f = 0, *g = 0;
    int16_t cursor_x;
    float gpssspd = m_context.gps.gps_speed * calspd;

    // change color when 2s speed is in top5
    /* if (m_context.gps.S2.s_max_speed * calspd > m_context.gps.S2.avg_speed[5] * calspd) {
        DISPLAY_FILL_SCREEN(FG_COLOR);
        DISPLAY_SET_TEXT_COLOR(BG_COLOR);
    } else {
        DISPLAY_FILL_SCREEN(BG_COLOR);
        DISPLAY_SET_TEXT_COLOR(FG_COLOR);
    } */
    xltoa((int)gpssspd, p);
    if (*p == 0) {
        *p = '0';
        *(p + 1) = 0;
    }
    uint8_t font_size = m_context.config->speed_large_font;
    if (!display_priv.state.speed_1_fonts.have_i_font) {
        display_priv.state.speed_2_fonts.maxh = speed_data_row_height;
        display_priv.state.speed_1_fonts.num_fonts_in_bank = 6;
        display_priv.state.speed_1_fonts.maxh = speed_data_row_height;
        // display_priv.state.speed_1_fonts.ipart = "00";
        // display_priv.state.speed_1_fonts.fpart = "0";
        display_priv.state.speed_1_fonts.ilen = 2;  // or 3 when goes over 100kmh ?
        display_priv.state.speed_1_fonts.flen = 1;
        if (font_size == 2) {
            // f = get_font(&FreeSansBold75pt7b);
            display_priv.state.speed_1_fonts.fontbank[0] = &SansSerif_bold_84_nr;
            display_priv.state.speed_1_fonts.fontbank[1] = 0;
            display_priv.state.speed_1_fonts.fontbank[2] = &FreeSansBold60pt7b;
            display_priv.state.speed_1_fonts.fontbank[3] = &SansSerif_bold_46_nr;
            display_priv.state.speed_1_fonts.fontbank[4] = &FreeSansBold30pt7b;
            display_priv.state.speed_1_fonts.fontbank[5] = &FreeSansBold24pt7b;
        } else {
            display_priv.state.speed_1_fonts.fontbank[3] = 0;
            display_priv.state.speed_1_fonts.use_i_font_for_f = 1;
            if ((font_size == 0) && m_context.config->speed_field <= 4) {
                display_priv.state.speed_1_fonts.fontbank[0] = &SansSerif_bold_84_nr;
                display_priv.state.speed_1_fonts.fontbank[1] = 0;
                display_priv.state.speed_1_fonts.fontbank[2] = &FreeSansBold60pt7b;
                display_priv.state.speed_1_fonts.fontbank[3] = &SansSerif_bold_46_nr;
                display_priv.state.speed_1_fonts.fontbank[4] = &FreeSansBold30pt7b;
                display_priv.state.speed_1_fonts.fontbank[5] = &FreeSansBold24pt7b;
            } else {  // test for bigger font alfa
                display_priv.state.speed_1_fonts.fontbank[0] = &SansSerif_bold_96_nr;
                display_priv.state.speed_1_fonts.fontbank[1] = &SansSerif_bold_84_nr;
                display_priv.state.speed_1_fonts.fontbank[2] = 0;
                display_priv.state.speed_1_fonts.fontbank[3] = &FreeSansBold60pt7b;
                display_priv.state.speed_1_fonts.fontbank[4] = &SansSerif_bold_46_nr;
                display_priv.state.speed_1_fonts.fontbank[5] = &FreeSansBold30pt7b;
            }
        }

        pass_font(&display_priv.state.speed_1_fonts, 2);
        assert(display_priv.state.speed_1_fonts.xpos >= 0);
    }
    f = display_priv.state.speed_1_fonts.ifont;
    g = display_priv.state.speed_1_fonts.ffont;
    len = gpssspd < 10 ? 1 : gpssspd < 100 ? 2
                                           : 3;
    cursor_x = (screen._width - (f->w * len + g->w + g->w)) / 2;
    if (cursor_x < 0)
        cursor_x = 1;
    cursor_y = DISPLAY_TOP_PAD + VCENTER_V(DISPLAY_HEIGHT - speed_info_row_height - (screen._height > 128 ? INFO_BAR_HEIGHT : 0), f->h + ROW_SPACING) + speed_info_row_height + ROW_SPACING / 2 - (screen._height > 128 ? 8 : 0);

    ESP_LOGD(TAG, "[%s] ifont:%s ffont: %s ipartw: %" PRIu16 " iparth: %" PRIu16 " startx: %" PRIu16 " conffsize:%" PRIu8, __FUNCTION__, f->name, g->name, f->w, f->h, cursor_x, font_size);
    DISPLAY_SET_FONT(f->font);
    DISPLAY_SET_CURSOR(cursor_x, cursor_y);
    DISPLAY_PRINT(p);  // 0

    // if (config->speed_large_font == 2) {  // test for bigger font speed
    if (gpssspd < 100) {
        assert(g);
        DISPLAY_SET_FONT(g->font);
        DISPLAY_PRINT(".");
        xltoa((int)((gpssspd - (int)gpssspd) * 10), p);
        if (*p == 0) {
            *p = '0';
            *(p + 1) = 0;
        }
        DISPLAY_PRINT(p);  // int((x-int(x))*10)
    }
    return 0;
}

// simon speed screen eq speed_large_font == 3
static esp_err_t speed_screen_2(const float calspd) {
    LOGR if (!m_context.gps.ublox_config->signal_ok) return -1;
    int16_t cursor_y = 0, len;
    char tmp[24] = {0}, *p = tmp;
    uint8_t info_row_height = DISPLAY_HEIGHT - INFO_BAR_HEIGHT / 3;
    uint8_t data_row_height = info_row_height * 2;
    font_type_t *f = 0, *g = 0;
    int16_t cursor_x;

    SET_FONT(f, &FreeSansBold9pt7b);
    cursor_x = screen._width - f->w * 10 + 5;
    cursor_y = DISPLAY_TOP_PAD + f->h;
    DISPLAY_SET_CURSOR(cursor_x, cursor_y);
    DISPLAY_PRINT("Run ");
    f1_to_char(m_context.gps.S10.s_max_speed * calspd, p);
    DISPLAY_PRINT(p);  // 1
    DISPLAY_SET_CURSOR(cursor_x, (cursor_y += f->h + ROW_SPACING));
    DISPLAY_PRINT("2S ");
    f1_to_char(m_context.gps.S2.s_max_speed * calspd, p);
    DISPLAY_PRINT(p);  // 1
    info_row_height = cursor_y;

    float gpssspd = m_context.gps.gps_speed * calspd;
    xltoa((int)gpssspd, p);
    if (*p == 0) {
        *p = '0';
        *(p + 1) = 0;
    }
    if (!display_priv.state.speed_2_fonts.have_i_font) {
        display_priv.state.speed_2_fonts.maxh = data_row_height;
        display_priv.state.speed_2_fonts.num_fonts_in_bank = 6;
        // display_priv.state.speed_2_fonts.ipart = "00";
        // display_priv.state.speed_2_fonts.fpart = "0";
        display_priv.state.speed_2_fonts.ilen = 2;  // or 3 when goes over 100kmh ?
        display_priv.state.speed_2_fonts.flen = 1;
        display_priv.state.speed_2_fonts.fontbank[0] = &FreeSansBold75pt7b;
        display_priv.state.speed_2_fonts.fontbank[1] = &FreeSansBold60pt7b;
        display_priv.state.speed_2_fonts.fontbank[2] = &SansSerif_bold_46_nr;
        display_priv.state.speed_2_fonts.fontbank[3] = &FreeSansBold30pt7b;
        display_priv.state.speed_2_fonts.fontbank[4] = 0;
        display_priv.state.speed_2_fonts.fontbank[5] = &FreeSansBold24pt7b;
        pass_font(&display_priv.state.speed_2_fonts, 2);  // one size smaller decimal font
        // assert(cursor_x = display_priv.state.speed_2_fonts.xpos >= 0);
    }

    f = display_priv.state.speed_2_fonts.ifont;
    g = display_priv.state.speed_2_fonts.ffont;
    len = gpssspd < 10 ? 1 : gpssspd < 100 ? 2
                                           : 3;
    cursor_x = (screen._width - (f->w * len + g->w + g->w)) / 2;
    if (cursor_x < 0)
        cursor_x = 0;
    len = info_row_height + f->h + ROW_SPACING;
    int8_t dh = DISPLAY_HEIGHT - 2 * ROW_SPACING;
    while (len >= dh) {
        --len;
    }
    len -= (f->h + ROW_SPACING);
    cursor_y = DISPLAY_TOP_PAD + len + VCENTER_V(dh - len, f->h + ROW_SPACING / 2);
    ESP_LOGD(TAG, "[%s] ifont:%s ffont: %s ipartw: %" PRIu16 " iparth: %" PRIu16 " startx: %" PRId16 " starty:%" PRId16 " fy: %" PRIu8 " len: %" PRId16, __FUNCTION__, f->name, g->name, f->w, f->h, cursor_x, cursor_y, info_row_height, len);
    DISPLAY_SET_FONT(f->font);
    DISPLAY_SET_CURSOR(cursor_x, cursor_y);
    DISPLAY_PRINT(p);  // 0
    if (gpssspd < 100) {
        DISPLAY_SET_FONT(g->font);
        // DISPLAY_SET_CURSOR(cursor_x, cursor_y);
        DISPLAY_PRINT(".");
        xltoa((int)((gpssspd - (int)gpssspd) * 10), p);
        if (*p == 0) {
            *p = '0';
            *(p + 1) = 0;
        }
        DISPLAY_PRINT(p);  // int((x-int(x))*10)
    }
    return ESP_OK;
}

static esp_err_t show_multi_row_screen(int offset, stat_screen_t *sc_data) {
    LOGR
    // config_t *config = m_context.config;
    //  display_priv.state.update_delay = config->stat_screens_time * 1000;
    font_type_t *f = 0;
    int16_t cursor_y, row_start_pos, row_end_pos, cursor_x = 0;
    uint16_t col_width = 0;
    uint8_t first_row_height = (DISPLAY_HEIGHT - INFO_BAR_HEIGHT) / sc_data->rows + 1, row_height = first_row_height;
    uint8_t rcols = sc_data->cols;
    uint8_t first_row_cols = sc_data->cols / 10;
    uint8_t other_row_cols = sc_data->cols % 10;
    bool has_async_fields = sc_data->num_fields % 2 == 1;
    // info rows
    if (screen._height > 128) {
        SET_FONT(f, &FreeSansBold9pt7b);
    } else {
        SET_FONT(f, sc_data->rows < 3 ? &FreeSansBold9pt7b : &FreeSansBold6pt7b);
    }
    uint8_t info_height = (f->h);
    uint8_t i, j, k, l, m;
    if(!first_row_cols)
        col_width = screen._width / other_row_cols;
    for (i = 0, j = sc_data->rows; i < j; ++i) {
        row_start_pos = i * (i == 0 ? first_row_height : row_height) + DISPLAY_TOP_PAD;
        row_end_pos = (i + 1) * (i == 0 ? first_row_height : row_height) + DISPLAY_TOP_PAD;
        rcols = first_row_cols && i==0 ? first_row_cols : other_row_cols;
        if(first_row_cols)
            col_width = screen._width / rcols;
        /// vertical line
        if (rcols>1)
            screen.op->fillRect(&screen, col_width, row_start_pos, 1, (i == 0 ? first_row_height : row_height), FG_COLOR);
        // horizontal line
        screen.op->fillRect(&screen, 0, row_end_pos, screen._width, 1, FG_COLOR);
        cursor_y = row_start_pos + info_height;
        for (k = 0, l = rcols; k < l; ++k) {
            m = i + (k * sc_data->rows);
            if(has_async_fields){
                if(i>0 && k>0) --m;
            }
            DISPLAY_SET_CURSOR(offset + 2 + (k * col_width), cursor_y);
            //printf("-- field no: %"PRIu8", col %"PRIu8" of cols %"PRIu8"\n", m, k, l);
            DISPLAY_PRINT(sc_data->fields[m].field->name);
        }
    }

    // data rows
    float curdlen = 0;
    if(!first_row_cols)
        SET_FONT(f, sc_data->cols == 1 && sc_data->rows < 3 ? &FreeSansBold24pt7b : &FreeSansBold18pt7b);
    for (i = 0, j = sc_data->rows; i < j; ++i) {
        row_start_pos = i * (i == 0 ? first_row_height : row_height) + DISPLAY_TOP_PAD;
        row_end_pos = (i + 1) * (i == 0 ? first_row_height : row_height) + DISPLAY_TOP_PAD;
        rcols = first_row_cols && i==0 ? first_row_cols : other_row_cols;
        if(first_row_cols){
            col_width = screen._width / rcols;
            SET_FONT(f, rcols==1 && sc_data->rows < 3 ? &FreeSansBold24pt7b : &FreeSansBold18pt7b);
        }
        cursor_y = info_height + row_start_pos + (((i == 0 ? first_row_height : row_height) - info_height) / 2) + ((f->h) / 2) + (ROW_SPACING / 2);
        // printf("-- cursor_y_1: %"PRIu16"\n", cursor_y);
        while (cursor_y >= (row_end_pos - 2))
            --cursor_y;
        // printf("-- cursor_y_2: %"PRIu16"\n", cursor_y);
        for (k = 0, l = rcols, m = 0; k < l; ++k) {
            m = i + (k * sc_data->rows);
            if(has_async_fields){
                if(i>0 && k>0) --m;
            }
            //printf("-- field no: %"PRIu8", col %"PRIu8" of cols %"PRIu8"\n", m, k, l);
            if (has_async_fields || sc_data->fields[m].dlen != curdlen) {
                curdlen = sc_data->fields[m].dlen;
                HCENTER_AT(col_width, f, curdlen);
            }
            DISPLAY_SET_CURSOR(cursor_x + (k * col_width), cursor_y);
            DISPLAY_PRINT(sc_data->fields[m].data);  // average 5*10s
        }
    }
    return ESP_OK;
}

static uint32_t epaper_update_screen(struct display_s *me, screen_mode_t screen_mode, void *arg) {
    TIMER_S
    printf("[%s] screen_mode: %d\n", __FUNCTION__, screen_mode);
    uint32_t ret = 0;
    if (!me->self->displayOK)
        return 500;
    // update_time();
    const ubx_msg_t *ubxMessage = &m_context.gps.ublox_config->ubx_msg;
    const logger_config_t *config = m_context.config;
    me->self->state.update_epaper = 1;  // was zonder else
    const float calspd = m_context_rtc.RTC_calibration_speed;
    update_offset_w_count(count);
    int16_t cursor_y = 0, len;
    char tmp[24] = {0}, *p = tmp;

    // uint8_t spacing = 2;
    font_type_t *f = 0;  //, *g = 0;
    int16_t cursor_x = 0;
    const char *title = 0;
    bool is_gps_stat_screen = (screen_mode > 0 && screen_mode < 10);

    /* if ((screen_mode == SCREEN_MODE_SPEED_1 || screen_mode == SCREEN_MODE_SPEED_2) && m_context.gps.S2.s_max_speed * calspd > m_context.gps.S2.avg_speed[5] * calspd) {
        DISPLAY_FILL_SCREEN(FG_COLOR);
        DISPLAY_SET_TEXT_COLOR(BG_COLOR);
    } else { */
    DISPLAY_FILL_SCREEN(BG_COLOR);
    DISPLAY_SET_TEXT_COLOR(FG_COLOR);
    //}

    if (screen._height > 128 || (screen._height <= 128 && screen_mode != SCREEN_MODE_SPEED_1 && screen_mode != SCREEN_MODE_SPEED_2)) {
        InfoBar(offset);
    }

    char *p1;
    double s1;

    me->self->state.update_delay = is_gps_stat_screen ? (config->stat_screens_time * 1000) : 500;
    stat_screen_t *sc_data = 0;
    switch(screen_mode) {
        case SCREEN_MODE_PUSH:
    //if (screen_mode == SCREEN_MODE_PUSH) {  // button down
        me->self->state.update_delay = 100;
        uint8_t *state = (uint8_t *)arg;
        ESP_LOGD(TAG, "[%s] SCREEN_MODE_PUSH %" PRIu8, __FUNCTION__, *state);
        title = *state == BUTTON_HELD_LONG ? "Sleep" : *state == BUTTON_HELD_LLONG ? "Mode"
                                                                                   : "Push";
        len = *state == BUTTON_HELD_LONG ? 5 : 4;
        SET_FONT_CENTER(f, &FreeSansBold12pt7b, len);
        cursor_y = DISPLAY_TOP_PAD + VCENTER_V(DISPLAY_HEIGHT - INFO_BAR_HEIGHT, f->h + ROW_SPACING);
        DISPLAY_SET_CURSOR(cursor_x, cursor_y);
        DISPLAY_PRINT(title);
        break;
        case SCREEN_MODE_GPS_INIT:
    //} else if (screen_mode == SCREEN_MODE_GPS_INIT) {
        ESP_LOGD(TAG, "[%s] SCREEN_MODE_GPS_INIT", __FUNCTION__);
        me->self->state.update_delay = 500;
        if (offset > 6)
            offset = 6;
        cursor_x += offset;
        screen.op->fillRect(&screen, 0, 0, 180, 104, BG_COLOR);
        title = !ubxMessage->mon_ver.hwVersion[0] ? "Gps initializing" : ubxMessage->navPvt.numSV < 5 ? "Waiting for Sats"
                                                                                                   : "Ready for action";
        len = strlen(title);
        SET_FONT_CENTER(f, &FreeSansBold12pt7b, len);

        cursor_y = DISPLAY_TOP_PAD + VCENTER_V(DISPLAY_HEIGHT - INFO_BAR_HEIGHT, f->h + ROW_SPACING);
        DISPLAY_SET_CURSOR(cursor_x, cursor_y);
        DISPLAY_PRINT(title);
        if (!m_context.gps.ublox_config->ready && me->self->state.initialized) {
            reset_display_state(&me->self->state);
            me->self->state.initialized = 0;
        }
        SET_FONT(f, &FreeSansBold9pt7b);
        DISPLAY_SET_CURSOR(cursor_x, (cursor_y += (f->h + ROW_SPACING * 2)));
        if (!ubxMessage->mon_ver.hwVersion[0]) {
            // me->self->state.update_delay = 500;
            DISPLAY_PRINT("Gps not detected ...");
        } else if (ubxMessage->navPvt.numSV < 5) {
            DISPLAY_PRINT("Please go outside");
        } else {
            DISPLAY_PRINT("Move faster than ");
            char tmp[12] = {0}, *p = tmp;
            if ((int)(calspd * 100000) == 194) {
                p += f1_to_char(config->stat_speed * 1.94384449, p);
                memcpy(p, "kn", 2);
            }
            else if ((int)(calspd * 1000000) == 3600) {
                p += f1_to_char(config->stat_speed * 3.6, p);
                memcpy(p, "km/h", 4);
            }
            DISPLAY_PRINT(tmp);
            //  M8_M10(offset);
            Speed_in_Unit(offset);
            // vTaskDelay(100 / portTICK_PERIOD_MS);
            ret += me->self->state.update_delay == 100 ? 400 : 0;
        }
        break;
        case SCREEN_MODE_WIFI_START:
    //} else if (screen_mode == SCREEN_MODE_WIFI_START) {  // 12
        ESP_LOGD(TAG, "[%s] SCREEN_MODE_WIFI_START", __FUNCTION__);
        title = "Wifi starting";
        len = strlen(title);
        SET_FONT_CENTER(f, &FreeSansBold12pt7b, len);
        cursor_y = DISPLAY_TOP_PAD + VCENTER_V(DISPLAY_HEIGHT - INFO_BAR_HEIGHT, f->h + ROW_SPACING);
        DISPLAY_SET_CURSOR(cursor_x+ offset, cursor_y);
        DISPLAY_PRINT(title);
        if (screen_mode != old_screen)
            count = 0;  // eerste keer full update
        ret += 0;
        break;
        case SCREEN_MODE_WIFI_STATION:
    //} else if (screen_mode == SCREEN_MODE_WIFI_STATION) {  // 11
        cursor_x += offset;
        ESP_LOGD(TAG, "[%s] SCREEN_MODE_WIFI_STATION", __FUNCTION__);
        me->self->state.update_delay = 500;
        title = m_context.firmware_update_started ? "Firmware update" : wifi_context.s_ap_connection ? wifi_context.ap.ssid
                                                                                                     : wifi_context.stas[0].ssid;
        len = strlen(title);
        if (!m_context.firmware_update_started)
            len += 7;
        SET_FONT_CENTER(f, &FreeSansBold12pt7b, len);
        cursor_y = DISPLAY_TOP_PAD + VCENTER_V(DISPLAY_HEIGHT - INFO_BAR_HEIGHT, f->h + ROW_SPACING);
        DISPLAY_SET_CURSOR(cursor_x, cursor_y);
        // screen.op->fillRect(&screen, 0, ROW_3_9PT+1, 180, INFO_BAR_TOP-ROW_3_9PT-1, BG_COLOR);//clear lower part
        if (!m_context.firmware_update_started) {
            DISPLAY_PRINT("Ssid: ");
        }
        DISPLAY_PRINT(title);

        SET_FONT(f, &FreeSansBold9pt7b);
        DISPLAY_SET_CURSOR(cursor_x, (cursor_y += (f->h + ROW_SPACING * 2)));
        if (!m_context.firmware_update_started) {
            if (wifi_context.s_ap_connection == true) {
                DISPLAY_PRINT("Password: password");
                DISPLAY_SET_CURSOR(cursor_x, (cursor_y += (f->h + ROW_SPACING * 2)));
            }
            DISPLAY_PRINT("http://");
            DISPLAY_PRINT(wifi_context.ip_address);
        } else {
            DISPLAY_PRINT("in progress ...");
        }
        ret += 0;
        break;
        case SCREEN_MODE_WIFI_AP:
    //} else if (screen_mode == SCREEN_MODE_WIFI_AP) {
        ESP_LOGD(TAG, "[%s] SCREEN_MODE_WIFI_AP", __FUNCTION__);
        cursor_x += offset;
        me->self->state.update_delay = 500;
        title = m_context.firmware_update_started ? "Firmware update" : wifi_context.ap.ssid;
        len = strlen(title);
        if (!m_context.firmware_update_started)
            len += 7;
        SET_FONT_CENTER(f, &FreeSansBold12pt7b, len);
        cursor_y = DISPLAY_TOP_PAD + VCENTER_V(DISPLAY_HEIGHT - INFO_BAR_HEIGHT, f->h + ROW_SPACING);
        DISPLAY_SET_CURSOR(cursor_x, cursor_y);
        if (!m_context.firmware_update_started) {
            DISPLAY_PRINT("Ssid: ");
        }
        DISPLAY_PRINT(title);

        SET_FONT(f, &FreeSansBold9pt7b);
        DISPLAY_SET_CURSOR(cursor_x, (cursor_y += (f->h + ROW_SPACING * 2)));
        if (!m_context.firmware_update_started) {
            DISPLAY_PRINT("Password: password");
            DISPLAY_SET_CURSOR(cursor_x, (cursor_y += (f->h + ROW_SPACING * 2)));
            DISPLAY_PRINT("http://");
            DISPLAY_PRINT(wifi_context.ip_address);
        } else {
            DISPLAY_PRINT("in progress ...");
        }
        if (screen_mode != old_screen)
            count = 0;  // eerste keer full update
        ret += 100;
        break;
        case SCREEN_MODE_SPEED_1:
    // }
    // else if (screen_mode == SCREEN_MODE_SPEED_1) {  /// 10 - default speed display
        me->self->state.update_delay = 100;
        if (speed_screen_1(calspd) < 0) {
            title = "Low GPS signal !";
            len = strlen(title);
            SET_FONT_CENTER(f, &FreeSansBold18pt7b, len);
            cursor_y = DISPLAY_TOP_PAD + VCENTER_V(DISPLAY_HEIGHT - INFO_BAR_HEIGHT, f->h + ROW_SPACING);
            DISPLAY_SET_CURSOR(cursor_x, cursor_y);
            DISPLAY_PRINT(title);
        }
        show_speed_info_bar(calspd);
        break;
        case SCREEN_MODE_SPEED_2:
    // }
    // else if (screen_mode == SCREEN_MODE_SPEED_2) {  /// 14 - simon speed display
        ESP_LOGD(TAG, "[%s] SCREEN_MODE_SPEED_2", __FUNCTION__);
        me->self->state.update_delay = 100;
        if (!speed_screen_2(calspd))
            goto done;
        else {
            title = "Low GPS signal !";
            len = strlen(title);
            SET_FONT_CENTER(f, &FreeSansBold18pt7b, len);

            cursor_y = DISPLAY_TOP_PAD + VCENTER_V(DISPLAY_HEIGHT - INFO_BAR_HEIGHT, f->h + ROW_SPACING);
            DISPLAY_SET_CURSOR(cursor_x, cursor_y);
            DISPLAY_PRINT(title);
        }
        break;
        case SCREEN_MODE_GPS_TROUBLE:
    // } else if (screen_mode == SCREEN_MODE_GPS_TROUBLE) {
        ESP_LOGD(TAG, "[%s] SCREEN_MODE_TROUBLE", __FUNCTION__);
        title = "No GPS frames for more then 1s....";
        len = strlen(title);
        SET_FONT_CENTER(f, &FreeSansBold12pt7b, len);

        cursor_y = DISPLAY_TOP_PAD + VCENTER_V(DISPLAY_HEIGHT - INFO_BAR_HEIGHT, f->h + ROW_SPACING);
        DISPLAY_SET_CURSOR(cursor_x + offset, cursor_y);
        DISPLAY_PRINT(title);
        // ret += 250;
        break;
    //    default:
    //    break;
    //}
    //} else 
    //if (is_gps_stat_screen) {
    //    switch(screen_mode) {
        case SCREEN_MODE_SPEED_STATS_1:
        //if (screen_mode == SCREEN_MODE_SPEED_STATS_1) {  /// 10s
            ESP_LOGD(TAG, "[%s] SCREEN_MODE_STATS_1 10s", __FUNCTION__);
            // | --.-- | --.-- |
            // |---------------|
            // | --.-- | --.-- |
            sc_data = &sc_screens[0];
            /* sc_data.cols = 2;
            sc_data.rows = 2;
            sc_data.num_fields = 4;
            sc_data.fields[0].field = &avail_fields[0];
            sc_data.fields[1].field = &avail_fields[1];
            sc_data.fields[2].field = &avail_fields[2];
            sc_data.fields[3].field = &avail_fields[3]; */
            break;
        case SCREEN_MODE_SPEED_STATS_2:
        //} else if (screen_mode == SCREEN_MODE_SPEED_STATS_2) {  /// 2s
            ESP_LOGD(TAG, "[%s] SCREEN_MODE_STATS_2 2s", __FUNCTION__);
            // | --.-- | --.-- |
            // |---------------|
            // | --.-- | --.-- |
            sc_data = &sc_screens[1];
            /* sc_data.cols = 2;
            sc_data.rows = 2;
            sc_data.num_fields = 4;
            sc_data.fields[0].field = &avail_fields[4];
            sc_data.fields[1].field = &avail_fields[5];
            sc_data.fields[2].field = &avail_fields[6];
            sc_data.fields[3].field = &avail_fields[7]; */
            /* s1 = (j == 1 ? m_context.gps.S10.display_last_run : j == 0 ? m_context.gps.S10.display_max_speed
                                                                   : m_context.gps.S10.avg_5runs) *
                 calspd; */
            break;
        case SCREEN_MODE_SPEED_STATS_3:
        //} else if (screen_mode == SCREEN_MODE_SPEED_STATS_3) {  /// 500m
            ESP_LOGD(TAG, "[%s] SCREEN_MODE_STATS_3 500m", __FUNCTION__);
            // | --.-- | --.-- |
            // |---------------|
            // | --.-- | --.-- |
            sc_data = &sc_screens[2];
            /* sc_data.cols = 2;
            sc_data.rows = 2;
            sc_data.num_fields = 4;
            sc_data.fields[0].field = &avail_fields[8];
            sc_data.fields[1].field = &avail_fields[9];
            sc_data.fields[2].field = &avail_fields[10];
            sc_data.fields[3].field = &avail_fields[11]; */
            break;
        case SCREEN_MODE_SPEED_STATS_4:
        // } else if (screen_mode == SCREEN_MODE_SPEED_STATS_4) {  // 10s,AVG,5 runs, update on the fly !!!
            ESP_LOGD(TAG, "[%s] SCREEN_MODE_STATS_4 avg", __FUNCTION__);
            // | --.-- | --.-- |
            // |---------------|
            // | --.-- | --.-- |
            // |---------------|
            // | --.-- | --.-- |
            sc_data = &sc_screens[3];
            /* sc_data.cols = 2;
            sc_data.rows = 3;
            sc_data.num_fields = 6;
            sc_data.fields[0].field = &avail_fields[2];
            sc_data.fields[1].field = &avail_fields[16];
            sc_data.fields[2].field = &avail_fields[17];
            sc_data.fields[3].field = &avail_fields[18];
            sc_data.fields[3].field = &avail_fields[19];
            sc_data.fields[3].field = &avail_fields[20]; */
            break;
        case SCREEN_MODE_SPEED_STATS_5:
        // } else if (screen_mode == SCREEN_MODE_SPEED_STATS_5) {  // alfa statistics
            ESP_LOGD(TAG, "[%s] SCREEN_MODE_STATS_5", __FUNCTION__);
            // | --.-- | --.-- |
            // |---------------|
            // | --.-- | --.-- |
            // |---------------|
            // | --.-- | --.-- |
            sc_data = &sc_screens[4];
            /* sc_data.cols = 2;
            sc_data.rows = 3;
            sc_data.num_fields = 6;
            sc_data.fields[0].field = &avail_fields[30];
            sc_data.fields[1].field = &avail_fields[13];
            sc_data.fields[2].field = &avail_fields[22];
            sc_data.fields[3].field = &avail_fields[26];
            sc_data.fields[3].field = &avail_fields[38];
            sc_data.fields[3].field = &avail_fields[41]; */

            /* s1 = j < 5 ? ((j == 0 ? m_context.gps.M100.display_max_speed : j == 1 ? m_context.gps.M250.display_max_speed
                                                                   : j == 2   ? m_context.gps.M1852.display_max_speed
                                                                   : j == 3   ? m_context.gps.A500.alfa_speed_max
                                                                              : m_context.gps.S3600.display_max_speed) *
                          calspd)
                       : m_context.gps.Ublox.total_distance / 1000000; */

            break;
        case SCREEN_MODE_SPEED_STATS_6:
        // } else if (screen_mode == SCREEN_MODE_SPEED_STATS_6) {  // Simon stat screen
            ESP_LOGD(TAG, "[%s] SCREEN_MODE_STATS_6", __FUNCTION__);
            // |     --.--     |
            // |---------------|
            // | --.-- | --.-- |
            sc_data = &sc_screens[5];
            /* sc_data.cols = 12;
            sc_data.rows = 2;
            sc_data.num_fields = 3;
            sc_data.fields[0].field = &avail_fields[0];
            sc_data.fields[1].field = &avail_fields[1];
            sc_data.fields[2].field = &avail_fields[2]; */
            break;
        case SCREEN_MODE_SPEED_STATS_7:
        // } else if (screen_mode == SCREEN_MODE_SPEED_STATS_7) {
            ESP_LOGD(TAG, "[%s] SCREEN_MODE_STATS_7", __FUNCTION__);
            // |     --.--     |
            // |---------------|
            // |     --.--     |
            sc_data = &sc_screens[6];
            break;
        default:
        //} else {
            me->self->state.update_delay = 1;
            break;
    }
    if (is_gps_stat_screen) {
        if (sc_data->num_fields) {
            for (int j = 0; j < sc_data->num_fields; ++j) {
                p1 = sc_data->fields[j].data;
                if (sc_data->fields[j].field->type == 0) {
                    s1 = sc_data->fields[j].field->value.num() * calspd;
                    if (s1 > 100 && screen._width < 250) {
                        f1_to_char(s1, p1);
                    } else {
                        f2_to_char(s1, p1);
                    }
                    sc_data->fields[j].dlen = s1 < 10 ? 4 : s1 < 100 || screen._width < 250 ? 5 : 6;
                } else if (sc_data->fields[j].field->type == 1) {
                    s1 = 0;
                    sc_data->fields[j].dlen = sc_data->fields[j].field->value.timestr(p1);
                }
            }
            show_multi_row_screen(offset, sc_data);
        }        
        /* else if (screen_mode == SCREEN_MODE_SPEED_STATS_7) {  // Simon bar graph screen
           me->self->state.update_delay = 1;
       } else if (screen_mode == SCREEN_MODE_SPEED_STATS_8) {  // 8
           // me->self->state.update_delay = show_data_screen(offset, "10sec", &m_context.gps.S10);
           me->self->state.update_delay = 1;
       } else if (screen_mode == SCREEN_MODE_SPEED_STATS_9) {  // 9
           me->self->state.update_delay = 1;
       } */
    }

done:
    // if (IS_STAT_SCREEN(screen)) {
    DISPLAY_SET_FONT(&FreeSansBold6pt7b);
    DISPLAY_SET_CURSOR(screen._width - 16, INFO_BAR_TOP - 2);
    xltoa(screen_mode, p);
    DISPLAY_PRINT(p);
    //}
    if (count % 200 == 0) {  // was 200
        if (me->self->state.update_epaper > 0) {
            epd_display(&screen, true);
        }
        offset = 0;
    } else {
        if (me->self->state.update_epaper == 2) {
            DISPLAY_FILL_SCREEN(BG_COLOR);
        }  // test
        if (me->self->state.update_epaper > 0) {
            epd_displayWindow(&screen, 0, 0, screen._width, screen._width > screen._height && screen._height == 128 ? 122 : screen._height);  // was 244,122, true !!!
        }
        // vTaskDelay(me->self->state.update_delay / portTICK_PERIOD_MS);  // update delay function of screen to show
    }
    ret += me->self->state.update_delay;
    old_screen = screen_mode;
    count++;
    TIMER_E
    return ret;
}

void display_uninit(struct display_s *me) {
    LOGR
    epd_powerDown(&screen);
    epdg_deinit(&screen);
#ifdef CONFIG_HAS_SCREEN_DEPG0213BN
    depg0213bn_deinit(&screen_driver);           
#endif
#ifdef CONFIG_HAS_SCREEN_GDEY0154D67
    gdey0154d67_deinit(&screen_driver);
#endif
#ifdef CONFIG_HAS_SCREEN_GDEY0213B74
    gdey0213b74_deinit(&screen_driver);
#endif

    memset(me->self, 0, sizeof(struct display_priv_s));
    memset(me, 0, sizeof(struct display_s));
}

display_op_t screen_ops = {
    .boot_screen = epaper_boot_screen,
    .off_screen = epaper_off_screen,
    .sleep_screen = epaper_sleep_screen,
    .update_screen = epaper_update_screen,
    .uninit = display_uninit};

struct display_s *display_init(struct display_s *me) {
    TIMER_S
    me->op = &screen_ops;
    me->self = &display_priv;
    reset_display_state(&display_priv.state);

    //assert(screen_driver >= 0);

#ifdef DEBUG
    epdg_init(&screen, 1);
#else
    epdg_init(&screen, 0);
#endif

    //epdg_init_display(&screen, screen_driver);
#ifdef CONFIG_HAS_SCREEN_DEPG0213BN
    depg0213bn_init(&screen_driver, &screen, screen._diag_enabled);
#endif
#ifdef CONFIG_HAS_SCREEN_GDEY0154D67
    gdey0154d67_init(&screen_driver, &screen, screen._diag_enabled);
#endif
#ifdef CONFIG_HAS_SCREEN_GDEY0213B74
    gdey0213b74_init(&screen_driver, &screen, screen._diag_enabled);
#endif
    epd_fillScreen(&screen, EPD_WHITE);
    // epd_drawBitmap(&screen,screen.WIDTH/2-24,screen.HEIGHT/2-24,ESP_GPS_logo,48,48,EPD_BLACK, bm_default);
    // epd_display(&screen);
    // screen.op->setRotation(&screen, s);
    display_priv.displayHeight = screen._height;
    display_priv.displayWidth = screen._width;

    init_fonts(fonts, sizeof(fonts) / sizeof(font_type_t));

    if (screen._height) {
        display_priv.displayOK = true;
    }
    TIMER_E
    return me;
}
