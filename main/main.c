#include "display.h"
#include "private.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include <math.h>
#if defined(CONFIG_LOGGER_USE_WDT)
#include "esp_task_wdt.h"
//#include "rtc_wdt.h"
#endif
#include "esp_err.h"
#include "esp_mac.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include "esp_timer.h"

#include <driver/gpio.h>
#include <driver/rtc_io.h>

#ifdef CONFIG_LOGGER_ADC_ENABLED
#include "adc.h"
#include "adc_events.h"
#if defined(CONFIG_ULP_COPROC_ENABLED)
/* Track if ULP was running before sleep to handle intermediate timer wakes.
 * When ULP wakes → timer wake → long sleep, we need to resume ULP (not restart).
 * This flag persists across the intermediate timer wake. */
// RTC_DATA_ATTR static bool ulp_was_running = false;
#endif
#endif
#ifdef CONFIG_BMX_ENABLE
#include "bmx.h"
#endif
#if defined(CONFIG_LOGGER_BUTTON_ENABLED)
#include "button_events.h"
#endif
#ifdef CONFIG_LOGGER_VFS_ENABLED
#include "vfs_events.h"
#include "vfs.h"

#endif
#if defined(CONFIG_GPS_LOG_ENABLED)
#include "gps_log.h"
#include "gps_data.h"
#include "gps_log_file.h"
#include "gps_log_events.h"
#include "gps_user_cfg.h"
#include "dstat_screens.h"
#endif
#ifdef CONFIG_LOGGER_HTTP_ENABLED
#include "http_rest_server.h"
#endif
#ifdef CONFIG_OTA_ENABLED
#include "ota.h"
#endif
#ifdef CONFIG_OTA_USE_AUTO_UPDATE
#include "https_ota.h"
#include "ota_events.h"
#endif
#ifdef CONFIG_UBLOX_ENABLED
#include "ubx.h"
#include "ubx_msg.h"
#include "ubx_events.h"
#endif
#ifdef CONFIG_LOGGER_WIFI_ENABLED
#include "logger_wifi.h"
#endif
#ifdef CONFIG_DISPLAY_ENABLED
#include "driver_vendor.h"
#include "ui_events.h"
#endif
#include "context.h"
#include "logger_config.h"
#include "lcd.h"

// events
#include "config_events.h"
#include "logger_events.h"
#include "logger_buffer_pool.h"

ESP_EVENT_DEFINE_BASE(LOGGER_EVENT);

#define TIME_TO_SLEEP  21600UL     /* Time ESP32 will go to sleep (no for 6h, 4/day) */
#if defined(CONFIG_LOGGER_USE_WDT)
#define WDT_TIMEOUT 60             // 60 seconds WDT, opgelet zoeken naar ssid time-out<dan 10s !!!
#endif

static const char *TAG = "main";

extern struct context_s m_context;
extern struct context_rtc_s m_context_rtc;

struct main_ctx_s m_app_ctx = {
#ifdef CONFIG_DISPLAY_ENABLED
.display = {0},
#endif
.config = 0,
.ctx = &m_context,
.wifi_ctx = &wifi_context,
// app_mode
// 0 - before config, still init running
// 1 - config, wifi and http running
// 2 - run, gps running
.app_mode = APP_MODE_UNKNOWN,
.cur_screen = CUR_SCREEN_NONE,
.next_screen = CUR_SCREEN_NONE,
.record_done = 25,
.button_down = false,
.button_press_mode = -1,
.gps_cfg_item = CFG_GPS_ITEM_BASE,
.stat_screen_cfg_item = 0,
.screen_cfg_item = 0,
.cfg_screen = 0,
.fw_update_screen = 0,
.fw_cfg_item = 0,
.app_mode_wifi_on = 0,
.app_mode_gps_on = 0,
.config_initialized = 0,
.screen_auto_refresh = 0,
#ifdef CONFIG_USE_SD_CARD
.sd_space = {0},
#endif
#ifdef CONFIG_USE_FATFS
.fatfs_space = {0},
#endif
};

#if (C_LOG_LEVEL <= LOG_INFO_NUM) // 2 - info
const char * const app_mode_str[] = { APP_MODE_LIST(STRINGIFY) };
const char * const cur_screen_str[] = { CUR_SCREEN_LIST(STRINGIFY) };
#else
const char * const app_mode_str[] = { "APP_MODE_EVENT" };
const char * const cur_screen_str[] = { "CUR_SCREEN_EVENT" };
#endif

#if (defined(GPSSS))
static TaskHandle_t gps_task_handle = 0;
#endif

#if defined(CONFIG_LOGGER_USE_WDT)
static int wdt_task0, wdt_task1;
#endif

static const char * const wakeup_reasons[] = {
    0, 0,
    "ESP_SLEEP_WAKEUP_EXT0",
    "ESP_SLEEP_WAKEUP_EXT1",
    "ESP_SLEEP_WAKEUP_TIMER",
    "ESP_SLEEP_WAKEUP_TOUCHPAD",
    "ESP_SLEEP_WAKEUP_ULP",
    "ESP_SLEEP_WAKEUP_OTHER",
};

#if defined(CONFIG_LOGGER_VFS_ENABLED)
static void vfs_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
#endif
#if defined(CONFIG_LOGGER_HTTP_ENABLED)
static void ota_fw_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
#endif
#if defined(CONFIG_OTA_USE_AUTO_UPDATE)
static void ota_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
#endif
static void logger_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
static void logger_cfg_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
#if defined(CONFIG_UBLOX_ENABLED)
static void ubx_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
#endif
#if defined(CONFIG_GPS_LOG_ENABLED)
static void gps_log_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
#endif
#if defined(CONFIG_LOGGER_ADC_ENABLED)
static void adc_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
#endif
#if defined(CONFIG_LOGGER_WIFI_ENABLED)
static void wifi_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
#endif
#if defined(CONFIG_DISPLAY_ENABLED)
static void ui_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
#endif

#if defined(CONFIG_LOGGER_WIFI_ENABLED)
// WiFi mode change callbacks for coordinating external dependencies
void wifi_before_mode_change_callback(void);
void wifi_after_mode_change_callback(void);
// ADC resume coordination
static void schedule_delayed_adc_resume(void);
#endif

#define MAIN_LOOP_PERIOD_MS            ((uint32_t)MS_50)
#if defined(CONFIG_ULP_COPROC_ENABLED) && defined(CONFIG_LOGGER_ADC_ENABLED)
#define BATTERY_MONITOR_PERIOD_LOOPS   (50U)
#else
#define BATTERY_MONITOR_PERIOD_LOOPS   (10U)
#endif
#define INIT_DELAY_SHORT_MS            ((uint32_t)10)
#define INIT_DELAY_MEDIUM_MS           ((uint32_t)20)

typedef struct {
    bool immediate_sleep;
    uint64_t sleep_time;
    bool enable_ext0;
    uint8_t enable_ulp;
} wakeup_plan_t;


static void service_power_requests(void);
static void ensure_app_ready(void);
static bool run_periodic_diagnostics(uint32_t loop_counter);
static void service_display(bool verbose, uint32_t now_ms);
static void cleanup(void);

#if defined CONFIG_LOGGER_ADC_ENABLED
// ADC event suppression functions are now in adc.h
#endif

static bool s_event_loop_ready = false;

// ADC event suppression is now handled by the adc module

