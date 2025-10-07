#ifndef A9C1C8AE_4ABC_49B5_B216_0E1CF4B86853
#define A9C1C8AE_4ABC_49B5_B216_0E1CF4B86853

#ifdef __cplusplus
extern "C" {
#endif

#include "sdkconfig.h"
#include "stdbool.h"
#include "stdint.h"
#include "ui_common.h"


#if defined(CONFIG_SSD168X_PANEL_SSD1680)
#define MAIN_CNT_H 82
#define STATUS_CNT_H 18
#define MAIN_CNT_H_SM 86
#define STATUS_CNT_H_SM 14
#define SCR_H 128
#else
#define MAIN_CNT_H 88
#define STATUS_CNT_H 12
#define MAIN_CNT_H_SM MAIN_CNT_H
#define STATUS_CNT_H_SM STATUS_CNT_H
#define SCR_H 170
#endif

#if defined(CONFIG_SSD168X_PANEL_SSD1680)
#define STATUS_CNT_H_P 6
#define MAIN_CNT_H_P 92
#else
#define STATUS_CNT_H_P 6
#define MAIN_CNT_H_P 94
#endif

void ui_status_panel_rearrange(ui_screen_t *parent);
void ui_status_panel_update_dims(ui_screen_t *parent);
void ui_status_panel_init(ui_screen_t * parent);
void ui_status_panel_load(ui_screen_t* parent, enum status_viewmode_e viewmode);
void ui_status_panel_delete(void);
#ifdef __cplusplus
}
#endif

#endif /* A9C1C8AE_4ABC_49B5_B216_0E1CF4B86853 */
