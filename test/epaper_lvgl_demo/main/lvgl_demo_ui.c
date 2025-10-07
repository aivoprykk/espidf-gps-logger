
#include <stdio.h>
#include "private.h"

#if LVGL_VERSION_MAJOR >= 9
#include "../managed_components/lvgl__lvgl/src/core/lv_refr_private.h"
#endif

static const char *TAG = "lvgl_demo_ui";
static float voltage_bat = 0.0;
#define MINIMUM_VOLTAGE 3.2
#ifdef CONFIG_SSD168X_PANEL_SSD1680
LV_IMG_DECLARE(speed_raw_250x122);
LV_IMG_DECLARE(speed_raw_122x250);
const lv_img_dsc_t * img_full_l = &speed_raw_250x122;
const lv_img_dsc_t * img_full_p = &speed_raw_122x250;
#endif
#ifdef CONFIG_SSD168X_PANEL_SSD1681
LV_IMG_DECLARE(speed_raw_200x200);
const lv_img_dsc_t * img_full_l = &speed_raw_200x200;
const lv_img_dsc_t * img_full_p = &speed_raw_200x200;
#endif

static void update_bat(uint8_t verbose) {
    voltage_bat = volt_read();
#if defined(DEBUG)
    if (verbose)
        ESP_LOGI(TAG, "[%s] Battery measured (computed:%.02f, required_min:%.02f)\n", __FUNCTION__, voltage_bat, MINIMUM_VOLTAGE);
#endif
}

static int count = 0;
static float max_speed = 0, avg_speed = 0, cur_speed[5] = {0};
static uint8_t index_speed = 0;

enum screen_e {
    BLANK_SCREEN,
    SPLASH_SCREEN,
    SLEEP_SCREEN,
    RECORD_SCREEN,
    BOOT_SCREEN,
    GPS_SCREEN,
    SPEED_SCREEN,
    STATS_SCREEN_3x1,
    STATS_SCREEN_2x2,
    STATS_SCREEN_3x2,
    GPS_TROUBLE_SCREEN,
    LOW_BAT_SCREEN,
    MAX_SCREENS
};

static SemaphoreHandle_t refreshing_sem = NULL;

static bool button_down = false;
static int8_t rot = 0;

void button_cb(int num, l_button_ev_t ev, uint64_t press_time) {
#if defined(DEBUG)
    ESP_LOGI(TAG, "Button %d event: %d time: %llu us", num, ev, press_time);
#endif
    switch (ev) {
    case L_BUTTON_UP:
        if (xSemaphoreTake(refreshing_sem, portMAX_DELAY) == pdTRUE){
            button_down = false;
            screen_cb(0);
            xSemaphoreGive(refreshing_sem);
        }
        break;
    case L_BUTTON_DOWN:
        if (xSemaphoreTake(refreshing_sem, portMAX_DELAY) == pdTRUE){
            button_down = true;
            xSemaphoreGive(refreshing_sem);
        }
        break;
    case L_BUTTON_LONG_PRESS_START:
        if(num==0){
            if (xSemaphoreTake(refreshing_sem, portMAX_DELAY) == pdTRUE){
                display_set_rotation(rot++%4);
                xSemaphoreGive(refreshing_sem);
            }
        }
        break;
    case L_BUTTON_LONG_LONG_PRESS_START:
        --count;
        break;
    default:
        break;
    }
}

#include "esp_random.h"
float randomFloat() {
    uint32_t r = esp_random();               // Generate a random 32-bit number
    float f = (float)r / (float)UINT32_MAX;  // Convert to a float between 0 and 1
    return f * 127;                          // Scale to the range 0-127
}

int randomInteger() {
    uint32_t r = esp_random();  // Generate a random 32-bit number
    return r % 1271;            // Scale to the range 0-1500
}

float averageFloat(float *array, int count) {
    float sum = 0.0;
    for (int i = 0; i < count; i++) {
        sum += array[i];
    }
    return sum / count;
}