static esp_err_t register_event_handlers(void) {
    esp_err_t first_err = ESP_OK;
#define REGISTER_EVENT(base, handler)                                                             \
    do {                                                                                          \
        esp_err_t err = esp_event_handler_register((base), ESP_EVENT_ANY_ID, (handler), NULL);    \
        if (err != ESP_OK) {                                                                      \
            WLOG(TAG, "[%s] event reg fail %s/%s: %s", __func__, #base, #handler, esp_err_to_name(err)); \
            if (first_err == ESP_OK) {                                                            \
                first_err = err;                                                                  \
            }                                                                                     \
        }                                                                                         \
    } while (0)

#if defined(CONFIG_LOGGER_VFS_ENABLED)
    REGISTER_EVENT(VFS_EVENT, vfs_event_handler);
#endif
#if defined(CONFIG_LOGGER_HTTP_ENABLED)
    REGISTER_EVENT(OTA_FW_EVENT, ota_fw_event_handler);
#if defined(CONFIG_OTA_USE_AUTO_UPDATE)
    REGISTER_EVENT(OTA_AUTO_EVENT, ota_event_handler);
#endif
#endif
    REGISTER_EVENT(LOGGER_EVENT, logger_event_handler);
    REGISTER_EVENT(LOGGER_CONFIG_EVENT, logger_cfg_event_handler);
#if defined(CONFIG_UBLOX_ENABLED)
    REGISTER_EVENT(UBX_EVENT, ubx_event_handler);
#endif
#if defined(CONFIG_GPS_LOG_ENABLED)
    REGISTER_EVENT(GPS_LOG_EVENT, gps_log_event_handler);
#endif
#if defined(CONFIG_LOGGER_ADC_ENABLED)
    REGISTER_EVENT(ADC_EVENT, adc_event_handler);
#endif
#if defined(CONFIG_LOGGER_WIFI_ENABLED)
    REGISTER_EVENT(WIFI_EVENT, wifi_event_handler);
    REGISTER_EVENT(IP_EVENT, wifi_event_handler);
#endif
#if defined(CONFIG_DISPLAY_ENABLED)
    REGISTER_EVENT(UI_EVENT, ui_event_handler);
#endif
#undef REGISTER_EVENT
    return first_err;
}

static void unregister_event_handlers(void) {
#define UNREGISTER_EVENT(base, handler)                                                            \
    do {                                                                                           \
        esp_err_t err = esp_event_handler_unregister((base), ESP_EVENT_ANY_ID, (handler));         \
        if (err != ESP_OK) {                                                                       \
            WLOG(TAG, "[%s] event unreg fail %s/%s: %s", __func__, #base, #handler, esp_err_to_name(err)); \
        }                                                                                          \
    } while (0)

#if defined(CONFIG_LOGGER_VFS_ENABLED)
    UNREGISTER_EVENT(VFS_EVENT, vfs_event_handler);
#endif
#if defined(CONFIG_LOGGER_HTTP_ENABLED)
    UNREGISTER_EVENT(OTA_FW_EVENT, ota_fw_event_handler);
#if defined(CONFIG_OTA_USE_AUTO_UPDATE)
    UNREGISTER_EVENT(OTA_AUTO_EVENT, ota_event_handler);
#endif
#endif
    UNREGISTER_EVENT(LOGGER_EVENT, logger_event_handler);
    UNREGISTER_EVENT(LOGGER_CONFIG_EVENT, logger_cfg_event_handler);
#if defined(CONFIG_UBLOX_ENABLED)
    UNREGISTER_EVENT(UBX_EVENT, ubx_event_handler);
#endif
#if defined(CONFIG_GPS_LOG_ENABLED)
    UNREGISTER_EVENT(GPS_LOG_EVENT, gps_log_event_handler);
#endif
#if defined(CONFIG_LOGGER_ADC_ENABLED)
    UNREGISTER_EVENT(ADC_EVENT, adc_event_handler);
#endif
#if defined(CONFIG_LOGGER_WIFI_ENABLED)
    UNREGISTER_EVENT(WIFI_EVENT, wifi_event_handler);
    UNREGISTER_EVENT(IP_EVENT, wifi_event_handler);
#endif
#if defined(CONFIG_DISPLAY_ENABLED)
    UNREGISTER_EVENT(UI_EVENT, ui_event_handler);
#endif
#undef UNREGISTER_EVENT
}

static void configure_sleep_wakeup_sources(uint64_t sleep_time, bool enable_ext0, bool enable_ulp) {
    FUNC_ENTRY(TAG);

    // Always disable all wakeup sources first to avoid conflicts
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    
    // Configure timer wakeup if specified
    if (sleep_time > 0) {
        esp_sleep_enable_timer_wakeup(SEC_TO_US(sleep_time));
        FUNC_ENTRY_ARGSD(TAG, "Enabled timer wakeup: %llu seconds", sleep_time);
    }
    esp_err_t ret = 0;
#if !defined(CONFIG_ULP_BUTTON_ENABLED) || (CONFIG_ULP_BUTTON_GPIO != WAKE_UP_GPIO)  || !defined(CONFIG_LOGGER_ADC_ENABLED) || !defined(CONFIG_ULP_COPROC_ENABLED)
    if (enable_ext0) {
#if C_LOG_LEVEL <= LOG_DEBUG_NUM
        uint8_t ext_num = 0;
#endif
        if (enable_ulp) {
#if defined(CONFIG_LOGGER_ADC_ENABLED) && defined(CONFIG_ULP_COPROC_ENABLED)
            ret = esp_sleep_enable_ext1_wakeup((1ULL << WAKE_UP_GPIO), 0);
#if C_LOG_LEVEL <= LOG_DEBUG_NUM
            ext_num = 1;
#endif
        } else {
#endif
            ret = esp_sleep_enable_ext0_wakeup(WAKE_UP_GPIO, 0);
        }
        if (ret == ESP_OK) {
            FUNC_ENTRY_ARGSD(TAG, "Enabled EXT%d wakeup on GPIO %d", ext_num, WAKE_UP_GPIO);
        } else {
            FUNC_ENTRY_ARGSD(TAG, "Failed to enable EXT%d wakeup: %s", ext_num, esp_err_to_name(ret));
        }
    }
#else
    FUNC_ENTRY_ARGSD(TAG, "ULP button enabled - skipping EXT wakeup for GPIO %d", WAKE_UP_GPIO);
#endif
    
    // Configure ULP wakeup (battery monitoring and button monitoring)  
#if defined(CONFIG_LOGGER_ADC_ENABLED) && defined(CONFIG_ULP_COPROC_ENABLED)
    if (enable_ulp) {
        esp_err_t err = esp_sleep_enable_ulp_wakeup();
        if (err != ESP_OK) {
            ELOG(TAG, "Failed to enable ULP wakeup: %s", esp_err_to_name(err));
        } else {
#ifdef CONFIG_ULP_BUTTON_ENABLED
            FUNC_ENTRY_ARGSD(TAG, "Enabled ULP wakeup for battery and button monitoring");
#else
            FUNC_ENTRY_ARGS(TAG, "Enabled ULP wakeup for battery monitoring");
#endif
        }
    }
#endif
    
    // Log all enabled wakeup sources for debugging
    FUNC_ENTRY_ARGS(TAG, "Sleep configured with wakeup sources: timer=%d, ext0=%d, ulp=%d", 
             sleep_time > 0, enable_ext0, enable_ulp);
}

static void low_to_sleep(uint64_t sleep_time, bool enable_ext0, uint8_t enable_ulp) {
#if CONFIG_IDF_TARGET_ESP32
#if defined(CONFIG_HAS_BOARD_LILYGO_EPAPER_T5)
    // On LilyGo T5, we need to set SD card CS pin to output and HIGH to avoid power drain during deep sleep
    int sd_cs_gpio = 13;
    gpio_set_direction(sd_cs_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(sd_cs_gpio, 1);
    gpio_deep_sleep_hold_en();
#endif
#if CONFIG_ULP_BUTTON_GPIO != 12 && WAKE_UP_GPIO != 12
    rtc_gpio_isolate(GPIO_NUM_12);
#endif
#endif

#if defined(CONFIG_LOGGER_ADC_ENABLED) && defined(CONFIG_ULP_COPROC_ENABLED)
    /* Check wake source - need to distinguish between:
     * 1. Direct ULP wake: Resume ULP with preserved history
     * 2. Timer wake after ULP wake: Resume ULP (flag persists through timer wake)
     * 3. Timer/button wake (fresh start): Start ULP with cleared history */
    esp_sleep_wakeup_cause_t prev_wake = esp_sleep_get_wakeup_cause();
    bool is_ulp_resume = (prev_wake == ESP_SLEEP_WAKEUP_ULP) || ulp_prog_is_initialized();
    
    // Enable ULP wakeup for battery monitoring if battery is low or critical
    // enable_ulp = false;
    // if (m_context_rtc.RTC_voltage_bat < (MINIMUM_VOLTAGE + 0.2f)) {
    //     ILOG(TAG, "Battery low (%.2fV), enabling ULP monitoring during sleep", 
    //              m_context_rtc.RTC_voltage_bat);
    // }
    
    /* Force enable_ulp=true if resuming from ULP wake (or timer wake after ULP wake),
     * otherwise the ULP will stop monitoring and never wake again */
    if (is_ulp_resume && !enable_ulp) {
        ILOG(TAG, "Forcing ULP enable: resuming after ULP wake (was_running=%d)", ulp_prog_is_initialized());
        enable_ulp = 1;
    }
    
    if(enable_ulp) {
        /* Choose appropriate ULP restart method based on wake source:
         * - ULP wake (or timer after ULP): Resume monitoring, preserve ADC history
         * - Other wake: Clear ADC history for fresh sleep cycle (voltage changed during wake) */
        if (is_ulp_resume) {
            ILOG(TAG, "Resuming ULP after ULP wake (preserving ADC history, was_running=%d)", ulp_prog_is_initialized());
            resume_ulp_program();    // Resume ULP without clearing history
        } else {
            ILOG(TAG, "Starting ULP for fresh sleep cycle (clearing ADC history)");
            resume_ulp_program();     // Start ULP with fresh history
        }
 #if !CONFIG_IDF_TARGET_ESP32
        /* RTC peripheral power domain needs to be kept on to keep SAR ADC related configs during sleep */
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
#endif
    } else {
        /* ULP not enabled for this sleep - clear the flag.
         * This handles button wake → user activity → sleep without ULP */
    }
#endif
    
    // Configure all wakeup sources in coordinated manner
    configure_sleep_wakeup_sources(sleep_time, enable_ext0, enable_ulp);
    
#if defined(CONFIG_LOGGER_ADC_ENABLED) && defined(CONFIG_ULP_COPROC_ENABLED)
    /* Log ULP state right before deep sleep for debugging */
    if (enable_ulp) {
        ILOG(TAG, "Entering deep sleep - ULP cycle_count=%lu, was_running=%d", 
             adc_ulp_get_cycle_count(), ulp_prog_main_cpu_is_running());
    }
    ulp_prog_set_main_cpu_running(false);
#endif
    esp_deep_sleep(TO_M_UL(sleep_time));
}

// Battery low callback - called by ADC module when low battery timer expires
static void on_low_battery_shutdown(void) {
#if (C_LOG_LEVEL <= LOG_INFO_NUM) // 2 - info
    WLOG(TAG, "[%s] low battery shutdown triggered by ADC module", __FUNCTION__);
#endif
    m_context.request_shutdown = 1;
#if defined(CONFIG_DISPLAY_ENABLED) && defined(CONFIG_LCD_IS_EPD)
    if(!m_app_ctx.screen_auto_refresh && display_task_is_paused()) {
        display_task_resume_for_times(1, -1, -1, false); // one partial refresh
    }
#endif
}



/*
Method to print the reason by which ESP32 has been awaken from sleep
*/

static wakeup_plan_t wakeup_init(void) {
    FUNC_ENTRY(TAG);
    
#if defined(CONFIG_LOGGER_ADC_ENABLED) && defined(CONFIG_ULP_COPROC_ENABLED)
    /* Initialize ULP program binary on every boot (power-on or wake from sleep)
     * This function checks reset reason and only reloads if needed (power-on reset).
     * Must be called early before any ULP operations. */
    init_ulp_program();
#endif
    
    wakeup_plan_t plan = {
        .immediate_sleep = false,
        .sleep_time = TIME_TO_SLEEP,
        .enable_ext0 = true,
        .enable_ulp = 0,
    };

    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    uint8_t start_ulp = 0;
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
#if defined(CONFIG_LOGGER_ADC_ENABLED) && defined(CONFIG_ULP_COPROC_ENABLED)
    /* Update last_wake_status based on current wake source:
     * - ULP ADC wake: save current as last for next comparison
     * - Other wake: clear last (no ADC wake to compare) */
    adc_ulp_wake_source_t ulp_source = adc_ulp_after_wake();
#endif
    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0:
        case ESP_SLEEP_WAKEUP_EXT1:
            ILOG(TAG, "%s", wakeup_reasons[wakeup_reason]);
            gpio_set_direction((gpio_num_t)WAKE_UP_GPIO, GPIO_MODE_INPUT);
            gpio_set_pull_mode((gpio_num_t)WAKE_UP_GPIO, GPIO_PULLUP_ONLY);
            rtc_gpio_deinit(WAKE_UP_GPIO);
#if defined(CONFIG_LOGGER_ADC_ENABLED) && defined(CONFIG_ULP_COPROC_ENABLED)
            /* Button wake means user wants to interact - clear the running flag
             * so ULP doesn't restart until user finishes and system goes to proper sleep */
            // ulp_was_running = false;
            goto ulp_clear_wake_sources;
#endif
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            ILOG(TAG, "%s", wakeup_reasons[wakeup_reason]);
            start_ulp = 2;
            plan.immediate_sleep = true;
            break;
        case ESP_SLEEP_WAKEUP_ULP:
#if defined(CONFIG_LOGGER_ADC_ENABLED) && defined(CONFIG_ULP_COPROC_ENABLED)
            {
#ifdef CONFIG_ULP_BUTTON_ENABLED
                // Check if button long press detected
                if (ulp_source == WAKE_SOURCE_BUTTON) {
                    ILOG(TAG, "ULP button wake");
                    /* ULP button wake means user wants to interact - clear the running flag
                     * so ULP doesn't restart until user finishes and system goes to proper sleep */
                    // ulp_was_running = false;
                }
                else 
#endif                    
                if (ulp_source == WAKE_SOURCE_BATTERY) {
                        adc_battery_state_t state = get_battery_state_from_ulp();
                        switch (state) {
                            case ADC_BATTERY_CRITICAL_LOW:
                                WLOG(TAG, "ULP wakeup: Battery critical low");
                                plan.immediate_sleep = true;
                                break;
                            case ADC_BATTERY_CHARGING:
                                ILOG(TAG, "ULP wakeup: Charging started");
                                m_app_ctx.app_mode = APP_MODE_CHARGE;
                                // ADC module manages both charge_state and charging_is_on
                                // adc_sync_initial_charging_state(1);
                                break;
                            case ADC_BATTERY_NORMAL:
                            case ADC_BATTERY_CHARGING_STOPPED:
                                ILOG(TAG, "ULP wakeup: Charging stopped");
                                plan.immediate_sleep = true;
                                // ADC module manages both charge_state and charging_is_on
                                // adc_sync_initial_charging_state(0);
                                break;
                            default:
                                break;
                    }
                }             
                // NOTE: Do NOT start ULP here - main program is about to run and ULP can't
                // run simultaneously with main CPU (both would access ADC). ULP will be
                // started in low_to_sleep() just before entering deep sleep.
                
                // If ULP button monitoring was enabled, deinitialize button GPIO from RTC mode
                // so logger_button module can configure it for normal GPIO use
#ifdef CONFIG_ULP_BUTTON_ENABLED
                vTaskDelay(pdMS_TO_TICKS(10));
                // Set button context flag if it was a button wake
                if (ulp_source & WAKE_SOURCE_BUTTON) {
                    ILOG(TAG, "ULP wakeup: Button long press wakeup detected");
                    // Handle button wake appropriately
                }
#endif
                // Clear wake sources after processing
            }
#else
            ILOG(TAG, "%s", wakeup_reasons[wakeup_reason]);
#endif
            start_ulp = 2;
            goto ulp_clear_wake_sources;
            break;
        default:
            ILOG(TAG, "%s int: %d", wakeup_reasons[7], wakeup_reason);
            ulp_clear_wake_sources:
#if defined(CONFIG_LOGGER_ADC_ENABLED) && defined(CONFIG_ULP_COPROC_ENABLED)
            adc_ulp_clear_wake_sources(plan.immediate_sleep);
#endif
            break;
    }

    if (plan.immediate_sleep) {
        // if (m_context_rtc.RTC_voltage_bat < MINIMUM_VOLTAGE) {
        //     WLOG(TAG, "Battery critically low (%.2fV), going back to sleep with monitoring", m_context_rtc.RTC_voltage_bat);
        // }
        m_app_ctx.app_mode = APP_MODE_SLEEP;
        plan.enable_ulp = start_ulp;
    }

    return plan;
}

static void go_to_sleep_or_restart(wakeup_plan_t plan) {
    FUNC_ENTRY(TAG);
#if defined(CONFIG_DISPLAY_ENABLED) && defined(CONFIG_LCD_IS_EPD)
    if(!adc_is_charging()) {
        display_wait_for_task();
    }
#endif
    cleanup();
    if(m_app_ctx.app_mode == APP_MODE_CHARGE) {
#if defined(CONFIG_DISPLAY_ENABLED) && defined(CONFIG_LCD_IS_EPD)
        if(!m_app_ctx.screen_auto_refresh && display_task_is_paused()) {
            display_task_resume_for_times(2, -1, -1, false);
        }
#endif
        return;
    }
    if(plan.sleep_time > 0) {
        ILOG(TAG, "[%s] sleep", __func__);
        low_to_sleep(plan.sleep_time, plan.enable_ext0, plan.enable_ulp);
    } else {
        ILOG(TAG, "[%s] restart", __func__);
        esp_restart();
    }
}

#if defined(CONFIG_LOGGER_USE_WDT)
#ifdef USE_OLD_WDT

// For RTOS, the watchdog has to be triggered

void feedTheDog_Task0() {
    TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;   // write enable
    TIMERG0.wdt_feed = 1;      // feed dog
    TIMERG0.wdt_wprotect = 0;  // write protect
}

void feedTheDog_Task1() {
    TIMERG1.wdt_wprotect = TIMG_WDT_WKEY_VALUE;  // write enable
    TIMERG1.wdt_feed = 1;                        // feed dog
    TIMERG1.wdt_wprotect = 0;                    // write protect
}
#else
static volatile bool run_wdt_loop;
static esp_task_wdt_user_handle_t func_a_twdt_user_hdl;
static esp_task_wdt_user_handle_t func_b_twdt_user_hdl;

static void feedTheDog_Task0(void) {
    esp_task_wdt_reset_user(func_a_twdt_user_hdl);
}

static void feedTheDog_Task1(void) {
    esp_task_wdt_reset_user(func_b_twdt_user_hdl);
}
#endif

static void wdt_user_task() {
    int32_t millis = get_millis();
    int wdt_task0_duration = millis - wdt_task0;
    int wdt_task1_duration = millis - wdt_task1;
    int task_timeout = SEC_TO_MS((WDT_TIMEOUT - 1));  // 1 second less then reboot timeout
    if ((wdt_task0_duration < task_timeout) && (wdt_task1_duration < task_timeout)) {
        feedTheDog_Task0();
        feedTheDog_Task1();
    }
    if ((wdt_task0_duration > task_timeout) && (m_context.downloading_file)) {
        feedTheDog_Task0();
        wdt_task0 = millis;
        WLOG(TAG, "Extend watchdog_timeout due long download");
    }
    if ((wdt_task0_duration > task_timeout) && (!m_context.downloading_file))
        WLOG(TAG, "Watchdog task0 triggered");
    if (wdt_task1_duration > task_timeout)
        WLOG(TAG, "Watchdog task1 triggered");
}

#if !CONFIG_ESP_TASK_WDT_INIT
static void wdt_task(void *arg) {
    // Subscribe this task to TWDT, then check if it is subscribed
    esp_task_wdt_add(NULL);
    esp_task_wdt_status(NULL);

    // Subscribe func_a and func_b as users of the the TWDT
    esp_task_wdt_add_user("feedTheDog_Task0", &func_a_twdt_user_hdl);
    esp_task_wdt_add_user("feedTheDog_Task1", &func_b_twdt_user_hdl);

    ILOG(TAG, "Subscribed to TWDT");
    // int timeout = WDT_TIMEOUT;
    while (run_wdt_loop) {
        esp_task_wdt_reset();
        wdt_user_task();
        delay_ms(50);
    }

    // Unsubscribe this task, func_a, and func_b
    esp_task_wdt_delete_user(func_a_twdt_user_hdl);
    esp_task_wdt_delete_user(func_b_twdt_user_hdl);
    esp_task_wdt_delete(NULL);

    ILOG(TAG, "Unsubscribed from TWDT");

    // Notify main task of deletion
    xTaskNotifyGive((TaskHandle_t)arg);
    // vTaskDelete(NULL);
}
#endif

static void init_watchdog() {
#if !CONFIG_ESP_TASK_WDT_INIT
   esp_task_wdt_config_t twdt_config = {
            .timeout_ms = SEC_TO_MS(WDT_TIMEOUT),
            .idle_core_mask = BIT(portNUM_PROCESSORS) - 1,  // Bitmask of all cores
            .trigger_panic = false,
   };
    esp_task_wdt_init(&twdt_config);
    ILOG(TAG, "TWDT initialized");
    ILOG(TAG, "Create TWDT task");
    run_wdt_loop = true;
    xTaskCreatePinnedToCore(wdt_task, "wdt_task", 4 * 1024,
                            xTaskGetCurrentTaskHandle(), 10, NULL, 0);
#endif  // CONFIG_ESP_TASK_WDT_INIT
}
#endif  // USE_WDT

#if defined(CONFIG_LOGGER_WIFI_ENABLED)
void wifi_sta_conf_sync() {
    FUNC_ENTRY(TAG);
    for(uint8_t i=0, j=5; i<j; ++i) {
        if(i>0 && !m_app_ctx.config->wifi_sta[i].ssid[0]) break;
        if (strcmp(wifi_context.stas[i].ssid, m_app_ctx.config->wifi_sta[i].ssid)) {
            strncpy(wifi_context.stas[i].ssid, m_app_ctx.config->wifi_sta[i].ssid, sizeof(wifi_context.stas[i].ssid) - 1);
            wifi_context.stas[i].ssid[sizeof(wifi_context.stas[i].ssid) - 1] = '\0';
            strncpy(wifi_context.stas[i].password, m_app_ctx.config->wifi_sta[i].password, sizeof(wifi_context.stas[i].password) - 1);
            wifi_context.stas[i].password[sizeof(wifi_context.stas[i].password) - 1] = '\0';
        }
    }
    wifi_context.offset = c_gps_cfg.timezone;
}

// Utility function to check WiFi readiness using event group bits
static bool wifi_is_ready_for_operation(void) {
    if (!wifi_context.s_wifi_event_group || !wifi_context.s_wifi_initialized) {
        return false;
    }
    
    EventBits_t bits = xEventGroupGetBits(wifi_context.s_wifi_event_group);
    
#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ENABLE_WIFI_AP_STA)
    // Check current WiFi mode to determine readiness criteria
    wifi_mode_t current_mode = WIFI_MODE_NULL;
    esp_err_t err = esp_wifi_get_mode(&current_mode);
    if (err != ESP_OK) {
        return false;
    }
    
    bool ap_ready = (bits & WIFI_AP_READY_BIT) != 0;
    bool sta_connected_with_ip = (bits & WIFI_CONNECTED_BIT) != 0;
    
    switch (current_mode) {
        case WIFI_MODE_AP:
            // AP-only mode: ready when AP is operational (voltage stable)
            return ap_ready;
        case WIFI_MODE_STA:
            // STA-only mode: ready when connected with IP (voltage stable after AP→STA transition)
            return sta_connected_with_ip;
        case WIFI_MODE_APSTA:
            // APSTA mode: ready when AP is up (STA connection optional, no voltage impact)
            return ap_ready;
        default:
            return false;
    }
#else
    // For AP-only mode, just need AP ready
    return (bits & WIFI_AP_READY_BIT) != 0;
#endif
}

// Utility function to check if time is synchronized
static bool wifi_time_is_synchronized(void) {
    if (!wifi_context.s_wifi_event_group) {
        return false;
    }
    
    EventBits_t bits = xEventGroupGetBits(wifi_context.s_wifi_event_group);
    return (bits & WIFI_TIME_SYNC_BIT) != 0;
}

static void wifi_deinit(void) {
    FUNC_ENTRY(TAG);
        wifi_uninit();
}

#if defined(CONFIG_LOGGER_WIFI_ENABLED)
// WiFi mode change callback implementations
void wifi_before_mode_change_callback(void) {
    DLOG(TAG, "[%s] WiFi mode change starting - suppressing ADC events & syncing config", __func__);
#if defined CONFIG_LOGGER_ADC_ENABLED
    adc_suppress_events("WiFi mode change");
#endif
#if defined(CONFIG_LOGGER_WIFI_ENABLED)
    wifi_sta_conf_sync();  // Sync WiFi station configuration before mode change
#endif
}

void wifi_after_mode_change_callback(void) {
    FUNC_ENTRY_ARGS(TAG, "WiFi mode change complete");
#if defined CONFIG_LOGGER_ADC_ENABLED
    // Check what type of mode change occurred
    wifi_mode_t current_mode = WIFI_MODE_NULL;
    esp_err_t err = esp_wifi_get_mode(&current_mode);
    
    if (err == ESP_OK) {
        switch (current_mode) {
            case WIFI_MODE_AP:
                // AP-only mode is stable immediately
                ILOG(TAG, "AP-only mode stable - resuming ADC events immediately");
                adc_resume_events("WiFi AP mode stable");
                break;
            case WIFI_MODE_STA:
                // AP→STA transition: need to wait for connection (voltage will stabilize)
                ILOG(TAG, "AP→STA transition - ADC will resume when STA connects");
                // ADC resume handled by STA_GOT_IP event
                break;
            case WIFI_MODE_APSTA:
                // APSTA mode is stable immediately (no voltage change expected)
                ILOG(TAG, "APSTA mode stable - resuming ADC events immediately");
                adc_resume_events("WiFi APSTA mode stable");
                break;
            default:
                ILOG(TAG, "Unknown WiFi mode - resuming ADC events with delay");
                schedule_delayed_adc_resume();
                break;
        }
    } else {
        WLOG(TAG, "Failed to get WiFi mode - using delayed ADC resume");
        schedule_delayed_adc_resume();
    }
#endif
#if defined(CONFIG_DISPLAY_ENABLED) && defined(CONFIG_LCD_IS_EPD)
    if(!m_app_ctx.screen_auto_refresh && display_task_is_paused()){
        display_task_resume_for_times(1, -1, -1, false);
    }
#endif
}
#endif // CONFIG_LOGGER_WIFI_ENABLED

static bool wifi_callbacks_registered = false;

// ADC resume coordination with WiFi stability
static bool adc_resume_pending = false;
static esp_timer_handle_t adc_resume_timer = NULL;

static void adc_resume_timer_callback(void* arg) {
    // Check if WiFi is truly stable for ADC operation
    if (wifi_is_ready_for_operation()) {
        // Additional check: ensure we're not in the middle of a connection attempt
        wifi_mode_t current_mode = WIFI_MODE_NULL;
        esp_err_t err = esp_wifi_get_mode(&current_mode);
        
        bool safe_to_resume = true;
        if (err == ESP_OK && (current_mode == WIFI_MODE_STA || current_mode == WIFI_MODE_APSTA)) {
            // For STA modes, ensure we have stable connection (WIFI_CONNECTED_BIT means STA connected and has IP)
            EventBits_t bits = xEventGroupGetBits(wifi_context.s_wifi_event_group);
            bool sta_connected_with_ip = (bits & WIFI_CONNECTED_BIT) != 0;
            if (!sta_connected_with_ip) {
                safe_to_resume = false;
                WLOG(TAG, "STA mode without IP - delaying ADC resume for voltage stability");
            }
        }
        
        if (safe_to_resume) {
            ILOG(TAG, "WiFi stabilized - resuming ADC events after mode change");
            adc_resume_events("WiFi stabilized after mode change");
            adc_resume_pending = false;
            
            // Clean up timer
            if (adc_resume_timer) {
                esp_timer_delete(adc_resume_timer);
                adc_resume_timer = NULL;
            }
        } else {
            WLOG(TAG, "WiFi ready but not voltage-stable - will retry ADC resume in 500ms");
        }
    } else {
        WLOG(TAG, "WiFi not yet stable - will retry ADC resume in 500ms");
        // Timer will fire again automatically
    }
}

static void schedule_delayed_adc_resume(void) {
    if (adc_resume_timer) {
        esp_timer_stop(adc_resume_timer);
        esp_timer_delete(adc_resume_timer);
        adc_resume_timer = NULL;
    }
    
    esp_timer_create_args_t timer_config = {
        .callback = adc_resume_timer_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "adc_resume_timer"
    };
    
    esp_err_t err = esp_timer_create(&timer_config, &adc_resume_timer);
    if (err == ESP_OK) {
        adc_resume_pending = true;
        ILOG(TAG, "Scheduling delayed ADC resume - waiting for WiFi stability");
        esp_timer_start_periodic(adc_resume_timer, 500000); // Check every 500ms
    } else {
        ELOG(TAG, "Failed to create ADC resume timer: %s", esp_err_to_name(err));
        // Fallback to immediate resume
        adc_resume_events("WiFi mode change complete (timer failed)");
    }
}

void wifi_ensure_callbacks_registered(void) {
    #if defined(CONFIG_LOGGER_WIFI_ENABLED)
    // Set up WiFi mode change callbacks for external dependencies
    if (wifi_callbacks_registered) return;
    wifi_set_mode_change_callbacks(&(wifi_mode_change_callbacks_t){
        .before_mode_change = wifi_before_mode_change_callback,
        .after_mode_change_complete = wifi_after_mode_change_callback
    });
    wifi_callbacks_registered = true;
#endif
}

// WiFi mode change handling moved to WiFi module (wifi_request_mode_change)
// Callbacks for external dependencies are set up in setup()

/**
 * App mode context functions for ADC module
 * These allow the ADC module to make intelligent decisions about charge state management
 */
#if defined(CONFIG_LOGGER_ADC_ENABLED)

// Get current app mode for ADC charge state logic
app_mode_t get_current_app_mode(void) {
    return m_app_ctx.app_mode;
}

// Check if app is in a state where charge events should be filtered
bool should_filter_charge_events(void) {
    // Filter charge events during boot - system not fully initialized
    if (m_app_ctx.app_mode == APP_MODE_BOOT) {
        return true;
    }
    
    // Filter charge events during shutdown/restart - system shutting down
    if (m_app_ctx.app_mode == APP_MODE_SHUT_DOWN || m_app_ctx.app_mode == APP_MODE_RESTART) {
        return true;
    }
    
    // No filtering needed for other modes
    return false;
}

#endif // CONFIG_LOGGER_ADC_ENABLED

void app_mode_wifi_handler(int verbose) {
    if (m_app_ctx.app_mode_wifi_on) return;
    FUNC_ENTRY(TAG);
    m_app_ctx.app_mode = APP_MODE_WIFI;
    m_app_ctx.app_mode_wifi_on = 1;
    
#if defined CONFIG_LOGGER_ADC_ENABLED
    // Suppress ADC events during WiFi transition to prevent false charge detection
    adc_suppress_events("WiFi initialization");
#endif
    
    gps_shut_down();  // save gps
    
    if (!wifi_context.s_wifi_initialized) {
        DLOG(TAG, "[%s] first turn wifi on", __FUNCTION__);
        wifi_ensure_callbacks_registered();
        wifi_init();
#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ENABLE_WIFI_AP_STA)
        wifi_mode(1, 1);
#else
        wifi_mode(0, 1);
        
#endif
        // Wait for AP to be ready with timeout
        if (wifi_wait_for_ap_ready(10000) == 0) {
            ILOG(TAG, "[%s] AP ready and operational", __FUNCTION__);
        } else {
            WLOG(TAG, "[%s] AP startup timeout, continuing anyway", __FUNCTION__);
        }
#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ENABLE_WIFI_AP_STA)                
        // Attempt STA connection with timeout (non-blocking)
        if (wifi_wait_for_connection(15000) == 0) {
            ILOG(TAG, "[%s] STA connected successfully", __FUNCTION__);
            
            // Wait for time synchronization with timeout
            if (wifi_wait_for_time_sync(30000) == 0) {
                ILOG(TAG, "[%s] Time synchronized via SNTP", __FUNCTION__);
            } else {
                WLOG(TAG, "[%s] Time sync timeout, continuing without NTP", __FUNCTION__);
            }
        } else {
            ILOG(TAG, "[%s] STA connection timeout, AP-only mode", __FUNCTION__);
        }
#endif
        
#if defined CONFIG_LOGGER_ADC_ENABLED
        // Resume ADC events after WiFi initialization is complete
        adc_resume_events("WiFi initialization complete");
#endif

        DLOG(TAG, "[%s] wifi started.", __FUNCTION__);
    } else {
        // WiFi already initialized, just ensure we're in the right mode
#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ENABLE_WIFI_AP_STA)
        // Check WiFi and time sync status using utility functions
        if (!wifi_is_ready_for_operation()) {
            ILOG(TAG, "[%s] WiFi not fully operational, waiting for connection...", __FUNCTION__);
            if (wifi_wait_for_connection(10000) == 0) {
                ILOG(TAG, "[%s] STA reconnected successfully", __FUNCTION__);
            }
        }
        if (!wifi_time_is_synchronized()) {
            ILOG(TAG, "[%s] Time not synchronized, waiting...", __FUNCTION__);
            if (wifi_wait_for_time_sync(15000) == 0) {
                ILOG(TAG, "[%s] Time resynchronized via SNTP", __FUNCTION__);
            }
        }
#endif
    }
    
#if defined(CONFIG_DISPLAY_ENABLED) && defined(CONFIG_LCD_IS_EPD)
    if(!m_app_ctx.screen_auto_refresh && display_task_is_paused()){
        display_task_resume_for_times(1, -1, -1, false);
    }
#endif
}
#endif

void app_mode_gps_handler(int verbose) {
    if (m_app_ctx.app_mode_gps_on) return;
    DMEAS_START();
    m_app_ctx.app_mode = APP_MODE_GPS;
    
#if defined CONFIG_LOGGER_ADC_ENABLED
    // Suppress ADC events during WiFi shutdown to prevent false charge detection
    adc_suppress_events("WiFi shutdown");
    
    // Clean up any pending ADC resume timer since we're shutting down WiFi
    if (adc_resume_timer) {
        ILOG(TAG, "Cleaning up pending ADC resume timer for GPS mode");
        esp_timer_stop(adc_resume_timer);
        esp_timer_delete(adc_resume_timer);
        adc_resume_timer = NULL;
        adc_resume_pending = false;
    }
#endif
    
#if defined(CONFIG_LOGGER_WIFI_ENABLED)
    wifi_deinit();
    
    // CRITICAL: Prepare memory for GPS operation after WiFi cleanup
    // GPS requires maximum available memory for proper operation
    wifi_prepare_memory_for_gps();
#endif
    if(m_app_ctx.config_initialized) {
#if (defined(CONFIG_UBLOX_ENABLED) && defined(CONFIG_GPS_LOG_ENABLED))
        gps_start();
#endif
        m_app_ctx.app_mode_gps_on = 1;
#if defined(CONFIG_DISPLAY_ENABLED) && defined(CONFIG_LCD_IS_EPD)
        if(!m_app_ctx.screen_auto_refresh){
            display_task_resume_for_times(1, -1, -1, false);
        }
#endif
        
#if defined CONFIG_LOGGER_ADC_ENABLED
        // Resume ADC events after GPS mode setup is complete
        adc_resume_events("GPS mode setup complete");
#endif

        DMEAS_END(TAG);
    }
}

void task_app_mode_handler(int verbose) {
    switch (m_app_ctx.app_mode) {
        case APP_MODE_RESTART:
        case APP_MODE_SHUT_DOWN:
            m_app_ctx.app_mode_wifi_on = 0;
            m_app_ctx.app_mode_gps_on = 0;
            ILOG(TAG, "[%s] mode: %s", __FUNCTION__, app_mode_str[m_app_ctx.app_mode]);
            gps_shut_down();  // save gps
            if (m_app_ctx.app_mode == APP_MODE_SHUT_DOWN) {
#if defined(CONFIG_LOGGER_ADC_ENABLED) && defined(CONFIG_ULP_COPROC_ENABLED)
                // Clear ULP wake state before sleep so it starts fresh
                adc_ulp_clear_wake_sources(false);
#endif
                go_to_sleep_or_restart((wakeup_plan_t){1, 2, false, false });
            } else {
                // immediate restart
                go_to_sleep_or_restart((wakeup_plan_t){1, 0, false, false });  // restart
            }
            break;
        case APP_MODE_WIFI:
#if defined(CONFIG_LOGGER_WIFI_ENABLED)
            m_app_ctx.app_mode_gps_on = 0;
            app_mode_wifi_handler(verbose);
            break;
#endif
        case APP_MODE_GPS:
        case APP_MODE_BOOT:
            m_app_ctx.app_mode_wifi_on = 0;
            app_mode_gps_handler(verbose);
            break;
        default:
            break;
    };
}

#if defined(CONFIG_DISPLAY_PWR)
void init_power() {
    FUNC_ENTRY(TAG);
    gpio_set_direction((gpio_num_t)CONFIG_DISPLAY_PWR, GPIO_MODE_OUTPUT);
    gpio_set_level(CONFIG_DISPLAY_PWR, 1);
}
#endif    

static void gps_save_rtc() {
    FUNC_ENTRY(TAG);
    m_context_rtc.RTC_distance = avail_fields[fld_distance].value.num();
    m_context_rtc.RTC_alp = avail_fields[fld_a500_display_max].value.num();
    m_context_rtc.RTC_500m = avail_fields[fld_m500_display_max].value.num();
    m_context_rtc.RTC_1h = avail_fields[fld_s3600_display_max].value.num();
    m_context_rtc.RTC_mile = avail_fields[fld_m1852_display_max].value.num(); // null...
    m_context_rtc.RTC_max_2s = avail_fields[fld_s2_display_max].value.num();
    m_context_rtc.RTC_avg_10s = avail_fields[fld_s10_display_avg].value.num();
    m_context_rtc.RTC_R1_10s = avail_fields[fld_s10_r1_display].value.num();
    m_context_rtc.RTC_R2_10s = avail_fields[fld_s10_r2_display].value.num();
    m_context_rtc.RTC_R3_10s = avail_fields[fld_s10_r3_display].value.num();
    m_context_rtc.RTC_R4_10s = avail_fields[fld_s10_r4_display].value.num();
    m_context_rtc.RTC_R5_10s = avail_fields[fld_s10_r5_display].value.num();
    
    get_local_time(&m_context_rtc.rtc_tm);
    // m_context_rtc.RTC_year = ((tms.tm_year) + 1900);
    // m_context_rtc.RTC_month = ((tms.tm_mon) + 1);
    // m_context_rtc.RTC_day = (tms.tm_mday);
    // m_context_rtc.RTC_hour = (tms.tm_hour);
    // m_context_rtc.RTC_min = (tms.tm_min);
}

#if defined(CONFIG_LOGGER_VFS_ENABLED)
static void vfs_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    uint8_t no_auto_refresh = m_app_ctx.config ? !m_app_ctx.screen_auto_refresh : 0;
    if(base == VFS_EVENT) {
        switch(id) {
            case VFS_EVENT_SDCARD_MOUNTED:
                FUNC_ENTRY_ARGS(TAG, " %s", vfs_event_strings(id));
                // m_context.sdOK = true;
                //m_context.freeSpace = sdcard_space();
                break;
            case VFS_EVENT_SDCARD_MOUNT_FAILED:
                FUNC_ENTRY_ARGS(TAG, " %s", vfs_event_strings(id));
                break;
            case VFS_EVENT_SDCARD_UNMOUNTED:
                FUNC_ENTRY_ARGS(TAG, " %s", vfs_event_strings(id));
                // m_context.sdOK = false; 
                break;
            case VFS_EVENT_FAT_PARTITION_MOUNTED:
                FUNC_ENTRY_ARGS(TAG, " %s", vfs_event_strings(id));
                // m_context.sdOK = true;
                break;
            case VFS_EVENT_FAT_PARTITION_MOUNT_FAILED:
                FUNC_ENTRY_ARGS(TAG, " %s", vfs_event_strings(id));
                break;
            case VFS_EVENT_FAT_PARTITION_UNMOUNTED:
                break;
            case VFS_EVENT_LOG_PARTITION_CHANGED:
                FUNC_ENTRY_ARGS(TAG, " %s", vfs_event_strings(id));
                if(log_files_opened(&m_context.gps)) {
                    close_files(&m_context.gps);
                    log_config_init();
                    open_files(&m_context.gps);
                }
                if(vfs_ctx.gps_log_part != VFS_PART_MAX) {
                    m_context.sdOK = true;
                }
                else {
                    m_context.sdOK = false;
                }
#if defined(CONFIG_DISPLAY_ENABLED) && defined(CONFIG_LCD_IS_EPD)
                if(no_auto_refresh && display_task_is_paused()){
                    display_task_resume_for_times(1, -1, -1, false);
                }
#endif
                break;
            default:
                // ILOG(TAG, "[%s] %s:%" PRId32, __FUNCTION__, base, id);
                break;
        }
    } 
}
#endif

#if defined(CONFIG_LOGGER_HTTP_ENABLED)
static void ota_fw_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    uint8_t no_auto_refresh = m_app_ctx.config ? !m_app_ctx.screen_auto_refresh : 0;
    if(base == OTA_FW_EVENT) {
        switch(id) {
            case OTA_FW_EVENT_UPDATE_FINISH:
                m_context.request_restart = 2;
                // m_app_ctx.next_screen = CUR_SCREEN_NONE;
            /* fall through */
            case OTA_FW_EVENT_UPDATE_FAILED:
                m_context.firmware_update_started = 0;
                if(!m_context.request_restart)
                    adc_resume_events("FW update finished");
                goto refresh;
                break;
            case OTA_FW_EVENT_UPDATE_START:
                adc_suppress_events("FW update started");
                m_context.firmware_update_started = 1;
                m_app_ctx.next_screen = CUR_SCREEN_FW_UPDATE;
                refresh:
                FUNC_ENTRY_ARGS(TAG, " %s", ota_fw_event_strings(id));
    #if defined(CONFIG_DISPLAY_ENABLED) && defined(CONFIG_LCD_IS_EPD)
                if(no_auto_refresh && display_task_is_paused()){
                    display_task_resume_for_times(2, -1, -1, false);
                }
    #endif
                break;
            default:
                break;
        }
    } 
}

