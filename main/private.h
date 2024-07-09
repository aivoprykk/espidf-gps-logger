#ifndef FAF00DDB_D330_462A_8A1C_425EBCE204D4
#define FAF00DDB_D330_462A_8A1C_425EBCE204D4

#include <stdio.h>

#include "sdkconfig.h"

#include <logger_common.h>

#ifdef __cplusplus
extern "C" {
#endif

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

typedef enum {
    APP_MODE_UNKNOWN = 0,
    APP_MODE_BOOT,
    APP_MODE_WIFI,
    APP_MODE_GPS,
    APP_MODE_SLEEP,
    APP_MODE_SHUT_DOWN,
    APP_MODE_RESTART
} app_mode_t;

typedef enum {
    CUR_SCREEN_NONE,
    CUR_SCREEN_GPS_STATS,
    CUR_SCREEN_GPS_SPEED,
    CUR_SCREEN_GPS_INFO,
    CUR_SCREEN_GPS_TROUBLE,
    CUR_SCREEN_SAVE_SESSION,
    CUR_SCREEN_WIFI_INFO,
    CUR_SCREEN_OFF_SCREEN,
    CUR_SCREEN_SLEEP_SCREEN,
    CUR_SCREEN_LOW_BAT,
} cur_screens_t;

#define INCLUDE_pcTaskGetTaskName 1

//#define STATIC_DEBUG        //indien gps test zonder snelheid en met wifi actief
//#define DLS                  //set date on march 26 1:55, to test daylightsaving
#if defined(CONFIG_BTN_GPIO_INPUT_1_ACTIVE)
#define GPIO12_ACTIF        //if GPIO12 is used as wake up, standard GPIO12 function is not activated !!
#endif
#define WAKE_UP_GPIO CONFIG_BTN_GPIO_INPUT_0   //default 39
//#define GPIO_NUM_xx CONFIG_BTN_GPIO_INPUT_0   //default GPIO_NUM_39 type is no int
//#define BUTTON_PIN CONFIG_BTN_GPIO_INPUT_0
#define EPOCH_2022 1640995200UL //start of the year 2022 1640995200
uint32_t screen_cb(void* arg);

#if (DEBUG == 1) // "A lot of logs to give detailed information"

#ifndef DLOG
#define DLOG(a, b, ...) ESP_LOGI(a, b, __VA_ARGS__)
#endif
#ifndef DMEAS_START
#define DMEAS_START() uint64_t _start = (esp_timer_get_time()), _end = 0
#endif
#ifndef DMEAS_END
#define DMEAS_END(a, b, ...) \
    _end = (esp_timer_get_time());  \
    ESP_LOGI(a, b, __VA_ARGS__, _end - _start)
#endif
#ifndef ILOG
#define ILOG DLOG
#endif
#ifndef IMEAS_START
#define IMEAS_START DMEAS_START
#endif
#ifndef IMEAS_END
#define IMEAS_END DMEAS_END
#endif
#ifndef WLOG
#define WLOG DLOG
#endif
#ifndef WMEAS_START
#define WMEAS_START DMEAS_START
#endif
#ifndef WMEAS_END
#define WMEAS_END DMEAS_END
#endif

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

#ifdef __cplusplus
}
#endif

#endif /* FAF00DDB_D330_462A_8A1C_425EBCE204D4 */
