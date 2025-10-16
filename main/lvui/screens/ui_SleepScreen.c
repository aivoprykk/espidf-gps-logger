
#include "ui.h"
#include "driver_vendor.h"
#include <string.h>

ui_sleep_screen_t ui_sleep_screen = {0};
// static const char * TAG = "ui_sleep_screen";
// order is defined in config module
const lv_img_dsc_t * const sail_logo_img [] = {&ga_logo_n_48px, &duotone_48px, &np_48px, &loftsails_48px, &gunsails_48px, &p7_black_48px, &patrik_48px};
const lv_img_dsc_t * const board_logo_img [] = {&sb_48px, &fanatic_48px, &jp_48px, &patrik_48px};

static void update_dims() {
#if !defined(CONFIG_LCD_IS_EPD)
    bool is_l = (display_drv_get_width()>170);
#else
    bool is_l = (display_drv_get_width()>128);
#endif
    lv_obj_t *obj = ui_sleep_screen.myid;
    // if(obj) {
    //     if(is_l)
    //         lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 0, lv_pct(84));
    //     else
    //         lv_obj_align(obj, LV_ALIGN_TOP_LEFT, 0, lv_pct(100));
    // }
    obj = ui_sleep_screen.up_img;
    // if(obj) {
    //     if(is_l)
    //         lv_obj_align(obj, LV_ALIGN_CENTER, 0, -24);
    //     else
    //         lv_obj_align(obj, LV_ALIGN_CENTER, 0, -20);
    // }
    obj = ui_sleep_screen.bottom_img;
    // if(obj) {
    //     if(is_l)
    //         lv_obj_align(obj, LV_ALIGN_CENTER, 0, 27);
    //     else
    //         lv_obj_align(obj, LV_ALIGN_CENTER, 0, 20);
    // }
    for (int i = 0; i < 6; ++i) {
        obj = lv_obj_get_parent(ui_sleep_screen.cells[i][0].self);
        if(is_l) {
            lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
        } else {
            lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
        }

        for (int j = 0; j < 2; ++j) {
            obj = ui_sleep_screen.cells[i][j].self;
            if(obj) {
                if(is_l){
                    lv_obj_set_width(obj, lv_pct(j==0 ? 35 : 46));
                    lv_obj_set_height(obj, lv_pct(100));
                    if(j>0)
                        lv_obj_set_style_pad_left(ui_sleep_screen.cells[i][j].info, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
                } else{
                    lv_obj_set_width(obj, lv_pct(86));
                    lv_obj_set_height(obj, lv_pct(50));
                    if(j>0)
                        lv_obj_set_style_pad_left(ui_sleep_screen.cells[i][j].info, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
            }
        }
    }
    // ui_status_panel_update_dims(&ui_sleep_screen.screen);
}

static lv_obj_t * ui_Cell(lv_obj_t *cnt, int w, int h, int col, ui_cell_t *cell) {
    lv_obj_t * panel = ui_common_panel_init(cnt, w, h);
    cell->self = panel;
    // lv_obj_t *info_cnt =  ui_common_panel_init(panel, wi, 100);
    // lv_obj_set_align(info_cnt, LV_ALIGN_LEFT_MID);
    
    lv_obj_t *info_lbl = lv_label_create(panel);
    lv_obj_set_width(info_lbl, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(info_lbl, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_align(info_lbl, LV_ALIGN_LEFT_MID);
    lv_label_set_text(info_lbl, "R5:");
    if(col>0)
        lv_obj_set_style_pad_left(info_lbl, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    cell->info = info_lbl;

    // lv_obj_t *title_cnt = ui_common_panel_init(panel, wt, 100);
    // lv_obj_set_align(title_cnt, LV_ALIGN_RIGHT_MID);
    
    lv_obj_t *title_lbl = lv_label_create(panel);
    lv_obj_set_width(title_lbl, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(title_lbl, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_align(title_lbl, LV_ALIGN_RIGHT_MID);
    lv_label_set_text(title_lbl, "109.09");
    cell->title = title_lbl;

    return panel;
}

static lv_obj_t *load(lv_obj_t *parent) {
    lv_obj_t *panel = ui_common_panel_init(parent, 100, 83);

    lv_obj_set_style_pad_left(panel, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(panel, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
#ifdef CONFIG_SSD168X_PANEL_SSD1681
    lv_obj_t *right_container =  ui_common_panel_init(panel, 20, 100);
#else
    lv_obj_t *right_container =  ui_common_panel_init(panel, 30, 100);
#endif
    lv_obj_set_align(right_container, LV_ALIGN_TOP_RIGHT);
    
    lv_obj_t *up_img = lv_img_create(right_container);
    lv_img_set_src(up_img, board_logo_img[0]);
    lv_obj_set_width(up_img, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(up_img, LV_SIZE_CONTENT);  /// 1
    lv_obj_align(up_img, LV_ALIGN_CENTER, 0, -24);
    // lv_obj_add_flag(up_img, LV_OBJ_FLAG_ADV_HITTEST);   /// Flags
    lv_obj_clear_flag(up_img, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
    ui_sleep_screen.up_img = up_img;

    lv_obj_t *bottom_img = lv_img_create(right_container);
    lv_img_set_src(bottom_img, sail_logo_img[0]);
    lv_obj_set_width(bottom_img, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(bottom_img, LV_SIZE_CONTENT);  /// 1
    lv_obj_align(bottom_img, LV_ALIGN_CENTER, 0, 27);
    // lv_obj_add_flag(bottom_img, LV_OBJ_FLAG_ADV_HITTEST);   /// Flags
    lv_obj_clear_flag(bottom_img, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
    ui_sleep_screen.bottom_img = bottom_img;

#ifdef CONFIG_SSD168X_PANEL_SSD1681
    lv_obj_t *left_container =  ui_common_panel_init(panel, 90, 100);
#else
    lv_obj_t *left_container =  ui_common_panel_init(panel, 80, 100);
#endif
    lv_obj_set_style_text_font(left_container, ui_sleep_screen.font.normal, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *rows[6]={0}; // , *cols[2]={0};
    int numrows = 6, h = 100/(numrows+1);
    for (int i = 0, ypos = 0; i < numrows; ++i, ypos += h) {


        rows[i] = ui_common_panel_init(left_container, 100, 0);
        lv_obj_set_x(rows[i], 0);
        lv_obj_clear_flag(rows[i], LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_height(rows[i], lv_pct(h));
        lv_obj_set_y(rows[i], lv_pct(ypos));
        lv_obj_set_flex_flow(rows[i], LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(rows[i], LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);

        for (int j = 0; j < 2; ++j) {
            ui_Cell(rows[i], j==0 ? 35 : 46, 100, j, &ui_sleep_screen.cells[i][j]);
        }
    }
    lv_obj_t *title_lbl = lv_label_create(left_container);
    lv_obj_align(title_lbl, LV_ALIGN_TOP_LEFT, 0, lv_pct(84));
    lv_obj_set_width(title_lbl, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(title_lbl, LV_SIZE_CONTENT);  /// 1
    lv_label_set_text(title_lbl, "#MY ID");
    lv_obj_set_style_text_font(title_lbl, ui_sleep_screen.font.title, LV_PART_MAIN | LV_STATE_DEFAULT);
    ui_sleep_screen.myid = title_lbl;
    // update_dims();
    return panel;
}

static void unload() {
#if (C_LOG_LEVEL < 1)
    printf("[%s] ui_sleep_screen main_cnt\n", __func__);
#endif
    if(ui_sleep_screen.screen.main_cnt == NULL) return;
    lv_obj_clean(ui_sleep_screen.screen.main_cnt);
    lv_obj_del(ui_sleep_screen.screen.main_cnt);
    ui_sleep_screen.screen.main_cnt = NULL;
    ui_sleep_screen.myid = NULL;
    ui_sleep_screen.up_img = NULL;
    ui_sleep_screen.bottom_img = NULL;
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 2; ++j) {
            memset(&ui_sleep_screen.cells[i][j], 0, sizeof(ui_cell_t));
        }
    }
}

int ui_SleepScreen_screen_init(void) {
    int ret = 0;
    if(!ui_sleep_screen.screen.self) {
        ui_sleep_screen.screen.has_status_cnt = 1;
        // ui_sleep_screen.screen.status_viewmode = STATUS_VIEWMODE_SLEEP;
        ui_sleep_screen.screen.load = load;
        ui_sleep_screen.screen.unload = unload;
        ui_sleep_screen.screen.update_dims = update_dims;
        ui_common_screen_init(&ui_sleep_screen.screen);
    }
    ui_flush_screens(&ui_sleep_screen.screen);
    if(ui_sleep_screen.screen.main_cnt == NULL) {
        ui_sleep_screen.screen.main_cnt = load(ui_sleep_screen.screen.self);
        ret = 1;
    }
    ui_status_panel_load(&ui_sleep_screen.screen, STATUS_VIEWMODE_SLEEP);
    update_dims();
    return ret;
}