#if defined(CONFIG_OTA_USE_AUTO_UPDATE)
static void ota_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    uint8_t no_auto_refresh = m_app_ctx.config ? !m_app_ctx.screen_auto_refresh : 0;
    if(base == OTA_AUTO_EVENT) {
        switch(id) {
            case OTA_AUTO_EVENT_UPDATE_FINISH:
                m_context.request_restart = 2;
                // if(!m_context.request_restart) {
                //     m_app_ctx.next_screen = CUR_SCREEN_NONE;
                // }
                /* Fall through */
            case OTA_AUTO_EVENT_UPDATE_FAILED:
                m_context.firmware_update_started = 0;
                if(!m_context.request_restart)
                    adc_resume_events("FW update finished");
                goto refresh;
                break;
            case OTA_AUTO_EVENT_UPDATE_START:
                m_context.firmware_update_started = 1;
                /* fall through */
            case OTA_AUTO_EVENT_UPDATE_AVAILABLE:
                if(id == OTA_AUTO_EVENT_UPDATE_AVAILABLE)
                    m_context.firmware_update_started = 2;
                adc_suppress_events("FW update started");
                m_app_ctx.next_screen = CUR_SCREEN_FW_UPDATE;
                refresh:
                FUNC_ENTRY_ARGS(TAG, " %s", ota_auto_event_strings(id));
#if defined(CONFIG_DISPLAY_ENABLED) && defined(CONFIG_LCD_IS_EPD)
                if(no_auto_refresh && display_task_is_paused()){
                    display_task_resume_for_times(2, -1, -1, false);
                }
#endif
                break;
            default:
                break;
        }
    } 
}
#endif
#endif

