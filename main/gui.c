#include "private.h"

#define ICON_G_CHECK_CIRCLE 0xe86c
#define ICON_G_RADIO_UNCHECKED 0xe836
#define ICON_G_RADIO_CHECKED 0xe837
#define ICON_G_RADIO_PARTIAL 0xf560
#define ICON_G_WIFI 0xe63e
#define ICON_G_GPS 0xe569
#define ICON_G_SDCARD 0xe623
#define ICON_G_BATTERY_HORIZ_0 0xf8ae
#define ICON_G_BATTERY_CHARGE 0xf7eb
#define ICON_G_THERMOMETER 0xe846

// Populate utf8 with 0-4 bytes
// Return length used in utf8[]
// 0 implies bad codepoint
static unsigned unicode_codepoint_to_utf8(uint8_t *utf8, uint32_t codepoint) {
  if (codepoint <= 0x7F) {
    utf8[0] = codepoint;
    return 1;
  }
  if (codepoint <= 0x7FF) {
    utf8[0] = 0xC0 | (codepoint >> 6);
    utf8[1] = 0x80 | (codepoint & 0x3F);
    return 2;
  }
  if (codepoint <= 0xFFFF) {
    // detect surrogates
    if (codepoint >= 0xD800 && codepoint <= 0xDFFF) return 0;
    utf8[0] = 0xE0 | (codepoint >> 12);
    utf8[1] = 0x80 | ((codepoint >> 6) & 0x3F);
    utf8[2] = 0x80 | (codepoint & 0x3F);
    return 3;
  }
  if (codepoint <= 0x10FFFF) {
    utf8[0] = 0xF0 | (codepoint >> 18);
    utf8[1] = 0x80 | ((codepoint >> 12) & 0x3F);
    utf8[2] = 0x80 | ((codepoint >> 6) & 0x3F);
    utf8[3] = 0x80 | (codepoint & 0x3F);
    return 4;
  }
  return 0;
}

esp_err_t gui_init() {
  return ESP_OK;
}  

esp_err_t gui_deinit() {
  return ESP_OK;
}

