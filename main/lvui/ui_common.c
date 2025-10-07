#include "ui_private.h"
#include "../main/private.h"
#include "core/lv_obj_pos.h"
#include "draw/lv_draw_rect.h"

static const char *TAG = "ui_common";

// TIMER_INIT

// const lv_style_const_prop_t style_plain_panel_props[] = {
//     LV_STYLE_CONST_PAD_BOTTOM(0),
//     LV_STYLE_CONST_PAD_TOP(0),
//     LV_STYLE_CONST_PAD_LEFT(0),
//     LV_STYLE_CONST_PAD_RIGHT(0),
//     LV_STYLE_CONST_BORDER_OPA(LV_OPA_COVER),
//     LV_STYLE_CONST_TEXT_OPA(LV_OPA_COVER),
//     LV_STYLE_CONST_RADIUS(0),
//     {0}};

// const lv_style_const_prop_t style_text_small_props[] = {
//     LV_STYLE_CONST_TEXT_FONT(&ui_font_OswaldRegular12p4),
//     {0}};

// const lv_style_const_prop_t style_text_normal_props[] = {
//     LV_STYLE_CONST_TEXT_FONT(&ui_font_OswaldRegular16p4),
//     {0}};

// const lv_style_const_prop_t style_text_large_props[] = {
//     LV_STYLE_CONST_TEXT_FONT(&ui_font_OswaldRegular24p2),
//     {0}};

// const lv_style_const_prop_t style_text_oswald_regular_20_props[] = {
//     LV_STYLE_CONST_TEXT_FONT(&ui_font_OswaldRegular20p4),
//     {0}};

// const lv_style_const_prop_t style_text_oswald_regular_24_props[] = {
//     LV_STYLE_CONST_TEXT_FONT(&ui_font_OswaldRegular24p4),
//     {0}};

// const lv_style_const_prop_t style_text_monserrat_16_props[] = {
//     LV_STYLE_CONST_TEXT_FONT(LV_FONT_MONTSERRAT_16),
//     {0}};

// #if defined(CONFIG_DISPLAY_DRIVER_ST7789)
// const lv_style_const_prop_t style_text_bigger_props[] = {
//     LV_STYLE_CONST_TEXT_FONT(&ui_font_OswaldRegular36p4),
//     {0}};
// const lv_style_const_prop_t style_text_biggest_props[] = {
//     LV_STYLE_CONST_TEXT_FONT(&ui_font_OswaldRegular120p4),
//     {0}};
// #else
// const lv_style_const_prop_t style_text_bigger_props[] = {
//     LV_STYLE_CONST_TEXT_FONT(&ui_font_OpenSansBold32p4),
//     {0}};
// const lv_style_const_prop_t style_text_morebigger_props[] = {
//     LV_STYLE_CONST_TEXT_FONT(&ui_font_OpenSansBold60p2),
//     {0}};
// const lv_style_const_prop_t style_text_biggest_props[] = {
//     LV_STYLE_CONST_TEXT_FONT(&ui_font_OpenSansBold84p4),
//     {0}};
// #endif

// const lv_style_const_prop_t style_border_none_props[] = {
//     LV_STYLE_CONST_BORDER_SIDE(LV_BORDER_SIDE_NONE),
//     LV_STYLE_CONST_BORDER_WIDTH(0),
//     {0}};

// const lv_style_const_prop_t style_border_bottom_props[] = {
//     LV_STYLE_CONST_BORDER_SIDE(LV_BORDER_SIDE_BOTTOM),
//     LV_STYLE_CONST_BORDER_WIDTH(1),
//     {0}};

// const lv_style_const_prop_t style_border_top_props[] = {
//     LV_STYLE_CONST_BORDER_SIDE(LV_BORDER_SIDE_TOP),
//     LV_STYLE_CONST_BORDER_WIDTH(1),
//     {0}};

// const lv_style_const_prop_t style_border_right_props[] = {
//     LV_STYLE_CONST_BORDER_SIDE(LV_BORDER_SIDE_RIGHT),
//     LV_STYLE_CONST_BORDER_WIDTH(1),
//     {0}};

