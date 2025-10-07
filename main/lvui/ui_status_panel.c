#include "ui_private.h"
#include "../main/private.h"
#include "driver_vendor.h"

static const char *TAG = "ui_status_panel";

ui_status_panel_t ui_status_panel = {0};

#if defined(USE_2BPP_FONT)
const lv_font_t * ui_status_font_default = &ui_font_OswaldRegular20p2;
const lv_font_t * ui_status_font_default_portrait = &ui_font_OswaldRegular14p2;
#elif defined(USE_1BPP_FONT)
const lv_font_t * ui_status_font_default = &ui_font_OswaldRegular20p1;
const lv_font_t * ui_status_font_default_portrait = &ui_font_OswaldRegular14p1;
#else
const lv_font_t * ui_status_font_default = &ui_font_OswaldRegular20p4;
const lv_font_t * ui_status_font_default_portrait = &ui_font_OswaldRegular14p4;
#endif

void ui_status_panel_rearrange(ui_screen_t *parent) {
    if(!parent) return;
    ILOG(TAG, "[%s] vm:%hhu", __func__,ui_status_panel.viewmode);
    if (ui_status_panel.self == NULL) return;
    lv_obj_t *l;
    if (ui_status_panel.viewmode == STATUS_VIEWMODE_DEFAULT)  {
        l = ui_status_panel.bat_image;
        if(lv_obj_has_flag(l, LV_OBJ_FLAG_HIDDEN)){
            lv_obj_clear_flag(l, LV_OBJ_FLAG_HIDDEN);
        }
    }
    else {
        l = ui_status_panel.bat_image;
        if(!lv_obj_has_flag(l, LV_OBJ_FLAG_HIDDEN))
            lv_obj_add_flag(l, LV_OBJ_FLAG_HIDDEN);
    }
    l = ui_status_panel.gps_image;
    if (ui_status_panel.viewmode == STATUS_VIEWMODE_DEFAULT || ui_status_panel.viewmode == STATUS_VIEWMODE_SPEED) {
        if(lv_obj_has_flag(l, LV_OBJ_FLAG_HIDDEN)){
            lv_obj_clear_flag(l, LV_OBJ_FLAG_HIDDEN);
        }
        l = ui_status_panel.sat_info_label;
        if(lv_obj_has_flag(l, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_clear_flag(l, LV_OBJ_FLAG_HIDDEN);
        }
    }
    else {
        if(!lv_obj_has_flag(l, LV_OBJ_FLAG_HIDDEN))
            lv_obj_add_flag(l, LV_OBJ_FLAG_HIDDEN);
        l = ui_status_panel.sat_info_label;
        if(!lv_obj_has_flag(l, LV_OBJ_FLAG_HIDDEN))
            lv_obj_add_flag(l, LV_OBJ_FLAG_HIDDEN);

    }
    ui_status_panel_update_dims(parent);
}

void ui_status_panel_update_dims(ui_screen_t *parent) {
    bool is_l = (display_drv_get_width() > SCR_H);
    ILOG(TAG, "[%s] l:%hhu", __func__,is_l);
    lv_obj_t *obj = ui_status_panel.self;
    if(obj) {
        // lv_obj_set_style_border_width(parent->main_cnt, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_text_color(parent->main_cnt, lv_color_hex(0x00000), LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_border_width(parent->status_cnt, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_text_color(parent->status_cnt, lv_color_hex(0x00000), LV_PART_MAIN | LV_STATE_DEFAULT);

        if(is_l) { /// landscape || ssd1681
            lv_obj_set_height(parent->main_cnt, lv_pct((ui_status_panel.viewmode == STATUS_VIEWMODE_DEFAULT) ? MAIN_CNT_H : MAIN_CNT_H_SM));
            lv_obj_set_height(parent->status_cnt, lv_pct((ui_status_panel.viewmode == STATUS_VIEWMODE_DEFAULT) ? STATUS_CNT_H : STATUS_CNT_H_SM));
            lv_obj_set_style_text_font(obj, parent->status_font ? parent->status_font : ui_status_font_default, LV_PART_MAIN | LV_STATE_DEFAULT);
// #ifdef CONFIG_SSD168X_PANEL_SSD1680
//             lv_obj_set_y(parent->status_cnt, -1); // for SSD1680, visible height is 128-6=122
// #endif
            obj = ui_status_panel.recoding_image;
            if(obj && !lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) {
#ifndef CONFIG_SSD168X_PANEL_SSD1681
                lv_obj_set_x(obj, lv_pct(-45));
#else
                lv_obj_set_x(obj, lv_pct(-49));
#endif
            }
            obj = ui_status_panel.sat_info_label;
            if(obj && !lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_set_style_text_font(obj, parent->status_font_portrait ? parent->status_font_portrait : ui_status_font_default_portrait, LV_PART_MAIN | LV_STATE_DEFAULT);
#ifdef CONFIG_SSD168X_PANEL_SSD1681
                if (parent && ui_status_panel.viewmode == STATUS_VIEWMODE_SPEED) {
                    lv_obj_set_x(obj, lv_pct(22));
                }
                else {
                    lv_obj_set_x(obj, lv_pct(25));
                }
#endif
            }
            obj = ui_status_panel.gps_image;
            if(obj && !lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) {
#ifndef CONFIG_SSD168X_PANEL_SSD1681
                lv_obj_set_x(obj, lv_pct(-35));
#else
                lv_obj_set_x(obj, lv_pct(-38));
#endif
            }
            obj = ui_status_panel.bat_image;
            if(obj && !lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)){
#ifndef CONFIG_SSD168X_PANEL_SSD1681
                lv_obj_set_x(obj, lv_pct(-20));
#else
                lv_obj_set_x(obj, lv_pct(-25));
#endif
            }
        }
#ifndef CONFIG_SSD168X_PANEL_SSD1681
        else { /// portrait
            lv_obj_set_height(parent->main_cnt, lv_pct(MAIN_CNT_H_P));
            lv_obj_set_height(parent->status_cnt, lv_pct(STATUS_CNT_H_P));
#ifdef CONFIG_SSD168X_PANEL_SSD1680
            lv_obj_set_y(parent->status_cnt, 0); // for SSD1680, visible height is 128-6=122
#endif
            lv_obj_set_style_text_font(obj, parent->status_font_portrait ? parent->status_font_portrait : ui_status_font_default_portrait, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_x(ui_status_panel.recoding_image, lv_pct(-62));
            lv_obj_set_x(ui_status_panel.gps_image, lv_pct(-50));
            lv_obj_set_x(ui_status_panel.bat_image, lv_pct(-28));
        }
#endif
    }
}

void ui_status_panel_init(ui_screen_t *parent) {
    ILOG(TAG, "[%s]", __func__);
    if ( ui_status_panel.self != NULL)
        return;
#if defined(STATUS_PANEL_V1)
    ui_status_panel_create(parent->status_cnt);
#else
    ui_status_panel.self = lv_statusbar_create(parent->status_cnt);
#endif
    lv_obj_t * obj = ui_status_panel.self;
    lv_obj_set_x(obj, 0);
    lv_obj_set_y(obj, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(obj, lv_color_hex(0x0D0D0D), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(obj, lv_color_hex(0x0D0D0D), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(obj, parent->status_font ? parent->status_font : ui_status_font_default, LV_PART_MAIN | LV_STATE_DEFAULT);
    ui_status_panel_rearrange(parent);
}

void ui_status_panel_load(ui_screen_t* parent, enum status_viewmode_e viewmode) {
    ILOG(TAG, "[%s]", __func__);
    if(!parent->has_status_cnt)
        return;
    // if(ui_status_panel.self != NULL && (parent->status_viewmode != ui_status_panel.viewmode)) {
    //     ui_status_panel_delete();
    // }
    ui_status_panel.viewmode = viewmode;
    if (ui_status_panel.self == NULL)
        ui_status_panel_init(parent);
    else {
        ui_status_panel_rearrange(parent);
        lv_obj_set_parent(ui_status_panel.self, parent->status_cnt);
    }
    ui_status_panel.parent = parent;
    lv_obj_set_x(ui_status_panel.self, parent->main_cnt_offset);
}

lv_obj_t* ui_status_panel_create(lv_obj_t* parent) {
    lv_obj_t *panel, *l;
    panel = ui_common_panel_init(parent, 100, 100);
    lv_obj_set_align(panel, LV_ALIGN_BOTTOM_LEFT);
    // if (!ui_status_panel.viewmode) {
    //     lv_obj_set_style_border_width(panel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    //     lv_obj_set_style_border_side(panel, LV_BORDER_SIDE_TOP, LV_PART_MAIN | LV_STATE_DEFAULT);
    // }
    lv_obj_set_style_pad_left(panel, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(panel, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(panel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    ui_status_panel.self = panel;

    l = lv_label_create(panel);
    lv_obj_set_width(l, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(l, LV_SIZE_CONTENT);  /// 1
    lv_obj_align(l, LV_ALIGN_LEFT_MID, 0, 0);
    lv_label_set_text(l, ""); // time
    ui_status_panel.time_label = l;
    
    l = lv_label_create(panel);
    lv_obj_add_flag(l, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_width(l, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(l, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_align(l, LV_ALIGN_LEFT_MID);
    lv_obj_set_y(l, 0);
    lv_obj_set_x(l, lv_pct(30));
    lv_label_set_text(l, "");
    ui_status_panel.sat_info_label = l;

    l = lv_obj_create(panel);
    lv_obj_add_flag(l, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_align(l, LV_ALIGN_RIGHT_MID);
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_radius(&style, LV_RADIUS_CIRCLE);
    lv_style_set_bg_color(&style, lv_color_hex(0xE32424));
    lv_obj_add_style(l, &style, 0);
    lv_obj_set_size(l, 6, 6);
    lv_obj_set_y(l, 0);
    ui_status_panel.recoding_image = l;

    l = lv_label_create(panel);
    lv_obj_add_flag(l, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_width(l, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(l, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_align(l, LV_ALIGN_RIGHT_MID);
    lv_obj_set_y(l, 0);
    lv_label_set_text(l, ""); // gps
    //     lv_obj_set_style_text_font(l, LV_FONT_DEFAULT, 0);
    //     lv_label_set_text(l, LV_SYMBOL_GPS);
    //     // lv_obj_set_style_pad_top(l, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_pad_right(l, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    ui_status_panel.gps_image = l;

//     l = lv_label_create(panel);
//     lv_obj_set_width(l, LV_SIZE_CONTENT);   /// 1
//     lv_obj_set_height(l, LV_SIZE_CONTENT);  /// 1
//     lv_obj_align(l, LV_ALIGN_RIGHT_MID, lv_pct(-28), 0);
//     lv_obj_set_style_text_font(l, LV_FONT_DEFAULT, 0);
//     lv_label_set_text(l, LV_SYMBOL_SD_CARD);
//     lv_obj_set_style_pad_right(l, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
//     // lv_obj_set_style_pad_top(l, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
//     ui_status_panel.sdcard_image = l;

    l = lv_label_create(panel);
    lv_obj_add_flag(l, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_width(l, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(l, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_align(l, LV_ALIGN_RIGHT_MID);
    lv_obj_set_y(l, 0);
    lv_obj_set_style_text_font(l, LV_FONT_DEFAULT, 0);
    lv_label_set_text(l, "");
    // lv_obj_set_style_transform_angle(l, 900, 0);
    // lv_obj_set_style_pad_right(l, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_pad_top(l, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    ui_status_panel.bat_image = l;

    l = lv_label_create(panel);
    lv_obj_set_width(l, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(l, LV_SIZE_CONTENT);  /// 1
    lv_obj_align(l, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_label_set_text(l, ""); // battery
    ui_status_panel.bat_label = l;

    return panel;
}

void ui_status_panel_delete(void) {
    ILOG(TAG, "[%s]", __func__);
    if (ui_status_panel.self != NULL) {
        lv_obj_clean(ui_status_panel.self);
        lv_obj_del(ui_status_panel.self);
        // ui_status_panel.self = NULL;
        // ui_status_panel.parent->status_cnt = 0;
        // uint8_t vmc = ui_status_panel.viewmode;
        memset(&ui_status_panel, 0, sizeof(ui_status_panel));
        // ui_status_panel.viewmode = vmc;
    }
}

