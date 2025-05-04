#ifndef FAF00DDB_D330_462A_8A1C_425EBCE204D4
#define FAF00DDB_D330_462A_8A1C_425EBCE204D4

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "sdkconfig.h"
#if (defined(CONFIG_LOGGER_USE_GLOBAL_LOG_LEVEL) && CONFIG_LOGGER_GLOBAL_LOG_LEVEL < CONFIG_LOGGER_COMMON_LOG_LEVEL)
#define C_LOG_LEVEL CONFIG_LOGGER_GLOBAL_LOG_LEVEL
#else
#define C_LOG_LEVEL CONFIG_LOGGER_COMMON_LOG_LEVEL
#endif
#include "common_log.h"

#include "logger_common.h"
#ifdef CONFIG_DISPLAY_ENABLED
#include "display.h"
#endif

#if (C_LOG_LEVEL < 2)
extern const char * const app_mode_str[];
extern const char * const cur_screen_str[];
#endif

enum speeds_e {
    BAR_RESET_IN_STILL_SECS = 120,
#if defined(CONFIG_SSD168X_PANEL_SSD1681)
    BAR_MAX = 190,
#else
    BAR_MAX = 240,
#endif
    QUATER_SEC_IN_MS = 250,
    HALF_SEC_IN_MS = 500,
    ONE_K = 1000,
    HALF_H_IN_SECS = 1800,
    ONE_H_IN_SECS = 3600
};

#define MINIMUM_VOLTAGE 3.25
#define LOW_BAT_COUNT 20
#define BAT_LOW_TRESHOLD 3.4
#define BAT_UP_TRESHOLD 4.0

#ifndef MILLIS
#define MILLIS(x) (int64_t)(x).tv_sec * 1000000L + (int64_t)(x).tv_usec;
#endif
#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif

#ifndef BUF_SIZE
#define BUF_SIZE 512
#endif

#define APP_MODE_LIST(l) \
l(APP_MODE_UNKNOWN) \
l(APP_MODE_BOOT) \
l(APP_MODE_WIFI) \
l(APP_MODE_GPS) \
l(APP_MODE_SLEEP) \
l(APP_MODE_SHUT_DOWN) \
l(APP_MODE_RESTART)

#define CUR_SCREEN_LIST(l) \
l(CUR_SCREEN_NONE) \
l(CUR_SCREEN_GPS_STATS) \
l(CUR_SCREEN_GPS_SPEED) \
l(CUR_SCREEN_GPS_INFO) \
l(CUR_SCREEN_GPS_TROUBLE) \
l(CUR_SCREEN_SAVE_SESSION) \
l(CUR_SCREEN_WIFI) \
l(CUR_SCREEN_OFF_SCREEN) \
l(CUR_SCREEN_SLEEP_SCREEN) \
l(CUR_SCREEN_LOW_BAT) \
l(CUR_SCREEN_SETTINGS) \
l(CUR_SCREEN_BOOT) \
l(CUR_SCREEN_FW_UPDATE) \
l(CUR_SCREEN_SD_TROUBLE)

#define SCREEN_MODE_LIST(l) \
    l(SCREEN_MODE_UNKNOWN, -1) \
    l(SCREEN_MODE_BOOT, 0) \
    l(SCREEN_MODE_SPEED_STATS_1, 1) \
    l(SCREEN_MODE_SPEED_STATS_2, 2) \
    l(SCREEN_MODE_SPEED_STATS_3, 3) \
    l(SCREEN_MODE_SPEED_STATS_4, 4) \
    l(SCREEN_MODE_SPEED_STATS_5, 5) \
    l(SCREEN_MODE_SPEED_STATS_6, 6) \
    l(SCREEN_MODE_SPEED_STATS_7, 7) \
    l(SCREEN_MODE_SPEED_STATS_8, 8) \
    l(SCREEN_MODE_SPEED_STATS_9, 9) \
    l(SCREEN_MODE_GPS_TROUBLE, 10) \
    l(SCREEN_MODE_GPS_INIT, 11) \
    l(SCREEN_MODE_GPS_READY, 12) \
    l(SCREEN_MODE_WIFI_START, 13) \
    l(SCREEN_MODE_WIFI_AP, 14) \
    l(SCREEN_MODE_WIFI_STATION, 15) \
    l(SCREEN_MODE_PUSH, 16) \
    l(SCREEN_MODE_SHUT_DOWN, 17) \
    l(SCREEN_MODE_SLEEP, 18) \
    l(SCREEN_MODE_RECORD, 19) \
    l(SCREEN_MODE_SD_TROUBLE, 20) \
    l(SCREEN_MODE_SETTINGS, 21) \
    l(SCREEN_MODE_LOW_BAT, 22) \
    l(SCREEN_MODE_OFF_SCREEN, 23) \
    l(SCREEN_MODE_FW_UPDATE, 24) \
    l(SCREEN_MODE_SPEED_1, 'a') \
    l(SCREEN_MODE_SPEED_2, 'b')

