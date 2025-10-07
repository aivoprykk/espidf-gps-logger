
#include "ui.h"
//#include "logger_common.h"

ui_blank_screen_t ui_blank_screen = {0};

static lv_obj_t * load(lv_obj_t *parent) {
    
    lv_obj_t *panel = ui_common_panel_init(parent, 100, 100);
    lv_obj_set_style_bg_color(panel, ui_blank_screen.invert ? lv_color_black() : lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    return panel;
}

static void unload(void) {
    if(ui_blank_screen.screen.main_cnt == 0) return;
    lv_obj_clean(ui_blank_screen.screen.main_cnt);
    lv_obj_del(ui_blank_screen.screen.main_cnt);
    ui_blank_screen.screen.main_cnt = 0;
}

void ui_BlankScreen_screen_init(void) {
    if(!ui_blank_screen.screen.self){
        ui_blank_screen.screen.load = load;
        ui_blank_screen.screen.unload = unload;
        ui_blank_screen.screen.has_status_cnt = 0;
        ui_common_screen_init(&ui_blank_screen.screen);
    }
    ui_flush_screens(&ui_blank_screen.screen);
    if(ui_blank_screen.screen.main_cnt == 0)
        ui_blank_screen.screen.main_cnt = load(ui_blank_screen.screen.self);
    lv_obj_set_x(ui_blank_screen.screen.main_cnt, lv_pct(ui_blank_screen.screen.main_cnt_offset));
}