static void logger_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    uint8_t no_auto_refresh = m_app_ctx.config ? !m_app_ctx.screen_auto_refresh : 0;
    if(base == LOGGER_EVENT) {
        switch(id) {
            case LOGGER_EVENT_DATETIME_SET:
                ILOG(TAG, "[%s] l %s", __FUNCTION__, logger_event_strings(id));
#if defined(CONFIG_DISPLAY_ENABLED) && defined(CONFIG_LCD_IS_EPD)
                if(no_auto_refresh && display_task_is_paused()){
                    display_task_resume_for_times(1, -1, -1, false);
                }
#endif
                break;
            default:
                // ILOG(TAG, "[%s] %s:%" PRId32, __FUNCTION__, base, id);
                break;
        }
    }
}
static void logger_cfg_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    if(base == LOGGER_CONFIG_EVENT) {
        switch(id) {
            case LOGGER_CONFIG_EVENT_CFG_CHANGED:
                FUNC_ENTRY_ARGS(TAG, " %s, save %hhu", logger_config_event_strings(id), *((uint8_t*)event_data));
                break;
            case LOGGER_CONFIG_EVENT_CFG_SET:
                FUNC_ENTRY_ARGS(TAG, " %s, save %hhu", logger_config_event_strings(id), *((uint8_t*)event_data));
                config_save_json(m_app_ctx.config);
                break;
#if (C_LOG_LEVEL <= LOG_INFO_NUM) // 2 - info
            case LOGGER_CONFIG_EVENT_CFG_GET:
                FUNC_ENTRY_ARGS(TAG, " %s", logger_config_event_strings(id));
                break;
            case LOGGER_CONFIG_EVENT_INIT_DONE:
                FUNC_ENTRY_ARGS(TAG, " %s", logger_config_event_strings(id));
                break;
            case LOGGER_CONFIG_EVENT_SAVE_DONE:
                FUNC_ENTRY_ARGS(TAG, " %s", logger_config_event_strings(id));
                break;
            case LOGGER_CONFIG_EVENT_SAVE_FAIL:
                FUNC_ENTRY_ARGS(TAG, " %s", logger_config_event_strings(id));
                break;
#endif
            default:
                break;
        }

    }
}