lv_obj_t * splashScreenLoad() {
    ESP_LOGI(TAG, "load splash screen");
    lv_obj_t * splash = ui_common_panel_init(NULL, 100, 100);
    lv_obj_t *img = lv_img_create(splash);
    lv_obj_set_size(img, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    if(rot == DISP_ROT_90 || rot == DISP_ROT_270) {
        lv_img_set_src(img, &speed_raw_122x250);
    } else {
        lv_img_set_src(img, &speed_raw_250x122);
    }
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_scr_load(splash);
    return splash;
}

lv_obj_t * blankScreenLoad(bool invert) {
    ESP_LOGI(TAG, "load blank screen with color %s", invert ? "black" : "white");
    lv_obj_t * panel = ui_common_panel_init(NULL, 100, 100);
    lv_obj_set_style_bg_color(panel, invert ? lv_color_black() : lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(panel, invert ? lv_color_white() : lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(panel, invert ? lv_color_white() : lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(panel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_scr_load(panel);
    return panel;
}

typedef struct sleep_scr_s {
    float data;
    const char *info;
} sleep_scr_t;

static struct sleep_scr_s sleep_scr_info_fields[2][6] = {
    {
        {100.49, "AV:"},
        {100.49, "R1:"},
        {100.49, "R2:"},
        {100.49, "R3:"},
        {100.49, "R4:"},
        {100.49, "R5:"}
    },
    {
        {101.25, "2sec:"},
        {54.16, "1h:"},
        {95.99, "500m:"},
        {94.81, "NM:"},
        {54.26, "Dist:"},
        {54.99, "Alfa:"}
    }
};
static lv_obj_t * scr = 0, *lscr = 0;

uint32_t screen_cb(void* arg) {
    update_bat(0);
    char tmp[24] = {0}, *p = tmp;
    // lv_obj_t *panel;
    printf("----------------------------------------\n");
    printf("load_screem: %d\n", count);
    printf("----------------------------------------\n");
    float last_speed = randomFloat();
    cur_speed[index_speed++ % 5] = last_speed;
    if (last_speed > max_speed)
        max_speed = last_speed;
    avg_speed = averageFloat(cur_speed, 5);
    if(scr)
        lscr = scr;
    if(button_down)
        return 0;
    
    if (count++ > MAX_SCREENS-2) {
        count = 0;
        rot = display_drv_get_rotation();
        display_drv_set_rotation(rot >= DISP_ROT_270 ? 0 : rot + 1); // to update lvgl disp_drv
    }

    if (count == SPLASH_SCREEN) {
        ESP_LOGI(TAG, "load splash screen");
        scr = splashScreenLoad();
    }
    else if(count == SLEEP_SCREEN) {
        ESP_LOGI(TAG, "load sleep screen");
        showSleepScreen();
        ui_status_panel_t * statusbar = &ui_status_panel;
        lv_label_set_text(statusbar->time_label, "12:00 2024-01-01");
        lv_label_set_text(statusbar->bat_label, "97%");
        for(int i = 0; i < 6; i++) {
            for(int j = 0; j < 2; j++) {
                f2_to_char(sleep_scr_info_fields[j][i].data, p);
                lv_label_set_text(ui_sleep_screen.cells[i][j].title, p);
                lv_label_set_text(ui_sleep_screen.cells[i][j].info, sleep_scr_info_fields[j][i].info);
            }
        }
    }
    else if (count == RECORD_SCREEN) {
        ESP_LOGI(TAG, "load record screen");
        showRecordScreen(0);
    }
    else if(count == BOOT_SCREEN) {
        ESP_LOGI(TAG, "load boot screen");
         showBootScreen("Booting");
    }
    else if(count == GPS_SCREEN) {
        ESP_LOGI(TAG, "load gps screen");
        showGpsScreen(0);
        set_label_text_safe(ui_info_screen.info_rows[0], "GPS", 0);
        set_label_text_safe(ui_info_screen.info_rows[1], "gps data row 1", 0);
        set_label_text_safe(ui_info_screen.info_rows[2], "gps data row 2", 0);
        set_label_text_safe(ui_info_screen.info_rows[3], "gps data row 3", 1);
    }
    else if(count == SPEED_SCREEN){
        ESP_LOGI(TAG, "load speed screen");
        showSpeedScreen();
        ui_status_panel_t * statusbar = &ui_status_panel;
        f2_to_char(voltage_bat, p);
        lv_label_set_text(statusbar->bat_label, p);
        if(count % 3 == 0) {
            f2_to_char(last_speed, p);
                lv_label_set_text(ui_speed_screen.speed, p);
                f2_to_char(max_speed, p);
                lv_label_set_text(ui_speed_screen.cells[0][0].title, p);
                f2_to_char(avg_speed, p);
                lv_label_set_text(ui_speed_screen.cells[0][1].title, p);
        }
        else {
                lv_label_set_text(ui_speed_screen.speed, "0.00");
                f2_to_char(max_speed, p);
                lv_label_set_text(ui_speed_screen.cells[0][0].title, "0.00");
                f2_to_char(avg_speed, p);
                lv_label_set_text(ui_speed_screen.cells[0][1].title, "0.00");
        }
    }
    
    else if(count == STATS_SCREEN_3x1) {
        ESP_LOGI(TAG, "load stats screen");
        loadStatsScreen(3,1);
        f2_to_char(last_speed, p);
        lv_label_set_text(ui_stats_screen.cells[0][0].title, p);
        lv_label_set_text(ui_stats_screen.cells[0][0].info, "500M");
    }

    else if(count == STATS_SCREEN_2x2) {
        ESP_LOGI(TAG, "load stats screen");
        loadStatsScreen(2,2);
        f2_to_char(last_speed, p);
        lv_label_set_text(ui_stats_screen.cells[0][0].title, p);
        lv_label_set_text(ui_stats_screen.cells[0][0].info, "MILE");
    }

    else if(count == STATS_SCREEN_3x2) {
        ESP_LOGI(TAG, "load stats screen");
        loadStatsScreen(3,2);
        f2_to_char(last_speed, p);
        lv_label_set_text(ui_stats_screen.cells[0][0].title, p);
        lv_label_set_text(ui_stats_screen.cells[0][0].info, "AVG");
    }

    else if(count == GPS_TROUBLE_SCREEN) {
        ESP_LOGI(TAG, "load gps trouble screen");
        showGpsTroubleScreen();
    }
    else if(count == LOW_BAT_SCREEN) {
        ESP_LOGI(TAG, "load low battery screen");
         showLowBatScreen(0);
    }
    else if(count == BLANK_SCREEN) {
        ESP_LOGI(TAG, "load blank screen");
        scr = blankScreenLoad((count & 1));
    }
    else {
        goto done;
    }

   if(lscr) {
        ESP_LOGI(TAG, "delete scr");
        if(!scr) {
            ESP_LOGI(TAG, "ups, !scr");
            scr = lscr;
        }
        else {
            if(scr == lscr) scr = 0;
            lv_obj_del(lscr);
            lscr = 0;
        }
    }

    done:
    print_lv_mem_mon();
    _lv_disp_refr_timer(NULL);
    return 5000;
}

void init_button() {
#if (C_LOG_LEVEL < 3)
    ILOG(TAG, "[%s]", __func__);
#endif
    button_init();
    btns[0].cb = button_cb;
#if defined(CONFIG_LOGGER_BUTTON_GPIO_1)
    btns[1].cb = button_cb;
#endif
    refreshing_sem = xSemaphoreCreateBinary();
    if (!refreshing_sem) {
        ESP_LOGE(TAG, "Failed to create semaphore");
        return;
    }
    xSemaphoreGive(refreshing_sem);
}

void deinit_button() {
#if (C_LOG_LEVEL < 3)
    ILOG(TAG, "[%s]", __func__);
#endif
    button_deinit();
    if (refreshing_sem) {
        vSemaphoreDelete(refreshing_sem);
        refreshing_sem = NULL;
    }
}