typedef enum {
    SCREEN_MODE_LIST(ENUM_V)
} screen_mode_t;

#define IS_STAT_SCREEN(screen) (screen >= SCREEN_MODE_SPEED_STATS_1 && screen <= SCREEN_MODE_SPEED_STATS_9)

#define CFG_GROUP_LIST(l) \
l(CFG_GROUP_GPS, 0) \
l(CFG_GROUP_STAT_SCREENS, 1) \
l(CFG_GROUP_SCREEN, 2) \
l(CFG_GROUP_FW, 3)

typedef enum {
    APP_MODE_LIST(ENUM)
} app_mode_t;

typedef enum {
    CUR_SCREEN_LIST(ENUM)
} cur_screens_t;

typedef enum {
    CFG_GROUP_LIST(ENUM_V)
} config_group_t;

typedef struct v_settings_s {
    config_group_t group;
    const char *name;
    void *settings_data;
} v_settings_t;

#define INCLUDE_pcTaskGetTaskName 1

// #define STATIC_DEBUG        /// if gps test without speed and with wifi active
// #define DLS                 /// set date on march 26 1:55, to test daylightsaving
#ifdef CONFIG_LOGGER_BUTTON_ENABLED
#define WAKE_UP_GPIO CONFIG_LOGGER_BUTTON_GPIO_0
#if defined(CONFIG_LOGGER_BUTTON_GPIO_1)
#define GPIO12_ACTIF        /// if GPIO12 is used as wake up, standard GPIO12 function is not activated !!
#endif
#endif

#ifndef WAKE_UP_GPIO
#if defined(CONFIG_HAS_BOARD_LILYGO_EPAPER_T5)
#define WAKE_UP_GPIO 39
#else
#define WAKE_UP_GPIO 14
#endif
#endif

#define EPOCH_2022 1640995200UL /// start of the year 2022 1640995200

enum record_done_e {
    RECORD_DONE_START = 0,
    RECORD_DONE_2 = 2,
    RECORD_DONE_4 = 4,
    RECORD_DONE_6 = 6,
    RECORD_DONE_8 = 8,
    RECORD_DONE_10 = 10,
    RECORD_DONE_12 = 12,
    RECORD_DONE_MARK = 25,
    RECORD_DONE_OK = 240,
    RECORD_DONE_END = 255
};

uint32_t screen_cb(void* arg);

struct record_forwarder_s {
    const struct screen_f_s * cur;
    const struct screen_f_s * prev;
    bool num;
};

struct push_forwarder_s {
    const int state;
    const char * title;
};

typedef struct main_ctx_s {
    struct context_s * ctx;
    struct m_wifi_context * wifi_ctx;
    struct logger_config_s * config;
#ifdef CONFIG_DISPLAY_ENABLED
    struct display_s display;
#endif
    app_mode_t app_mode;
    cur_screens_t cur_screen;
    cur_screens_t next_screen;
    uint8_t stat_screen_count;
    uint8_t fw_update_screen;
    uint8_t cfg_screen;
    uint8_t button_press_mode;
    uint8_t low_bat_countdown;
    uint8_t low_bat_count;
    uint8_t record_done;
    bool button_down;
    uint8_t gps_cfg_item;
    uint8_t stat_screen_cfg_item;
    uint8_t screen_cfg_item;
    uint8_t fw_cfg_item;
    uint8_t app_mode_gps_on;
    uint8_t app_mode_wifi_on;
    uint8_t screen_auto_refresh;
    uint8_t config_initialized;
#ifdef CONFIG_USE_SD_CARD
    uint64_t sd_space[2];
#endif
#ifdef CONFIG_USE_FATFS
    uint64_t fatfs_space[2];
#endif
} main_ctx_t;

extern struct main_ctx_s m_app_ctx;

#define LOW_BAT_TRIGGER 7

void wifi_sta_conf_sync();
static esp_err_t events_deinit();
void deinit_button();
void init_button();
struct ubx_config_s;
struct gps_context_s;
static size_t update_gps_info_row_str(const struct ubx_config_s *ubx_dev, char * p);
static size_t update_gps_desc_row_str(const struct gps_context_s * gps, char * p);

// void lcd_ui_request_fast_refresh(bool force);

#ifdef __cplusplus
}
#endif

#endif /* FAF00DDB_D330_462A_8A1C_425EBCE204D4 */
