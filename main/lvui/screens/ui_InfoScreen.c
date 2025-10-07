
#include "ui.h"
#include "driver_vendor.h"
#include "logger_common.h"
#include "ui_common.h"

ui_info_screen_t ui_info_screen = {0};
// static const char * TAG = "ui_info_screen";

#ifdef CONFIG_SSD168X_PANEL_SSD1681
static lv_coord_t col_2_x = 33, col_1_x = 4;
#elif defined(CONFIG_SSD168X_PANEL_SSD1680)
static lv_coord_t col_2_x = 27, col_1_x = 6;
#else
static lv_coord_t col_2_x = 35, col_1_x = 12;
#endif
#if defined(CONFIG_SSD168X_PANEL_SSD1680)
#define SCR_H 128
#else
#define SCR_H 170
#endif
static int get_pct(int i, int count_labels, bool is_not_l) {
#ifndef CONFIG_SSD168X_PANEL_SSD1681
    if(is_not_l) {
        if(count_labels > 3)
            return (i==UI_INFO_SCREEN_TITLE_LBL) ? -10 : (i==UI_INFO_SCREEN_ROW_2_LBL) ? 5 : (i==UI_INFO_SCREEN_ROW_3_LBL) ? 15 : 25;
        else
            return (i==UI_INFO_SCREEN_TITLE_LBL) ? -10 : (i==UI_INFO_SCREEN_ROW_2_LBL) ? 5 : 15;
    }
#else 
    UNUSED_PARAMETER(is_not_l);
#endif
    if(count_labels > 3) {
#ifdef CONFIG_SSD168X_PANEL_SSD1681
        return (i==UI_INFO_SCREEN_TITLE_LBL) ? -19 : (i==UI_INFO_SCREEN_ROW_2_LBL) ? 0 : (i==UI_INFO_SCREEN_ROW_3_LBL) ? 15 : 30;
#else
        return (i==UI_INFO_SCREEN_TITLE_LBL) ? -20 : (i==UI_INFO_SCREEN_ROW_2_LBL) ? 4 : (i==UI_INFO_SCREEN_ROW_3_LBL) ? 22 : 40;
#endif
    } else {
#ifdef CONFIG_SSD168X_PANEL_SSD1681
        return (i==UI_INFO_SCREEN_TITLE_LBL) ? -14 : (i==UI_INFO_SCREEN_ROW_2_LBL) ? 5 : 20;
#else
        return (i==UI_INFO_SCREEN_TITLE_LBL) ? -14 : (i==UI_INFO_SCREEN_ROW_2_LBL) ? 11 : 30;
#endif
    }
}

static void update_dims() {
    bool is_l = (display_drv_get_width()>SCR_H);
    lv_obj_t *obj;
    int i=0, count_labels = ui_info_screen.info_screen_label_count + 1 > UI_INFO_SCREEN_ROWS ? UI_INFO_SCREEN_ROWS : ui_info_screen.info_screen_label_count + 1;
    for (; i < count_labels; ++i) {
        obj = ui_info_screen.info_rows[i];
        // r1 - 25 - r2 -19 - r3 - 19 - r4
        if(is_l) {
            lv_obj_set_x(obj, lv_pct(col_2_x));
        }
        else {
            lv_obj_set_x(obj, lv_pct(col_1_x));
        }
        lv_obj_set_y(obj, lv_pct(get_pct(i, count_labels, !is_l)));
    }
    obj = ui_info_screen.info_img;
    if(obj) {
        if(is_l)
            lv_obj_align(obj, LV_ALIGN_LEFT_MID, lv_pct(col_1_x), 1);
        else
            lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, lv_pct(5));
    }
    // ui_status_panel_update_dims(&ui_info_screen.screen);
}

static lv_obj_t * load(lv_obj_t *parent) {
    lv_obj_t *panel = ui_common_panel_init(parent, 100, 80);

    lv_obj_t *ui_MainImage = lv_img_create(panel);
    lv_img_set_src(ui_MainImage, &near_me_bold_48px);
    lv_obj_set_width(ui_MainImage, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_MainImage, LV_SIZE_CONTENT);  /// 1
    lv_obj_align(ui_MainImage, LV_ALIGN_LEFT_MID, lv_pct(col_1_x), 1);
    lv_obj_add_flag(ui_MainImage, LV_OBJ_FLAG_ADV_HITTEST);   /// Flags
    lv_obj_clear_flag(ui_MainImage, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
    ui_info_screen.info_img = ui_MainImage;

    lv_obj_t *label;
    
    int i=0, count_labels = ui_info_screen.info_screen_label_count + 1 > UI_INFO_SCREEN_ROWS ? UI_INFO_SCREEN_ROWS : ui_info_screen.info_screen_label_count + 1;
    for (; i < count_labels; ++i) {
        // r1 - 25 - r2 -19 - r3 - 19 - r4
        if(ui_info_screen.info_rows[i]) {
            label = ui_info_screen.info_rows[i];
        }
        else {
            label = lv_label_create(panel);
            lv_obj_set_width(label, LV_SIZE_CONTENT);   /// 1
            lv_obj_set_height(label, LV_SIZE_CONTENT);  /// 1
        }
        lv_obj_align(label, LV_ALIGN_LEFT_MID, lv_pct(col_2_x), lv_pct(get_pct(i, count_labels, 1)));
        if(i == 0)
            lv_obj_set_style_text_font(label, ui_info_screen.font.title, LV_PART_MAIN | LV_STATE_DEFAULT);
        else
            lv_obj_set_style_text_font(label, ui_info_screen.font.info, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(label, "");
        ui_info_screen.info_rows[i] = label;
    }
    // update_dims();
    return panel;
}

static void unload(void) {
    if(ui_info_screen.screen.main_cnt == 0) return;
    lv_obj_clean(ui_info_screen.screen.main_cnt);
    lv_obj_del(ui_info_screen.screen.main_cnt);
    ui_info_screen.screen.main_cnt = 0;
    for (int i = 0; i < UI_INFO_SCREEN_ROWS; ++i) {
        ui_info_screen.info_rows[i] = 0;
    }
    ui_info_screen.info_img = 0;
}

int ui_InfoScreen_screen_init(int rows) {
    int ret = 0;
    if(!ui_info_screen.screen.self){
        ui_info_screen.screen.has_status_cnt = 1;
        // ui_info_screen.screen.status_viewmode = STATUS_VIEWMODE_DEFAULT;
        ui_info_screen.screen.load = load;
        ui_info_screen.screen.unload = unload;
        ui_info_screen.screen.update_dims = update_dims;
        ui_info_screen.screen.self = ui_common_screen_init(&ui_info_screen.screen);
    }
    ui_flush_screens(&ui_info_screen.screen);
    if(rows != ui_info_screen.info_screen_label_count) {
        unload();
    }
    if(ui_info_screen.screen.main_cnt == 0){
        ui_info_screen.info_screen_label_count = rows;
        ui_info_screen.screen.main_cnt = load(ui_info_screen.screen.self);
        ret = 1;
    }
    lv_obj_set_x(ui_info_screen.screen.main_cnt, lv_pct(ui_info_screen.screen.main_cnt_offset));
    ui_status_panel_load(&ui_info_screen.screen, STATUS_VIEWMODE_DEFAULT);
    update_dims();
    return ret;
}