#if defined(CONFIG_UBLOX_ENABLED)
static void ubx_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    uint8_t no_auto_refresh = m_app_ctx.config ? !m_app_ctx.screen_auto_refresh : 0;
    if(base == UBX_EVENT) {
        switch(id) {
            case UBX_EVENT_DATETIME_SET:
                goto refresh;
                break;
            case UBX_EVENT_UART_INIT_DONE:
            case UBX_EVENT_UART_INIT_FAIL:
                goto refresh;
                break;
            case UBX_EVENT_UART_DEINIT_DONE:
                break;
            case UBX_EVENT_SETUP_DONE:
            case UBX_EVENT_SETUP_FAIL:
                refresh:
                FUNC_ENTRY_ARGS(TAG, " %s", ubx_event_strings(id));
#if defined(CONFIG_DISPLAY_ENABLED) && defined(CONFIG_LCD_IS_EPD)
                if(no_auto_refresh && display_task_is_paused()){
                    display_task_resume_for_times(1, -1, -1, false);
                }
#endif
                break;
            default:
                break;
        }
    }
}
#endif

#if defined(CONFIG_GPS_LOG_ENABLED)
static void gps_log_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    if(base==GPS_LOG_EVENT) {
        const char * c = 0;
        uint8_t *a = (uint8_t*)event_data;
        switch(id) {
            case GPS_LOG_EVENT_GPS_NAV_MODE_CHANGED:
                FUNC_ENTRY_ARGS(TAG, " %s", gps_log_event_strings(id));
#if (C_LOG_LEVEL <= LOG_INFO_NUM) // 2 - info
                gps_log_nav_mode_change(&m_context.gps, 1);
#endif
                break;
            case GPS_LOG_EVENT_LOG_FILES_OPENED:
                FUNC_ENTRY_ARGS(TAG, " %s", gps_log_event_strings(id));
                m_context.Shut_down_Save_session = true;
                goto printfiles;
                break;
            case GPS_LOG_EVENT_LOG_FILES_OPEN_FAILED:
                FUNC_ENTRY_ARGS(TAG, " %s", gps_log_event_strings(id));
                printfiles:
#if (C_LOG_LEVEL <= LOG_DEBUG_NUM) // 3 - debug
                for(uint8_t i=0; i<SD_FD_END; i++) {
                    printf("** [%s] ", i==SD_GPX ? "GPX" : 
#ifdef GPS_LOG_ENABLE_GPY
                    i==SD_GPY ? "GPY" : 
#endif
                    i==SD_SBP ? "SBP" : i==SD_UBX ? "UBX" : i==SD_TXT ? "TXT" : "-");
                    if(GETBIT(m_context.gps.log_config->log_file_bits, i) && m_context.gps.log_config->filefds[i]<=0)
                    {
                        printf(" ERROR to");
                    }
                    if(c) {
                        printf(" open: %s ",  m_context.gps.log_config->filenames[i]);
                    }
                    printf("\n");
                }
#endif
                break;
            case GPS_LOG_EVENT_LOG_FILES_SAVED:
                FUNC_ENTRY_ARGS(TAG, " %s", gps_log_event_strings(id));
                break;
            case GPS_LOG_EVENT_LOG_FILES_CLOSED:
                FUNC_ENTRY_ARGS(TAG, " %s", gps_log_event_strings(id));
                break;
            case GPS_LOG_EVENT_GPS_SAVE_FILES:
                FUNC_ENTRY_ARGS(TAG, " %s", gps_log_event_strings(id));
                m_app_ctx.next_screen = CUR_SCREEN_SAVE_SESSION;
                gps_save_rtc();
                break;
            case GPS_LOG_EVENT_GPS_SHUT_DOWN_DONE:
                FUNC_ENTRY_ARGS(TAG, " %s", gps_log_event_strings(id));
                if(m_app_ctx.next_screen == CUR_SCREEN_SAVE_SESSION) {
                    m_app_ctx.next_screen = CUR_SCREEN_NONE;
                }
                break;
            case GPS_LOG_EVENT_GPS_REQUEST_RESTART:
                FUNC_ENTRY_ARGS(TAG, " %s", gps_log_event_strings(id));
                m_context.request_restart = 2;
                break;
            case GPS_LOG_EVENT_GPS_IS_MOVING:
                adc_suppress_events("GPS is moving");
                FUNC_ENTRY_ARGS(TAG, " %s", gps_log_event_strings(id));
#if defined(CONFIG_DISPLAY_ENABLED) && defined(CONFIG_LCD_IS_EPD)
                if(!m_app_ctx.screen_auto_refresh) {
                    display_task_resume();
                }
#endif
                break;
            case GPS_LOG_EVENT_GPS_IS_STOPPING:
                adc_resume_events("GPS is stopping");
                FUNC_ENTRY_ARGS(TAG, " %s", gps_log_event_strings(id));
#if defined(CONFIG_DISPLAY_ENABLED) && defined(CONFIG_LCD_IS_EPD)
                if(!m_app_ctx.screen_auto_refresh) {
                    display_start_task_pause_seq();
                    // display_task_resume_for_times(1, 1, -1, false);
                }
#endif
                break;
            case GPS_LOG_EVENT_GPS_FIRST_FIX:
                FUNC_ENTRY_ARGS(TAG, " %s", gps_log_event_strings(id));
#if defined(CONFIG_DISPLAY_ENABLED) && defined(CONFIG_LCD_IS_EPD)
                if(!m_app_ctx.screen_auto_refresh && display_task_is_paused()) {
                    display_task_resume_for_times(1, -1, -1, true);
                }
#endif
                break;
            case GPS_LOG_EVENT_GPS_NEW_RUN:
                FUNC_ENTRY_ARGS(TAG, " %s", gps_log_event_strings(id));
#if defined(CONFIG_DISPLAY_ENABLED) && defined(CONFIG_LCD_IS_EPD)
                display_cancel_delay();
#endif
                break;
            case GPS_LOG_EVENT_CFG_CHANGED:
                FUNC_ENTRY_ARGS(TAG, " %s d %hhu", gps_log_event_strings(id), *((uint8_t*)event_data));
                break;
            case GPS_LOG_EVENT_CFG_SET:
                FUNC_ENTRY_ARGS(TAG, " %s, save %hhu", gps_log_event_strings(id), *((uint8_t*)event_data));
                struct gps_user_cfg_evt_data_s * evt_data = event_data;
                if(evt_data->pos == gps_cfg_timezone) {
                    struct tm tm;
                    get_local_time(&tm);
                    c_set_time(&tm, 0, c_gps_cfg.timezone - evt_data->value);
                }
                config_save_json(m_app_ctx.config);
                break;
            default:
                // ILOG(TAG, "[%s] %s:%" PRId32, __FUNCTION__, base, id);
                break;
        }
    }
}
#endif

