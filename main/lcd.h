#ifndef AE577623_35A6_43A5_9250_F68AA337A0DF
#define AE577623_35A6_43A5_9250_F68AA337A0DF

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "logger_common.h"

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

struct display_priv_s;
typedef struct display_priv_s display_priv_t;
struct display_s;

typedef struct display_op_s {
    uint32_t (*sleep_screen)(const struct display_s *me, int choice);
    uint32_t (*update_screen)(const struct display_s *me, const screen_mode_t screen_mode, void *arg);
    void (*uninit)(struct display_s *me);
} display_op_t;

typedef struct display_s {
    display_op_t *op;
    display_priv_t *self;
} display_t;

struct display_s *display_init(struct display_s *me);
void display_uninit(struct display_s *me);
void lcd_timer_task(void *args);
uint32_t lcd_lv_timer_handler();

#ifdef __cplusplus
}
#endif

#endif /* AE577623_35A6_43A5_9250_F68AA337A0DF */
