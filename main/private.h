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

#ifdef __cplusplus
}
#endif

#endif /* FAF00DDB_D330_462A_8A1C_425EBCE204D4 */