// const lv_style_const_prop_t style_border_full_props[] = {
//     LV_STYLE_CONST_BORDER_SIDE(LV_BORDER_SIDE_FULL),
//     LV_STYLE_CONST_BORDER_WIDTH(1),
//     {0}};

// LV_STYLE_CONST_INIT(style_plain_panel, style_plain_panel_props);
// LV_STYLE_CONST_INIT(style_text_oswald_regular_20, style_text_oswald_regular_20_props);
// LV_STYLE_CONST_INIT(style_text_oswald_regular_24, style_text_oswald_regular_24_props);
// LV_STYLE_CONST_INIT(style_border_none, style_border_none_props);
// LV_STYLE_CONST_INIT(style_border_bottom, style_border_bottom_props);
// LV_STYLE_CONST_INIT(style_border_top, style_border_top_props);
// LV_STYLE_CONST_INIT(style_border_right, style_border_right_props);
// LV_STYLE_CONST_INIT(style_border_full, style_border_full_props);

// LV_STYLE_CONST_INIT(style_text_small, style_text_small_props);
// LV_STYLE_CONST_INIT(style_text_normal, style_text_normal_props);
// LV_STYLE_CONST_INIT(style_text_large, style_text_large_props);
// LV_STYLE_CONST_INIT(style_text_bigger, style_text_bigger_props);
// LV_STYLE_CONST_INIT(style_text_morebigger, style_text_morebigger_props);
// LV_STYLE_CONST_INIT(style_text_biggest, style_text_biggest_props);
// LV_STYLE_CONST_INIT(style_text_monserrat_16, style_text_monserrat_16_props);

// lv_style_t style_plain_container;
// lv_style_t style_statusbar;
// lv_style_t style_speed_big_panel;

void ui_create_styles(void) {
    // lv_style_ptr_t s = &style_plain_container;
    // lv_style_init(s);
    // lv_style_set_pad_all(s, 0);
    // lv_style_set_border_opa(s, LV_OPA_COVER);
    // lv_style_set_text_opa(s, LV_OPA_COVER);
    // lv_style_set_radius(s, 0);
    // lv_style_set_text_color(s, lv_color_black());
    // lv_style_set_bg_color(s, lv_color_white());
    // lv_style_set_bg_opa(s, LV_OPA_COVER);
    // lv_style_set_border_side(s, LV_BORDER_SIDE_NONE);
    // lv_style_set_border_width(s, 0);
    // lv_style_set_width(s, LV_PCT(100));

    // s = &style_statusbar;
    // lv_style_init(s);
    // lv_style_set_border_side(s, LV_BORDER_SIDE_TOP);
    // lv_style_set_border_width(s, 1);
    // lv_style_set_text_font(s, &ui_font_OswaldRegular16p4);
}

//lv_obj_t* ui_StatusPanel = 0;

// lv_obj_t *ui____initial_actions0;

static bool ui_init_done = false;

static void ui_common_init(void) {
    ILOG(TAG, "[%s]", __func__);
    if(ui_init_done) return;
    ui_init_done = true;
    lv_disp_t* dispp = lv_disp_get_default();
#if defined(CONFIG_LCD_IS_EPD)
    lv_theme_t* theme = lv_theme_mono_init(dispp, false, LV_FONT_DEFAULT);
#else
    lv_theme_t* theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
#endif
    lv_disp_set_theme(dispp, theme);
    ui_create_styles();
}

void ui_init(void) {
    ILOG(TAG, "[%s]", __func__);
    ui_common_init();
    // ui____initial_actions0 = lv_obj_create(NULL);
    // lv_disp_load_scr( ui_SpeedScreen);
}

static void ui_common_deinit() {
    ILOG(TAG, "[%s]", __func__);
    if(!ui_init_done) return;
    ui_init_done = false;
    ui_status_panel_delete();
    lv_deinit();
}

void ui_deinit() {
    ILOG(TAG, "[%s]", __func__);
    // if(ui____initial_actions0)
    //     lv_obj_del(ui____initial_actions0);
    ui_common_deinit();
}