#if defined CONFIG_LOGGER_ADC_ENABLED
static void adc_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    uint8_t no_auto_refresh = m_app_ctx.config ? !m_app_ctx.screen_auto_refresh : 0;
    if(base == ADC_EVENT) {
        // Check if ADC events should be suppressed during WiFi/GPS transitions
        if (adc_should_suppress_event(id)) {
            return;  // Silently ignore the event
        }
        switch(id) {
            case ADC_EVENT_UPDATE:
                // Update RTC voltage from ADC module event data - no display refresh needed for voltage updates
                if (event_data) {
                    float voltage = *(float*)event_data;
                    // m_context_rtc.RTC_voltage_bat = voltage;
                    DLOG(TAG, "[%s] ADC voltage updated to %.2fV", __FUNCTION__, voltage);
                }
                break;
            case ADC_EVENT_CRITICAL_LOW:
                FUNC_ENTRY_ARGS(TAG, " %s", adc_event_strings(id));
                // ADC module manages charge_state - no need to duplicate in main
                goto refresh;
                break;
            case ADC_EVENT_NORMAL:
                FUNC_ENTRY_ARGS(TAG, " %s", adc_event_strings(id));
                // Battery is back to normal - ADC module manages state, just refresh display
                adc_battery_state_t current_state = battery_get_current_battery_state();
                if (current_state == ADC_BATTERY_NORMAL) {
                    goto refresh;
                }
                break;
            case ADC_EVENT_CHARGING:
                FUNC_ENTRY_ARGS(TAG, " %s", adc_event_strings(id));
                if(m_app_ctx.app_mode == APP_MODE_BOOT) break;
                // ADC module now manages charging_is_on flag internally
                // ADC module manages charge_state - no need to duplicate in main
                // No automatic app mode switching - only update charging state
                // WLOG(TAG, "main: charging started, voltage: %.2fV", m_context_rtc.RTC_voltage_bat);
                goto refresh;
                break;
            case ADC_EVENT_CHARGING_STOPPED:
                FUNC_ENTRY_ARGS(TAG, " %s", adc_event_strings(id));
                if(m_app_ctx.app_mode == APP_MODE_BOOT) break;
                // ADC module now manages charging_is_on flag internally
                // ADC module manages charge_state - no need to duplicate in main
                // WLOG(TAG, "main: charging stopped, voltage: %.2fV", m_context_rtc.RTC_voltage_bat);
                if (m_app_ctx.app_mode == APP_MODE_CHARGE) {
                    m_context.request_shutdown = 1;
                } else {
                    goto refresh;
                }
                break;
            refresh:
#if defined(CONFIG_DISPLAY_ENABLED) && defined(CONFIG_LCD_IS_EPD)
                if(no_auto_refresh && display_task_is_paused()){
                    display_task_resume_for_times(2, -1, -1, false);
                }
#endif
                break;
            default:
                FUNC_ENTRY_ARGS(TAG, " %s:%ld", base, id);
                break;
        }
    }
}
#endif

