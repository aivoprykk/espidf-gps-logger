
#include "ui.h"

ui_record_screen_t ui_record_screen = {0};
// static const char * TAG = "ui_record_screen";

static lv_obj_t * load(lv_obj_t *parent) {
    lv_obj_t *panel = ui_common_panel_init(parent, 100, 100);
    
    lv_obj_set_style_text_font(panel, ui_record_screen.font.title, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *label;
    label = lv_label_create(panel);
    lv_label_set_text(label, "10sec");
    lv_obj_align(label, LV_ALIGN_CENTER, lv_pct(-35), lv_pct(-2));
    lv_obj_set_style_text_font(label, ui_record_screen.font.info, LV_PART_MAIN | LV_STATE_DEFAULT);
    ui_record_screen.info_lbl = label;
    
    label = lv_label_create(panel);
    lv_label_set_text(label, "109.05");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, lv_pct(-14));
    ui_record_screen.cur_lbl = label;

    label = lv_label_create(panel);
    lv_label_set_text(label, "105.72");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, lv_pct(14));
    ui_record_screen.prev_lbl = label;

    return panel;
}

static void unload(void) {
    if(ui_record_screen.screen.main_cnt == 0) return;
    lv_obj_clean(ui_record_screen.screen.main_cnt);
    lv_obj_del(ui_record_screen.screen.main_cnt);
    ui_record_screen.screen.main_cnt = 0;
    ui_record_screen.prev_lbl = 0;
    ui_record_screen.cur_lbl = 0;
    ui_record_screen.info_lbl = 0;
}

void ui_RecordScreen_screen_init(void) {
    if(!ui_record_screen.screen.self){
        ui_record_screen.screen.load = load;
        ui_record_screen.screen.unload = unload;
        ui_record_screen.screen.has_status_cnt = 0;
        ui_common_screen_init(&ui_record_screen.screen);
    }
    ui_flush_screens(&ui_record_screen.screen);
    if(ui_record_screen.screen.main_cnt == 0)
        ui_record_screen.screen.main_cnt = load(ui_record_screen.screen.self);
}
