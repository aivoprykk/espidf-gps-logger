#include "private.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "demo_display_lvgl";

struct display_s display;

display_op_t screen_ops = {
    .screen_cb = screen_cb,
    .ui_init = ui_init,
    .ui_deinit = ui_deinit,
};

struct display_s *lcd_init() {
    ESP_LOGI(TAG, "[%s]", __func__);
    display_init(&display, &screen_ops);
    return &display;
}

void lcd_deinit() {
    ESP_LOGI(TAG, "[%s]", __func__);
    display_uninit(&display);
}

void app_main(void)
{
    init_button();
    adc_init();
    lcd_init();
    display_task_start();
    display_task_pause();
    while (1) {
        // printf("LVGL tick handler running...\n");
        // raise the task priority of LVGL and/or reduce the handler period can improve the performance
        // The task running lv_timer_handler should have lower priority than that running `lv_tick_inc`
        display_task_resume_for_times(1, -1, -1, false);
        lv_timer_handler();
        print_lv_mem_mon();
        vTaskDelay(pdMS_TO_TICKS(5000));
    }

    lcd_deinit();
}
