
#include "ui.h"
//#include "logger_common.h"

ui_init_screen_t ui_init_screen = {0};
// static const char * TAG = "ui_init_screen";

static lv_obj_t * load(lv_obj_t *parent) {
    
    lv_obj_t *panel = ui_common_panel_init(parent, 100, 100);
    
    lv_obj_t *ui_InitLabel = lv_label_create(panel);
    lv_obj_set_width(ui_InitLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_InitLabel, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_x(ui_InitLabel, 0);
    lv_obj_set_y(ui_InitLabel, LV_PCT(-5));
    lv_obj_set_align(ui_InitLabel, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_text_font(ui_InitLabel, ui_init_screen.font.title, LV_PART_MAIN | LV_STATE_DEFAULT);
    ui_init_screen.init_lbl = ui_InitLabel;

    lv_obj_t *ui_InitImage = lv_img_create(panel);
    lv_img_set_src(ui_InitImage, &espidf_logo_v2_48px);
    lv_obj_set_width(ui_InitImage, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_InitImage, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_x(ui_InitImage, 0);
    lv_obj_set_y(ui_InitImage, lv_pct(-10));
    lv_obj_set_align(ui_InitImage, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_InitImage, LV_OBJ_FLAG_ADV_HITTEST);   /// Flags
    lv_obj_clear_flag(ui_InitImage, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
    ui_init_screen.init_img = ui_InitImage;

    return panel;
}

static void unload(void) {
    if(ui_init_screen.screen.main_cnt == 0) return;
    lv_obj_clean(ui_init_screen.screen.main_cnt);
    lv_obj_del(ui_init_screen.screen.main_cnt);
    ui_init_screen.screen.main_cnt = 0;
    ui_init_screen.init_lbl = 0;
    ui_init_screen.init_img = 0;
}

int ui_InitScreen_screen_init(int reload) {
    int ret = 0;
    if(!ui_init_screen.screen.self){
        ui_init_screen.screen.load = load;
        ui_init_screen.screen.unload = unload;
        ui_init_screen.screen.has_status_cnt = 0;
        ui_common_screen_init(&ui_init_screen.screen);
    }
    ui_flush_screens(&ui_init_screen.screen);
    if(reload) unload();
    if(ui_init_screen.screen.main_cnt == 0) {
        ui_init_screen.screen.main_cnt = load(ui_init_screen.screen.self);
        ret = 1;
    }
    lv_obj_set_x(ui_init_screen.screen.main_cnt, lv_pct(ui_init_screen.screen.main_cnt_offset));
    return ret;
}
