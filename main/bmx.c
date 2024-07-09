

#include <driver/gpio.h>
#include <esp_log.h>

#include "private.h"
#include "logger_common.h"
#if defined(CONFIG_BMX_ENABLE)
#include "bmx.h"
#include <bmx280.h>

static const char *TAG = "bmx";
static bmx280_t *bmx280 = 0;
static bool bmx_run = true;
static TaskHandle_t t1;
static bmx_t bmx_stat = {0, 0, 0, 0, 0, 0};
esp_timer_handle_t bmx_periodic_timer;
TIMER_INIT

// #define BMX_TASK_STACK_SIZE 1536
#define BMX_INTERVAL_SEC 10

static esp_err_t bmx_read() {
    TIMER_S
    esp_err_t err = ESP_OK;
    const char * last_err = 0;
    bmx_stat.millis = get_millis();
    bmx280_setMode(bmx280, BMX280_MODE_FORCE);
    do {
        vTaskDelay(pdMS_TO_TICKS(2));
    } while (bmx280_isSampling(bmx280));

    err = bmx280_readout(bmx280, &bmx_stat.temp, &bmx_stat.press, &bmx_stat.humid);
    bmx_stat.elapsed = get_millis() - bmx_stat.millis;
    if (err != ESP_OK) {
        last_err = esp_err_to_name(err);
        bmx_stat.status = -11;
    } else {
        bmx_stat.status = 0;
    }
    bmx280_setMode(bmx280, BMX280_MODE_SLEEP);
//done:
    if(bmx_stat.status!=0) {
        ESP_LOGE(TAG, "Bmx280 %s failed: %s ", bmx_stat.status==-10?"set_mode force":bmx_stat.status==-11?"read":"set_mode sleep", last_err);
    } else {
        ESP_LOGI(TAG, "Bmx280 Values: temp = %" PRId32 ", pres = %" PRIu32 ", hum = %" PRIu32 ", elapsed = %" PRIu32, bmx_stat.temp, bmx_stat.press, bmx_stat.humid, bmx_stat.elapsed);
    }
    TIMER_E
    return err;
}

// static void bmx_task() {
static void periodic_timer_callback(void *arg) {
    // while (bmx_run) {
        //LOGR;
        bmx_read();
    //    task_memory_info("bmxTimer");
    //     delay_ms(BMX_INTERVAL_SEC * 1000U);
    // }
    // vTaskDelete(NULL);
}

esp_err_t init_bmx() {
    LOGR;
    esp_err_t ret = ESP_OK;
    i2c_config_t i2c_cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = CONFIG_BMX_SDA,
        .scl_io_num = CONFIG_BMX_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000};
    ret = i2c_param_config(I2C_NUM_0, &i2c_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2c_param_config failed: %s", esp_err_to_name(ret));
        bmx_stat.status = -5;
    }
    ret = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2c_driver_install failed: %s", esp_err_to_name(ret));
        bmx_stat.status = -6;
    }
    bmx280 = bmx280_create(I2C_NUM_0);

    if (!bmx280) {
        ESP_LOGE("test", "Could not create bmx280 driver.");
        bmx_stat.status = -2;
    }

    ret = bmx280_init(bmx280);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2c_driver_install failed: %s", esp_err_to_name(ret));
        bmx_stat.status = -3;
    }
    bmx280_config_t bmx_cfg = BMX280_DEFAULT_CONFIG;
    ret = bmx280_configure(bmx280, &bmx_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2c_driver_install failed: %s", esp_err_to_name(ret));
        bmx_stat.status = -4;
    }
    if (!bmx_stat.status) {
        const esp_timer_create_args_t periodic_timer_args = {
            .callback = &periodic_timer_callback,
            .name = "periodic_bmx_timer",
            .arg = NULL
        };
        ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &bmx_periodic_timer));
        ESP_ERROR_CHECK(esp_timer_start_periodic(bmx_periodic_timer, 1000000000));
        //xTaskCreatePinnedToCore(bmx_task, "bmx_task", BMX_TASK_STACK_SIZE, NULL, 0, &t1, 1);
    }

    return ret;
}

void deinit_bmx() {
    LOGR;
    vTaskDelete(t1);
    bmx280_close(bmx280);
}

bmx_t *bmx_readings() {
    return &bmx_stat;
}

void bmx_readings_i(int32_t *temp, uint32_t *press, uint32_t *humid) {
    *temp = bmx_stat.temp;
    *press = bmx_stat.press;
    *humid = bmx_stat.humid;
}

void bmx_readings_f(float *temp, float *press, float *humid) {
    bmx280_readout2float(&bmx_stat.temp, &bmx_stat.press, &bmx_stat.humid, temp, press, humid);
}
#endif