lv_obj_t * ui_common_panel_init(lv_obj_t * parent, uint8_t w, uint8_t h) {
    lv_obj_t * panel = lv_obj_create(parent);
    lv_obj_remove_style_all(panel);
    if(w > 0)
        lv_obj_set_width(panel, lv_pct(w));
    if(h > 0)
        lv_obj_set_height(panel, lv_pct(h));
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);  /// Flags
    return panel;
}

lv_obj_t * ui_common_screen_init(ui_screen_t * screen) {
    lv_obj_t * scr = screen->self;
    if(!scr) {
        scr = lv_obj_create(NULL);
        lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
        // lv_obj_set_style_border_color(scr, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_border_width(scr, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        screen->self = scr;
    }
    if(screen->has_status_cnt) {
        if(!screen->status_cnt) {
            lv_obj_t *panel = ui_common_panel_init(scr, 100, 18);
            lv_obj_set_align(panel, LV_ALIGN_BOTTOM_LEFT);
            screen->status_cnt = panel;
        }
        else {
            lv_obj_set_parent(screen->status_cnt, scr);
        }
    }
    return scr;
}

void ui_common_screen_uninit(ui_screen_t * screen) {
    if(screen->self) {
        if(screen->status_cnt) {
            if((lv_obj_get_parent(ui_status_panel.self) == screen->status_cnt)) {
                ui_status_panel_delete();
            }
            screen->status_cnt = NULL;
        }
        lv_obj_clean(screen->self);
        lv_obj_del(screen->self);
        screen->self = NULL;
        screen->main_cnt = NULL;
        
    }
}

static void flush_screen(ui_screen_t * screen, ui_screen_t * screen2) {
    if(screen && screen->main_cnt && screen->unload) {
        if(!screen2 || screen->main_cnt != screen2->main_cnt)
            screen->unload();
    }
}

void ui_uninit_screens(void) {
    ILOG(TAG, "[%s]", __func__);
    ui_common_screen_uninit(&ui_init_screen.screen);
    ui_common_screen_uninit(&ui_info_screen.screen);
    ui_common_screen_uninit(&ui_speed_screen.screen);
    ui_common_screen_uninit(&ui_stats_screen.screen);
    ui_common_screen_uninit(&ui_sleep_screen.screen);
    ui_common_screen_uninit(&ui_record_screen.screen);
}


void ui_flush_screens(ui_screen_t * screen) {
    ILOG(TAG, "[%s]", __func__);
    flush_screen(&ui_init_screen.screen, screen);
    flush_screen(&ui_speed_screen.screen, screen);
    flush_screen(&ui_speed_screen.screen, screen);
    flush_screen(&ui_stats_screen.screen, screen);
    flush_screen(&ui_sleep_screen.screen, screen);
    flush_screen(&ui_record_screen.screen, screen);
}

void ui_set_main_cnt_offset(ui_screen_t * screen, int8_t off) {
    if(screen){
        screen->main_cnt_offset = off;
    }
}

static void ui_invalidate_screen(ui_screen_t * screen) {
    if(screen && screen->main_cnt) {
        lv_obj_invalidate(screen->main_cnt);
    }
}
void ui_invalidate_screens(void) {
    ILOG(TAG, "[%s]", __func__);
    ui_invalidate_screen(&ui_init_screen.screen);
    ui_invalidate_screen(&ui_info_screen.screen);
    ui_invalidate_screen(&ui_speed_screen.screen);
    ui_invalidate_screen(&ui_stats_screen.screen);
    ui_invalidate_screen(&ui_sleep_screen.screen);
    ui_invalidate_screen(&ui_record_screen.screen);
}


#if (C_LOG_LEVEL < 3)
void print_lv_mem_mon() {
    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
#if LVGL_VERSION_MAJOR < 9
    printf("used: %6lu (%3hhu %%), frag: %3hhu %%, biggest free: %6d\n", mon.total_size - mon.free_size,
#else
    printf("used: %6u (%3u %%), frag: %3u %%, biggest free: %6d\n", mon.total_size - mon.free_size,
#endif
            mon.used_pct,
            mon.frag_pct,
            (int)mon.free_biggest_size);
}
#else
inline void print_lv_mem_mon() {}
#endif


// #endif