#if defined CONFIG_LOGGER_WIFI_ENABLED
static void wifi_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    uint8_t no_auto_refresh = m_app_ctx.config ? !m_app_ctx.screen_auto_refresh : 0;
    
    if(base == WIFI_EVENT) {
        switch(id) {
            case WIFI_EVENT_AP_START:
                // Check WiFi operational status using utility function
                if (wifi_is_ready_for_operation()) {
                    ILOG(TAG, "[%s] WiFi fully operational", __FUNCTION__);
                } else {
                    ILOG(TAG, "[%s] AP started but not yet ready", __FUNCTION__);
                }
                // ADC event resumption is now handled by WiFi module for unified flow
                goto refresh;
                break;
            case WIFI_EVENT_AP_STOP:
                ILOG(TAG, "[%s] AP stopped", __FUNCTION__);
                goto refresh;
                break;
            case WIFI_EVENT_STA_START:
                ILOG(TAG, "[%s] STA started", __FUNCTION__);
                
                // Only suppress ADC for AP→STA transitions (not APSTA mode)
                #if defined CONFIG_LOGGER_ADC_ENABLED
                wifi_mode_t current_mode = WIFI_MODE_NULL;
                esp_err_t err = esp_wifi_get_mode(&current_mode);
                if (err == ESP_OK && current_mode == WIFI_MODE_STA) {
                    // This is AP→STA transition, voltage will increase, suppress ADC
                    adc_suppress_events("AP to STA transition - voltage increase expected");
                    ILOG(TAG, "ADC suppressed for AP→STA transition (voltage increase)");
                } else {
                    ILOG(TAG, "APSTA mode - no ADC suppression needed");
                }
                #endif
                
                goto refresh;
                break;
            case WIFI_EVENT_STA_STOP:
                ILOG(TAG, "[%s] STA stopped", __FUNCTION__);
                goto refresh;
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ILOG(TAG, "[%s] STA connected to AP", __FUNCTION__);
                goto refresh;
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ILOG(TAG, "[%s] STA disconnected from AP", __FUNCTION__);
                goto refresh;
                break;
            default:
                break;
        }
    }
    else if(base == IP_EVENT) {
        switch(id) {
            case IP_EVENT_STA_GOT_IP:
                // Check connection and time sync status using utility functions
                ILOG(TAG, "[%s] STA got IP - WiFi: %s, Time sync: %s", 
                     __FUNCTION__,
                     wifi_is_ready_for_operation() ? "Ready" : "Pending",
                     wifi_time_is_synchronized() ? "Synced" : "Pending");
                
                if (!wifi_time_is_synchronized()) {
                    ILOG(TAG, "[%s] Waiting for time synchronization...", __FUNCTION__);
                }
                
                // Resume ADC events for AP→STA transition (voltage should now be stable)
                #if defined CONFIG_LOGGER_ADC_ENABLED
                wifi_mode_t current_mode = WIFI_MODE_NULL;
                esp_err_t err = esp_wifi_get_mode(&current_mode);
                if (err == ESP_OK && current_mode == WIFI_MODE_STA) {
                    // This was AP→STA transition, voltage is now stable
                    ILOG(TAG, "STA got IP after AP→STA transition - resuming ADC events");
                    adc_resume_events("AP→STA transition complete - voltage stable");
                }
                
                // Cancel any pending delayed resume regardless of mode
                if (adc_resume_pending) {
                    ILOG(TAG, "Canceling delayed ADC resume - connection established");
                    adc_resume_pending = false;
                    
                    if (adc_resume_timer) {
                        esp_timer_stop(adc_resume_timer);
                        esp_timer_delete(adc_resume_timer);
                        adc_resume_timer = NULL;
                    }
                }
                #endif
                
                goto refresh;
                break;
            case IP_EVENT_STA_LOST_IP:
                WLOG(TAG, "[%s] STA lost IP - WiFi: %s, Time sync: %s", 
                     __FUNCTION__,
                     wifi_is_ready_for_operation() ? "Still ready" : "Not ready",
                     wifi_time_is_synchronized() ? "Still valid" : "Lost");
                goto refresh;
                break;
            default:
                break;
        }
    }
    return;
    
    refresh:
    FUNC_ENTRY_ARGS(TAG, " %s", wifi_event_strings(id));
#if defined(CONFIG_DISPLAY_ENABLED) && defined(CONFIG_LCD_IS_EPD)
    if(no_auto_refresh && display_task_is_paused()){
        display_task_resume_for_times(1, -1, -1, false);
    }
#endif
}
#endif

#if defined(CONFIG_DISPLAY_ENABLED)
static void ui_event_handler(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
#if defined(CONFIG_LCD_IS_EPD)
    if(base == UI_EVENT) {
        switch(id) {
            case UI_EVENT_FLUSH_START:
                FUNC_ENTRY_ARGS(TAG, " %s", ui_event_strings(id));
                break;
            case UI_EVENT_FLUSH_DONE:
                FUNC_ENTRY_ARGSD(TAG, " %s flush_count:%lu buf_update_count:%lu", ui_event_strings(id), display_get_flush_count(), display_get_buf_update_count());
                break;
            default:
                break;
        }
    }
#endif
}
#endif

static esp_err_t events_init() {
    FUNC_ENTRY(TAG);
    if (s_event_loop_ready) {
        return ESP_OK;
    }
    esp_err_t loop_err = esp_event_loop_create_default();
    if (loop_err != ESP_OK) {
        WLOG(TAG, "[%s] event loop create fail: %s", __FUNCTION__, esp_err_to_name(loop_err));
        return loop_err;
    }
    s_event_loop_ready = true;

    esp_err_t err = register_event_handlers();
    if (err != ESP_OK) {
        WLOG(TAG, "[%s] event registration incomplete: %s", __FUNCTION__, esp_err_to_name(err));
    }
    return err;
}

static esp_err_t events_deinit() {
    FUNC_ENTRY(TAG);
    if (!s_event_loop_ready) {
        return ESP_OK;
    }
    unregister_event_handlers();
    esp_err_t err = esp_event_loop_delete_default();
    if (err != ESP_OK) {
        WLOG(TAG, "[%s] event loop delete fail: %s", __FUNCTION__, esp_err_to_name(err));
    }
    s_event_loop_ready = false;
    return ESP_OK;
}

// observer like callback
static void config_changed_cb(const char *key) {
    FUNC_ENTRY_ARGS(TAG, "key: %s", key);
    if(strcmp(key, "screen_rotation")==0 || strcmp(key, "board_logo")==0||strcmp(key, "sail_logo")==0||strcmp(key, "speed_unit")==0 || strcmp(key, "sleep_info")==0 || strcmp(key, "bat_view") == 0) {
        g_context_rtc_add_config(&m_context_rtc, m_app_ctx.config);
#if defined(CONFIG_DISPLAY_ENABLED) && defined(CONFIG_LCD_IS_EPD)
        goto refresh;
#endif
    }
#if defined(CONFIG_DISPLAY_ENABLED)
    if(strcmp(key, "screen_rotation") == 0) {
        display_set_rotation(m_app_ctx.config->screen.screen_rotation);
#if defined(CONFIG_LCD_IS_EPD)
    refresh:
        if(!m_app_ctx.screen_auto_refresh && display_task_is_paused()){
            display_task_resume_for_times(1, -1, -1, false);
        }
#endif
    }
#if !defined(CONFIG_LCD_IS_EPD)
    if(strcmp(key, "screen_brightness")==0) {
        display_drv_bl_set(m_app_ctx.config->screen_brightness);
    }
#endif
#endif
}

// ADC event suppression functions are now implemented in the adc module

static void ctx_load_cb() {
    FUNC_ENTRY(TAG);
#if defined(CONFIG_GPS_LOG_ENABLED)
    log_config_init();
#endif
#if defined(CONFIG_LOGGER_HTTP_ENABLED)
    http_rest_init(CONFIG_WEB_APP_PATH);
#endif
    m_app_ctx.config = config_new();
    if(!m_app_ctx.config) {
        ELOG(TAG, "[%s] config_new failed!", __FUNCTION__);
        return;
    }
    m_app_ctx.config->config_changed_screen_cb = config_changed_cb;
    g_context_defaults(&m_context);
    m_context.config = m_app_ctx.config;
    if (!m_context.gps.Gps_fields_OK) {
#if defined(CONFIG_GPS_LOG_ENABLED)
        gps_init(&m_context.gps);
#endif
    }
    delay_ms(50);

    config_load_json(m_app_ctx.config);
#if defined(CONFIG_DISPLAY_ENABLED) && !defined(CONFIG_LCD_IS_EPD)
    if(m_context_rtc.RTC_screen_brightness != m_app_ctx.config->screen_brightness)
        display_drv_bl_set(m_app_ctx.config->screen_brightness);
#endif
    g_context_rtc_add_config(&m_context_rtc, m_app_ctx.config);
#if defined(CONFIG_DISPLAY_ENABLED)
    if(display_drv_get_rotation() != m_context_rtc.RTC_screen_rotation){
        display_set_rotation(m_context_rtc.RTC_screen_rotation);
    }
#endif
    g_context_add_config(&m_context, m_app_ctx.config);
    config_fix_values(m_app_ctx.config);
    gps_config_fix_values();
    // g_context_ubx_add_config(&m_context, 0);
    // log_config_add_config(m_context.gps.log_config, m_app_ctx.config);
    m_app_ctx.config_initialized = 1;
    setenv("TZ", "UTC", 0);
    tzset();
    // c_set_time(&m_context_rtc.rtc_tm, 0, 0);
#if defined(CONFIG_DISPLAY_ENABLED) && defined(CONFIG_LCD_IS_EPD)
    if(!m_app_ctx.screen_auto_refresh && display_task_is_paused()){
       display_task_resume_for_times(1, -1, -1, false);
    }
#endif
}

