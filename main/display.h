#ifndef AE577623_35A6_43A5_9250_F68AA337A0DF
#define AE577623_35A6_43A5_9250_F68AA337A0DF

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ICON_G_CHECK_CIRCLE 0xe86c
#define ICON_G_RADIO_UNCHECKED 0xe836
#define ICON_G_RADIO_CHECKED 0xe837
#define ICON_G_RADIO_PARTIAL 0xf560
#define ICON_G_WIFI 0xe63e
#define ICON_G_GPS 0xe569
#define ICON_G_SDCARD 0xe623
#define ICON_G_BATTERY_HORIZ_0 0xf8ae
#define ICON_G_BATTERY_CHARGE 0xf7eb
#define ICON_G_THERMOMETER 0xe846

typedef enum {
    SCREEN_MODE_UNKNOWN = -1,
    SCREEN_MODE_BOOT = 0,
    SCREEN_MODE_SPEED_STATS_1 = 1,
    SCREEN_MODE_SPEED_STATS_2 = 2,
    SCREEN_MODE_SPEED_STATS_3 = 3,
    SCREEN_MODE_SPEED_STATS_4 = 4,
    SCREEN_MODE_SPEED_STATS_5 = 5,
    SCREEN_MODE_SPEED_STATS_6 = 6,
    SCREEN_MODE_SPEED_STATS_7 = 7,
    SCREEN_MODE_SPEED_STATS_8 = 8,
    SCREEN_MODE_SPEED_STATS_9 = 9,
    SCREEN_MODE_GPS_TROUBLE = 10,
    SCREEN_MODE_GPS_INIT = 11,
    SCREEN_MODE_GPS_READY = 12,
    SCREEN_MODE_WIFI_START = 13,
    SCREEN_MODE_WIFI_AP = 14,
    SCREEN_MODE_WIFI_STATION = 15,
    SCREEN_MODE_PUSH = 16,
    SCREEN_MODE_SHUT_DOWN = 17,
    SCREEN_MODE_SLEEP = 18,
    SCREEN_MODE_RECORD = 19,
    SCREEN_MODE_SD_TROUBLE = 20,
    SCREEN_MODE_SPEED_1 = 'a',
    SCREEN_MODE_SPEED_2 = 'b',
} screen_mode_t;

#define IS_STAT_SCREEN(screen) (screen >= SCREEN_MODE_SPEED_STATS_1 && screen <= SCREEN_MODE_SPEED_STATS_9)
#define MINIMUM_VOLTAGE 3.2

struct display_priv_s;
typedef struct display_priv_s display_priv_t;
struct display_s;
struct lcd_g_s;

typedef struct display_op_s {
    uint32_t (*boot_screen)(const struct display_s *me);
    uint32_t (*off_screen)(const struct display_s *me, int choice);
    uint32_t (*sleep_screen)(const struct display_s *me, int choice);
    uint32_t (*update_screen)(const struct display_s *me, const screen_mode_t screen_mode, void *arg);
    void (*uninit)(struct display_s *me);
} display_op_t;

typedef struct display_s {
    display_op_t *op;
    display_priv_t *self;
#if !defined(CONFIG_HAS_SCREEN_GDEY0154D67) && !defined(CONFIG_HAS_BOARD_LILYGO_T_DISPLAY_S3)
#if defined(CONFIG_HAS_BOARD_LILYGO_EPAPER_T5)
    struct epd_g_s *screen;
#else
    struct lcd_g_s *screen;
#endif
#endif
} display_t;

struct display_s *display_init(struct display_s *me);
void display_uninit(struct display_s *me);
void lcd_timer_task(void *args);
uint32_t lcd_lv_timer_handler();

#ifdef __cplusplus
}
#endif

#endif /* AE577623_35A6_43A5_9250_F68AA337A0DF */
