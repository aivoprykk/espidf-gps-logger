#ifndef FAF00DDB_D330_462A_8A1C_425EBCE204D4
#define FAF00DDB_D330_462A_8A1C_425EBCE204D4

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "sdkconfig.h"

#include <logger_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MINIMUM_VOLTAGE 3.25
#define LOW_BAT_COUNT 20

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

#define APP_MODE_LIST(l) l(APP_MODE_UNKNOWN) l(APP_MODE_BOOT) l(APP_MODE_WIFI) l(APP_MODE_GPS) l(APP_MODE_SLEEP) l(APP_MODE_SHUT_DOWN) l(APP_MODE_RESTART)
#define CUR_SCREEN_LIST(l) l(CUR_SCREEN_NONE) l(CUR_SCREEN_GPS_STATS) l(CUR_SCREEN_GPS_SPEED) l(CUR_SCREEN_GPS_INFO) l(CUR_SCREEN_GPS_TROUBLE) l(CUR_SCREEN_SAVE_SESSION) l(CUR_SCREEN_WIFI) l(CUR_SCREEN_OFF_SCREEN) l(CUR_SCREEN_SLEEP_SCREEN) l(CUR_SCREEN_LOW_BAT) l(CUR_SCREEN_SETTINGS) l(CUR_SCREEN_BOOT) l(CUR_SCREEN_FW_UPDATE) l(CUR_SCREEN_SD_TROUBLE)
#define CFG_GROUP_LIST(l) l(CFG_GROUP_GPS, 0x00) l(CFG_GROUP_STAT_SCREENS, 0x01) l(CFG_GROUP_SCREEN, 0x02) l(CFG_GROUP_FW, 0x03)
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

//#define STATIC_DEBUG        //indien gps test zonder snelheid en met wifi actief
//#define DLS                  //set date on march 26 1:55, to test daylightsaving
#if defined(CONFIG_LOGGER_BUTTON_GPIO_1)
#define GPIO12_ACTIF        //if GPIO12 is used as wake up, standard GPIO12 function is not activated !!
#endif
#define WAKE_UP_GPIO CONFIG_LOGGER_BUTTON_GPIO_0   //default 39
#define EPOCH_2022 1640995200UL //start of the year 2022 1640995200

uint32_t screen_cb(void* arg);

#if (CONFIG_LOGGER_COMMON_LOG_LEVEL <= 2)

#include "esp_timer.h"
#include "esp_log.h"

#endif

#if defined(CONFIG_LOGGER_COMMON_LOG_LEVEL_TRACE) // "A lot of logs to give detailed information"

#define DLOG LOG_INFO
#define DMEAS_START MEAS_START
#define DMEAS_END MEAS_END
#define ILOG LOG_INFO
#define IMEAS_START MEAS_START
#define IMEAS_END MEAS_END
#define WLOG LOG_INFO
#define WMEAS_START MEAS_START
#define WMEAS_END MEAS_END

#elif defined(CONFIG_LOGGER_COMMON_LOG_LEVEL_INFO) // "Log important events"

#define DLOG(a, b, ...) ((void)0)
#define DMEAS_START() ((void)0)
#define DMEAS_END(a, b, ...) ((void)0)
#define ILOG LOG_INFO
#define IMEAS_START MEAS_START
#define IMEAS_END MEAS_END
#define WLOG LOG_INFO
#define WMEAS_START MEAS_START
#define WMEAS_END MEAS_END

#elif defined(CONFIG_LOGGER_COMMON_LOG_LEVEL_WARN) // "Log if something unwanted happened but didn't cause a problem"

#define DLOG(a, b, ...) ((void)0)
#define DMEAS_START() ((void)0)
#define DMEAS_END(a, b, ...) ((void)0)
#define ILOG(a, b, ...) ((void)0)
#define IMEAS_START() ((void)0)
#define IMEAS_END(a, b, ...) ((void)0)
#define WLOG LOG_INFO
#define WMEAS_START MEAS_START
#define WMEAS_END MEAS_END

#else // "Do not log anything"

#define DLOG(a, b, ...) ((void)0)
#define DMEAS_START() ((void)0)
#define DMEAS_END(a, b, ...) ((void)0)
#define ILOG(a, b, ...) ((void)0)
#define IMEAS_START() ((void)0)
#define IMEAS_END(a, b, ...) ((void)0)
#define WLOG(a, b, ...) ((void)0)
#define WMEAS_START() ((void)0)
#define WMEAS_END(a, b, ...) ((void)0)
#endif

struct record_forwarder_s {
    const struct screen_f_s * cur;
    const struct screen_f_s * prev;
    bool num;
};

struct push_forwarder_s {
    const int state;
    const char * title;
};

#if defined(CONFIG_DISPLAY_DRIVER_ST7789)
#define LCD_UI_TASK_STACK_SIZE 5120
#else
#define LCD_UI_TASK_STACK_SIZE 3584
#endif
#define LCD_UI_TIMER_PERIOD_S 60

uint32_t lcd_ui_screen_draw();
void lcd_ui_task(void *args);
void cancel_lcd_ui_delay();
uint32_t get_lcd_ui_count();
void lcd_ui_start_task();
void lcd_ui_task_pause();
void lcd_ui_task_resume();
uint16_t get_offscreen_counter();
bool lcd_ui_task_is_paused();
void lcd_ui_request_fast_refresh();
void lcd_ui_request_full_refresh(bool force);
void lcd_ui_task_resume_for_times(uint8_t times, int8_t fast_refresh_time, int8_t full_refresh_time, bool full_refresh_force);
void lcd_ui_task_cancel_req_full_refresh();
void lcd_ui_task_req_full_refresh(int8_t full_refresh_time, bool full_refresh_force);
void lcd_ui_task_req_fast_refresh(int8_t full_refresh_time);
void lcd_ui_task_cancel_req_fast_refresh();
void wifi_sta_conf_sync();
static esp_err_t events_uninit();
void wait_for_ui_task();

// void lcd_ui_request_fast_refresh(bool force);

#ifdef __cplusplus
}
#endif

#endif /* FAF00DDB_D330_462A_8A1C_425EBCE204D4 */