static void setup(uint8_t initial) {
    FUNC_ENTRY(TAG);
    m_app_ctx.app_mode = APP_MODE_BOOT;
#if defined(CONFIG_IDF_TARGET_ESP32S3)
    init_power();
#endif
    init_rtc();
    wakeup_plan_t wake_plan = initial ? wakeup_init() : (wakeup_plan_t){0};
    events_init();
#if defined(CONFIG_LOGGER_ADC_ENABLED)
    adc_init();  // This now automatically initializes ULP if available
    adc_set_low_battery_callback(on_low_battery_shutdown);  // Register shutdown callback
    // adc_set_minimum_battery_voltage(MINIMUM_VOLTAGE);       // Set voltage threshold
    delay_ms(INIT_DELAY_MEDIUM_MS);
    
    // Additional delay to ensure ADC has time for initial readings before state check
    delay_ms(100);
    
    // Check initial charging state during boot - but don't override wakeup-detected charge mode
    ILOG(TAG, "Boot: Initial ADC state = %d, current app_mode = %s", battery_get_current_battery_state(), app_mode_str[m_app_ctx.app_mode]);
    
    // if (m_app_ctx.app_mode != APP_MODE_CHARGE) {
    //     // Only set charge mode if not already set by wakeup init
    //     if (initial_state == ADC_BATTERY_CHARGING_STARTED || initial_state == ADC_BATTERY_HIGH) {
    //         ILOG(TAG, "Boot: Charging detected - entering charge mode");
    //         m_app_ctx.charging_is_on = 1;
    //         m_app_ctx.charge_state = initial_state;
    //         m_app_ctx.app_mode = APP_MODE_CHARGE;
    //         adc_sync_initial_charging_state(1);
    //     } else {
    //         ILOG(TAG, "Boot: No charging - normal operation");
    //         m_app_ctx.charging_is_on = 0;
    //         m_app_ctx.charge_state = initial_state;
    //         adc_sync_initial_charging_state(0);
    //     }
    // } else {
    //     // Charge mode already set by wakeup - just sync the state
    //     ILOG(TAG, "Boot: Charge mode already set by wakeup - syncing state");
    //     m_app_ctx.charge_state = initial_state;
    //     m_app_ctx.charging_is_on = (initial_state == ADC_BATTERY_CHARGING_STARTED || initial_state == ADC_BATTERY_HIGH) ? 1 : 0;
    // }
#endif
    // Initial battery check is now handled by ADC timer automatically
#if defined(CONFIG_LOGGER_USE_WDT)
    init_watchdog();
#endif
#if defined(CONFIG_DISPLAY_ENABLED)
    m_app_ctx.screen_auto_refresh = m_context_rtc.RTC_screen_auto_refresh;
    lcd_init();
    display_set_rotation(m_context_rtc.RTC_screen_rotation);
#if !defined(CONFIG_LCD_IS_EPD)
    display_drv_bl_set(m_context_rtc.RTC_screen_brightness==-1 ? SCR_DEFAULT_BRIGHTNESS : m_context_rtc.RTC_screen_brightness);
#endif
    display_task_start();
    delay_ms(10);
#endif
    if (initial && wake_plan.immediate_sleep) {
        go_to_sleep_or_restart(wake_plan);  // sleep immediately
    }
#if defined(CONFIG_LOGGER_BUTTON_ENABLED)
    init_button();
    delay_ms(INIT_DELAY_SHORT_MS);
#endif

    ILOG(TAG, "[%s] done, c_log_level:%d.", __FUNCTION__, C_LOG_LEVEL);
}

static void service_power_requests(void) {
    FUNC_ENTRY_ARGT(TAG, " restart_req:%d shutdown_req:%d", m_context.request_restart, m_context.request_shutdown);
    const bool restart_requested = (m_context.request_restart > 0);
    const bool shutdown_requested = m_context.request_shutdown;

    if (!restart_requested && !shutdown_requested) {
        return;
    }

    if (restart_requested) {
        if(m_app_ctx.app_mode == APP_MODE_CHARGE && m_context.request_restart < 2) {
            setup(0);
        }
        else
            m_app_ctx.app_mode = APP_MODE_RESTART;
    } else {
        m_app_ctx.app_mode = APP_MODE_SHUT_DOWN;
    }

#if defined(CONFIG_DISPLAY_ENABLED) && defined(CONFIG_LCD_IS_EPD)
    if(!m_app_ctx.screen_auto_refresh && display_task_is_paused()) {
        display_task_resume_for_times(1, -1, -1, false); // one partial refresh
    }
#endif

    m_context.request_restart = 0;
    m_context.request_shutdown = 0;
}

static void ensure_app_ready(void) {
    if (m_app_ctx.config_initialized
        || m_app_ctx.app_mode == APP_MODE_CHARGE 
        || m_app_ctx.app_mode == APP_MODE_SLEEP 
        || m_app_ctx.app_mode == APP_MODE_SHUT_DOWN 
        || m_app_ctx.app_mode == APP_MODE_RESTART) {
        return;
    }
    FUNC_ENTRY_ARGT(TAG, " app_mode:%s", app_mode_str[m_app_ctx.app_mode]);
    if (vfs_ctx.vfs_initialized == 0) {
        logger_buffer_pool_init();
        vfs_init();
#if defined(CONFIG_BMX_ENABLE)
        init_bmx();
#endif
    }
    if (vfs_ctx.config_part == VFS_PART_MAX) {
        return;
    }
    FUNC_ENTRY_ARGSD(TAG, " config not loaded, do it as sdcard is initialized.");
    ctx_load_cb();
}

static bool run_periodic_diagnostics(uint32_t loop_counter) {
    FUNC_ENTRY_ARGT(TAG, " loop_counter:%lu", loop_counter);
    bool verbose = false;
#if (C_LOG_LEVEL <= LOG_WARN_NUM)
    const uint32_t diag_period = 50U;
    if ((loop_counter % diag_period) == 0U) {
        print_lv_mem_mon();
        mem_info();
#if (C_LOG_LEVEL <= LOG_DEBUG_NUM) // 3 - debug
        tasks_memory_info();
        task_top();
#endif
#if (C_LOG_LEVEL <= LOG_INFO_NUM) // 2 - info
        if(m_app_ctx.app_mode == APP_MODE_GPS && m_context.gps.ubx_device) {
            struct ubx_msg_s *ubxMessage = &m_context.gps.ubx_device->ubx_msg;
            if(ubxMessage->navPvt.valid) {
                WLOG(TAG, "sAcc: %lu mm/s, numSv: %hhu, hDop: %.02f", ubxMessage->navPvt.sAcc, ubxMessage->navPvt.numSV, ubxMessage->navDOP.hDOP/1000.0f);
            } else {
                WLOG(TAG, "sAcc: %lu mm/s, numSv: 0, hDop: %.02f", ubxMessage->navPvt.sAcc, ubxMessage->navDOP.hDOP/1000.0f);
            }
        }
#endif
#if (C_LOG_LEVEL <= LOG_DEBUG_NUM) // 3 - debug
#if defined(CONFIG_LOGGER_ADC_ENABLED) && defined(CONFIG_ULP_COPROC_ENABLED)
        /* Monitor ULP activity during wake time - shows if ULP is running */
        static uint32_t last_cycle_count = 0;
        uint32_t current_cycle = adc_ulp_get_cycle_count();
        int32_t delta = (int32_t)(current_cycle - last_cycle_count);
        DLOG(TAG, "ULP cycle_count: %lu (delta: %ld since last check) - %s", 
             current_cycle, delta, delta > 0 ? "RUNNING" : "FROZEN");
        last_cycle_count = current_cycle;
        debug_ulp_status();
#endif
#endif
        verbose = true;
    }
#else
    UNUSED_PARAMETER(loop_counter);
#endif
    return verbose;
}

static void service_display(bool verbose, uint32_t loop_start_ms) {
#if defined(CONFIG_DISPLAY_ENABLED)
#if defined(CONFIG_LCD_IS_EPD)
    if(!m_app_ctx.screen_auto_refresh && !m_app_ctx.display.first_flush_done) {
        DLOG(TAG, "** Wait for first flush done %s", app_mode_str[m_app_ctx.app_mode]);
        return;
    }
#endif
    task_app_mode_handler(verbose);
    // WiFi mode change handling is now done via direct API calls from HID module
#if defined(CONFIG_LCD_IS_EPD)
    if(!m_app_ctx.screen_auto_refresh && m_app_ctx.display.first_flush_done == 1) {
        DLOG(TAG, "** pause task when first_flush_done: %s %hhu count: %lu", app_mode_str[m_app_ctx.app_mode], m_app_ctx.display.first_flush_done, display_get_flush_count());
        if(display_get_flush_count() >= 3 || (get_millis() - loop_start_ms) > SEC_TO_MS(12)) {
            DLOG(TAG, "** pause task when first_flush_done: %s %hhu count: %lu", app_mode_str[m_app_ctx.app_mode], m_app_ctx.display.first_flush_done, display_get_flush_count());
            display_task_pause();
            m_app_ctx.display.first_flush_done = 2;
        }
    }
#endif
#else
    UNUSED_PARAMETER(verbose);
    UNUSED_PARAMETER(loop_start_ms);
#endif
}
static void cleanup(void) {
    FUNC_ENTRY(TAG);
#if defined(CONFIG_LOGGER_USE_WDT)
    run_wdt_loop = false;
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
#if !CONFIG_ESP_TASK_WDT_INIT
    esp_task_wdt_deinit();
#endif
#endif
#if defined(CONFIG_BMX_ENABLE)
    deinit_bmx();
#endif
#if defined CONFIG_LOGGER_ADC_ENABLED
    // Suppress ADC events during system shutdown
    adc_suppress_events("system shutdown");
#endif

#if defined(CONFIG_LOGGER_WIFI_ENABLED)
    wifi_deinit();
#endif
#if defined(CONFIG_GPS_LOG_ENABLED)
    gps_deinit();
#endif
    if (m_app_ctx.config) {
        config_delete(m_app_ctx.config);
        m_app_ctx.config = NULL;
        m_app_ctx.config_initialized = 0;
    }
#if defined(CONFIG_LOGGER_ADC_ENABLED)
    if(adc_is_charging()) {
        // Battery voltage is maintained by ADC timer automatically
        m_app_ctx.app_mode = APP_MODE_CHARGE;
        ILOG(TAG, "[%s] charging is on, go to charge mode.", __FUNCTION__);
        return;
    }
#endif
    // esp_timer_stop(screen_periodic_timer);
#if defined(CONFIG_DISPLAY_ENABLED)
    lcd_deinit();
#endif
#if defined(CONFIG_LOGGER_ADC_ENABLED)
    adc_deinit();  // This now cleans up all ADC resources including low battery timer
#endif
    vfs_deinit();
#if defined(CONFIG_LOGGER_BUTTON_ENABLED)
    deinit_button();
#endif
    events_deinit();
}

void app_main(void) {
    FUNC_ENTRY(TAG);
    uint32_t loop_counter = 0;
#if defined(CONFIG_LCD_IS_EPD)
    const uint32_t loop_start_ms = get_millis();
#else
    const uint32_t loop_start_ms = 0U;
#endif
    // rtc_wdt_protect_off();
    setup(1);
    while (1) {
        // Battery monitoring is now handled by ADC timer - no periodic calls needed

        const bool verbose = run_periodic_diagnostics(loop_counter);
        ++loop_counter;
        service_power_requests();
        ensure_app_ready();
        service_display(verbose, loop_start_ms);

        delay_ms(MAIN_LOOP_PERIOD_MS);
    }
    cleanup();
}
