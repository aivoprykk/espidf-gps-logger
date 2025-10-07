/*******************************************************************************
 * Size: 12 px
 * Bpp: 2
 * Opts: --bpp 2 --size 12 --font /Users/aivo/Dropbox/gt31/squareline_proj_2_13_01/assets/Oswald-Regular.ttf -o /Users/aivo/Dropbox/gt31/squareline_proj_2_13_01/assets/ui_font_OswaldRegular12p2.c --format lvgl -r 0x20-0x7f --no-compress --no-prefilter
 ******************************************************************************/

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif

#ifndef UI_FONT_OSWALDREGULAR12P2
#define UI_FONT_OSWALDREGULAR12P2 1
#endif

#if UI_FONT_OSWALDREGULAR12P2

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */

    /* U+0021 "!" */
    0x77, 0x72, 0x22, 0x21, 0x17,

    /* U+0022 "\"" */
    0xdc, 0xdc, 0x88,

    /* U+0023 "#" */
    0x1c, 0xd1, 0xcc, 0x28, 0xcb, 0xfd, 0x35, 0xc3,
    0x68, 0xbf, 0xd3, 0x34, 0x73, 0x4b, 0x30,

    /* U+0024 "$" */
    0x0, 0x2, 0xf4, 0x71, 0xc7, 0x8, 0x34, 0x1,
    0xe0, 0x7, 0x80, 0x1c, 0x70, 0xd3, 0xc, 0x2f,
    0x80, 0x10,

    /* U+0025 "%" */
    0x3e, 0xc, 0x1, 0x8c, 0x70, 0xa, 0x32, 0x80,
    0x18, 0xcd, 0x0, 0x3e, 0x30, 0x0, 0x1, 0xcf,
    0x80, 0xa, 0x73, 0x0, 0x32, 0x8d, 0x0, 0xc7,
    0x30, 0x6, 0xf, 0x80,

    /* U+0026 "&" */
    0x1f, 0x80, 0xe7, 0x3, 0x4c, 0xa, 0xa0, 0xf,
    0x0, 0x78, 0x53, 0x77, 0x5c, 0x7c, 0x31, 0xf0,
    0xbd, 0xa0,

    /* U+0027 "'" */
    0xdc, 0x80,

    /* U+0028 "(" */
    0xc, 0x28, 0x34, 0x30, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x34, 0x28, 0xc,

    /* U+0029 ")" */
    0xa0, 0xc2, 0x8a, 0x28, 0xa2, 0x8a, 0x28, 0x93,
    0x28,

    /* U+002A "*" */
    0xc, 0x1b, 0x91, 0xd0, 0x88, 0x0, 0x0,

    /* U+002B "+" */
    0xc, 0x3, 0xb, 0xf8, 0x30, 0xc, 0x0,

    /* U+002C "," */
    0x1b, 0x20,

    /* U+002D "-" */
    0xbc,

    /* U+002E "." */
    0x17,

    /* U+002F "/" */
    0x3, 0x1, 0x80, 0x90, 0x30, 0xc, 0x6, 0x2,
    0x40, 0xc0, 0x70, 0x28, 0x0,

    /* U+0030 "0" */
    0x1f, 0x83, 0x4d, 0x70, 0xd7, 0xe, 0x70, 0xe7,
    0xe, 0x70, 0xe7, 0xd, 0x34, 0xd1, 0xf8,

    /* U+0031 "1" */
    0x1d, 0xbd, 0xd, 0xd, 0xd, 0xd, 0xd, 0xd,
    0xd, 0xd,

    /* U+0032 "2" */
    0x2f, 0x43, 0x5c, 0x70, 0xd0, 0xc, 0x2, 0xc0,
    0x70, 0xe, 0x2, 0xc0, 0x34, 0xb, 0xfc,

    /* U+0033 "3" */
    0x2f, 0x47, 0x1c, 0x60, 0xc0, 0x1c, 0x7, 0x40,
    0x2c, 0x0, 0xc6, 0xc, 0x71, 0xc2, 0xf4,

    /* U+0034 "4" */
    0x3, 0x80, 0xb8, 0xe, 0x82, 0xa8, 0x36, 0x87,
    0x28, 0xbf, 0xe0, 0x38, 0x2, 0x80, 0x28,

    /* U+0035 "5" */
    0x3f, 0xc3, 0x54, 0x30, 0x3, 0x0, 0x3f, 0x82,
    0xc, 0x0, 0xd6, 0xd, 0x71, 0xc2, 0xf4,

    /* U+0036 "6" */
    0x1f, 0x83, 0x4d, 0x30, 0x47, 0x0, 0x7b, 0x87,
    0xd, 0x70, 0xd3, 0xd, 0x34, 0xc1, 0xf8,

    /* U+0037 "7" */
    0xbf, 0x0, 0xc0, 0x70, 0x28, 0xd, 0x3, 0x0,
    0xc0, 0x70, 0x28, 0xd, 0x0,

    /* U+0038 "8" */
    0x2f, 0x43, 0xc, 0x70, 0xc3, 0x5c, 0x2f, 0x43,
    0x5c, 0x70, 0xd7, 0xd, 0x30, 0xc2, 0xf8,

    /* U+0039 "9" */
    0x1f, 0x43, 0x1c, 0x70, 0xd7, 0xd, 0x70, 0xd2,
    0xed, 0x0, 0xd5, 0xc, 0x71, 0xc2, 0xf4,

    /* U+003A ":" */
    0x1, 0xc1, 0x0, 0x1, 0xc1, 0x0,

    /* U+003B ";" */
    0x70, 0x40, 0x0, 0x1, 0xc2, 0x0,

    /* U+003C "<" */
    0x0, 0xf, 0xb4, 0xb4, 0x1e, 0x0,

    /* U+003D "=" */
    0x7f, 0x40, 0x7, 0xf4,

    /* U+003E ">" */
    0x0, 0x1d, 0x0, 0xb0, 0x2c, 0x78, 0x0, 0x0,

    /* U+003F "?" */
    0x2f, 0x47, 0x1c, 0x60, 0xd0, 0xc, 0x2, 0x80,
    0xb0, 0xc, 0x0, 0x80, 0x4, 0x0, 0xc0,

    /* U+0040 "@" */
    0x1, 0xff, 0x40, 0x2d, 0x7, 0x41, 0xc0, 0x3,
    0xd, 0xf, 0xcc, 0x30, 0xd7, 0x24, 0xc7, 0x28,
    0x97, 0x28, 0x93, 0x1c, 0xa3, 0x58, 0x30, 0xe7,
    0x80, 0xa0, 0x0, 0x0, 0xe0, 0x0, 0x0, 0xbf,
    0xc0,

    /* U+0041 "A" */
    0xe, 0x0, 0xf0, 0xf, 0x1, 0xb4, 0x2a, 0x83,
    0x58, 0x31, 0xc7, 0xfc, 0x70, 0xda, 0xa,

    /* U+0042 "B" */
    0x7f, 0x87, 0xd, 0x70, 0xa7, 0xd, 0x7f, 0xc7,
    0xd, 0x70, 0xb7, 0x7, 0x70, 0xe7, 0xfc,

    /* U+0043 "C" */
    0x1f, 0x83, 0x4d, 0x70, 0xa7, 0x5, 0x70, 0x7,
    0x0, 0x70, 0x57, 0xa, 0x34, 0xd1, 0xf8,

    /* U+0044 "D" */
    0x7f, 0x87, 0xe, 0x70, 0xa7, 0xb, 0x70, 0xb7,
    0xb, 0x70, 0xb7, 0xa, 0x70, 0xd7, 0xf8,

    /* U+0045 "E" */
    0x7f, 0x9c, 0x7, 0x1, 0xc0, 0x70, 0x1f, 0xc7,
    0x1, 0xc0, 0x70, 0x1f, 0xe0,

    /* U+0046 "F" */
    0x7f, 0x9c, 0x7, 0x1, 0xc0, 0x7f, 0x1c, 0x7,
    0x1, 0xc0, 0x70, 0x1c, 0x0,

    /* U+0047 "G" */
    0x1f, 0x83, 0x4e, 0x70, 0xa7, 0x0, 0x70, 0x7,
    0x2f, 0x70, 0x77, 0x7, 0x34, 0xf1, 0xf7,

    /* U+0048 "H" */
    0x70, 0x77, 0x7, 0x70, 0x77, 0x7, 0x7f, 0xf7,
    0x7, 0x70, 0x77, 0x7, 0x70, 0x77, 0x7,

    /* U+0049 "I" */
    0x30, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc0,

    /* U+004A "J" */
    0x2c, 0xb2, 0xcb, 0x2c, 0xb2, 0xcb, 0x2f, 0xd0,

    /* U+004B "K" */
    0x70, 0xd7, 0x1c, 0x73, 0x87, 0x70, 0x7f, 0x7,
    0xf0, 0x73, 0x87, 0x1c, 0x70, 0xd7, 0xa,

    /* U+004C "L" */
    0x70, 0x1c, 0x7, 0x1, 0xc0, 0x70, 0x1c, 0x7,
    0x1, 0xc0, 0x70, 0x1f, 0xe0,

    /* U+004D "M" */
    0x34, 0x1c, 0x38, 0x2c, 0x3c, 0x3c, 0x3c, 0x3c,
    0x38, 0x6c, 0x35, 0x5c, 0x32, 0x8c, 0x33, 0xcc,
    0x33, 0x8c, 0x72, 0x4c,

    /* U+004E "N" */
    0x70, 0xa7, 0x4a, 0x78, 0xa7, 0xca, 0x7a, 0xa7,
    0x3a, 0x72, 0xe7, 0x1e, 0x70, 0xe7, 0xa,

    /* U+004F "O" */
    0x1f, 0x83, 0x4e, 0x70, 0x77, 0x7, 0x70, 0x77,
    0x7, 0x70, 0x77, 0x7, 0x34, 0xe1, 0xf8,

    /* U+0050 "P" */
    0x7f, 0x87, 0xd, 0x70, 0xa7, 0xa, 0x70, 0xd7,
    0xf8, 0x70, 0x7, 0x0, 0x70, 0x7, 0x0,

    /* U+0051 "Q" */
    0x1f, 0x83, 0x4e, 0x70, 0x77, 0x7, 0x70, 0x77,
    0x7, 0x70, 0x77, 0x7, 0x34, 0xe1, 0xfc, 0x0,
    0xc0, 0x4,

    /* U+0052 "R" */
    0x7f, 0x87, 0xd, 0x70, 0xe7, 0xd, 0x7f, 0x87,
    0x28, 0x71, 0xc7, 0xc, 0x70, 0xd7, 0xa,

    /* U+0053 "S" */
    0x2f, 0x47, 0x1c, 0x70, 0x87, 0x40, 0x1e, 0x0,
    0x78, 0x1, 0xc7, 0xd, 0x34, 0xc1, 0xf8,

    /* U+0054 "T" */
    0xff, 0xc3, 0x0, 0xc0, 0x30, 0xc, 0x3, 0x0,
    0xc0, 0x30, 0xc, 0x3, 0x0,

    /* U+0055 "U" */
    0x70, 0x77, 0x7, 0x70, 0x77, 0x7, 0x70, 0x77,
    0x7, 0x70, 0x77, 0x7, 0x34, 0xe1, 0xfc,

    /* U+0056 "V" */
    0xa0, 0xa7, 0xd, 0x30, 0xc3, 0x4c, 0x25, 0xc2,
    0xa8, 0x1e, 0x40, 0xf0, 0xf, 0x0, 0xb0,

    /* U+0057 "W" */
    0x60, 0xc3, 0x1c, 0x70, 0xc3, 0x2d, 0x30, 0xca,
    0x9c, 0x33, 0x66, 0x9, 0xce, 0x42, 0xe3, 0xd0,
    0x78, 0xb0, 0xd, 0x2c, 0x3, 0x7, 0x0,

    /* U+0058 "X" */
    0xa0, 0xd3, 0x1c, 0x36, 0x41, 0xf0, 0xe, 0x0,
    0xe0, 0x1f, 0x3, 0x68, 0x71, 0xca, 0xd,

    /* U+0059 "Y" */
    0xa0, 0xd7, 0xc, 0x31, 0xc2, 0x68, 0x1f, 0x0,
    0xf0, 0xe, 0x0, 0xe0, 0xe, 0x0, 0xe0,

    /* U+005A "Z" */
    0x7f, 0x80, 0xd0, 0x30, 0x28, 0xd, 0x7, 0x2,
    0x80, 0xd0, 0x70, 0x2f, 0xe0,

    /* U+005B "[" */
    0x79, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70,
    0x70, 0x70, 0x70, 0x79,

    /* U+005C "\\" */
    0xa0, 0x1c, 0x3, 0x0, 0x90, 0x18, 0x3, 0x0,
    0xc0, 0x24, 0x6, 0x0, 0xc0,

    /* U+005D "]" */
    0x7c, 0x71, 0xc7, 0x1c, 0x71, 0xc7, 0x1c, 0x71,
    0xdf,

    /* U+005E "^" */
    0xe, 0xa, 0xc3, 0x35, 0xc6,

    /* U+005F "_" */
    0xff, 0x0,

    /* U+0060 "`" */
    0x20, 0xc1, 0x80,

    /* U+0061 "a" */
    0x3e, 0x28, 0xc0, 0x34, 0xad, 0xa3, 0x68, 0xd7,
    0xb4,

    /* U+0062 "b" */
    0x70, 0x1c, 0x7, 0x1, 0xfd, 0x72, 0x9c, 0x77,
    0x1d, 0xc7, 0x72, 0x9f, 0xd0,

    /* U+0063 "c" */
    0x3f, 0x1c, 0xd7, 0x11, 0xc0, 0x71, 0x5c, 0xd3,
    0xf0,

    /* U+0064 "d" */
    0x2, 0x80, 0xa0, 0x28, 0xfe, 0x72, 0x9c, 0xa7,
    0x29, 0xca, 0x72, 0x8f, 0xe0,

    /* U+0065 "e" */
    0x3f, 0x1c, 0xd7, 0x29, 0xed, 0x70, 0x1c, 0xd3,
    0xf0,

    /* U+0066 "f" */
    0x2d, 0x34, 0x34, 0xbd, 0x34, 0x34, 0x34, 0x34,
    0x34, 0x34,

    /* U+0067 "g" */
    0x0, 0x2, 0xe8, 0x73, 0x46, 0x24, 0x73, 0x43,
    0xe0, 0x74, 0x3, 0xb8, 0xa0, 0xc7, 0xf4,

    /* U+0068 "h" */
    0x70, 0x1c, 0x7, 0x1, 0xed, 0x72, 0x9c, 0xa7,
    0x29, 0xca, 0x72, 0x9c, 0xa0,

    /* U+0069 "i" */
    0x70, 0x40, 0x1c, 0x71, 0xc7, 0x1c, 0x71, 0xc0,

    /* U+006A "j" */
    0xc, 0x4, 0x0, 0xc, 0xc, 0xc, 0xc, 0xc,
    0xc, 0xc, 0x1c, 0x38,

    /* U+006B "k" */
    0x70, 0x7, 0x0, 0x70, 0x7, 0x1c, 0x73, 0x47,
    0xa0, 0x7f, 0x7, 0x30, 0x72, 0x87, 0x1c,

    /* U+006C "l" */
    0x30, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc0,

    /* U+006D "m" */
    0x7b, 0xac, 0x72, 0x8a, 0x71, 0x8a, 0x71, 0x8a,
    0x71, 0x8a, 0x71, 0x8a, 0x71, 0x8a,

    /* U+006E "n" */
    0x7b, 0x5c, 0xa7, 0x29, 0xca, 0x72, 0x9c, 0xa7,
    0x28,

    /* U+006F "o" */
    0x2f, 0x1c, 0xd7, 0x29, 0xca, 0x72, 0x9c, 0xd2,
    0xf0,

    /* U+0070 "p" */
    0x7f, 0x5c, 0xa7, 0x1d, 0xc7, 0x71, 0xdc, 0xa7,
    0xf1, 0xc0, 0x70, 0x0,

    /* U+0071 "q" */
    0x3f, 0x9c, 0xa7, 0x29, 0xca, 0x72, 0x9c, 0xa3,
    0xf8, 0xa, 0x2, 0x80,

    /* U+0072 "r" */
    0x7a, 0x74, 0x70, 0x70, 0x70, 0x70, 0x70,

    /* U+0073 "s" */
    0x3d, 0x28, 0xc7, 0x0, 0x74, 0x7, 0x28, 0xc3,
    0xe0,

    /* U+0074 "t" */
    0x34, 0x34, 0xbd, 0x34, 0x34, 0x34, 0x34, 0x34,
    0x2d,

    /* U+0075 "u" */
    0x72, 0x9c, 0xa7, 0x29, 0xca, 0x72, 0x9c, 0xe3,
    0xf8,

    /* U+0076 "v" */
    0xa3, 0x68, 0xc7, 0x30, 0xdc, 0x3a, 0xb, 0x42,
    0xc0,

    /* U+0077 "w" */
    0x93, 0x19, 0x8d, 0x96, 0x76, 0x4e, 0x6c, 0x3c,
    0xf0, 0xf3, 0xc2, 0x8a, 0x0,

    /* U+0078 "x" */
    0xa3, 0x4d, 0xc2, 0xd0, 0x70, 0x3d, 0x1d, 0xca,
    0x34,

    /* U+0079 "y" */
    0xa3, 0x68, 0xd7, 0x30, 0xdc, 0x3a, 0xb, 0x41,
    0xc0, 0x70, 0x74, 0x0,

    /* U+007A "z" */
    0x7f, 0xa, 0xc, 0x2c, 0x34, 0x70, 0xbf,

    /* U+007B "{" */
    0x2d, 0x38, 0x38, 0x38, 0x34, 0x70, 0x34, 0x38,
    0x38, 0x38, 0x38, 0x1d,

    /* U+007C "|" */
    0x30, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3,
    0xc,

    /* U+007D "}" */
    0x74, 0x28, 0x28, 0x28, 0x28, 0x1d, 0x2c, 0x28,
    0x28, 0x28, 0x28, 0x74,

    /* U+007E "~" */
    0x38, 0x56, 0xe0, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 44, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 40, .box_w = 2, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 5, .adv_w = 51, .box_w = 4, .box_h = 3, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 8, .adv_w = 93, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 23, .adv_w = 94, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 41, .adv_w = 171, .box_w = 11, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 69, .adv_w = 116, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 87, .adv_w = 24, .box_w = 2, .box_h = 3, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 89, .adv_w = 57, .box_w = 4, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 101, .adv_w = 50, .box_w = 3, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 110, .adv_w = 76, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 117, .adv_w = 80, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 124, .adv_w = 36, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 126, .adv_w = 58, .box_w = 4, .box_h = 1, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 127, .adv_w = 36, .box_w = 2, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 128, .adv_w = 71, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 141, .adv_w = 99, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 156, .adv_w = 73, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 166, .adv_w = 92, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 181, .adv_w = 92, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 196, .adv_w = 93, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 211, .adv_w = 91, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 226, .adv_w = 97, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 241, .adv_w = 74, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 254, .adv_w = 96, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 269, .adv_w = 96, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 284, .adv_w = 38, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 290, .adv_w = 41, .box_w = 3, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 296, .adv_w = 72, .box_w = 4, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 302, .adv_w = 80, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 306, .adv_w = 72, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 314, .adv_w = 93, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 329, .adv_w = 174, .box_w = 11, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 362, .adv_w = 94, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 377, .adv_w = 101, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 392, .adv_w = 99, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 407, .adv_w = 101, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 422, .adv_w = 78, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 435, .adv_w = 75, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 448, .adv_w = 103, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 463, .adv_w = 108, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 478, .adv_w = 48, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 486, .adv_w = 58, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 494, .adv_w = 95, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 509, .adv_w = 76, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 522, .adv_w = 127, .box_w = 8, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 542, .adv_w = 101, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 557, .adv_w = 103, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 572, .adv_w = 92, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 587, .adv_w = 104, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 605, .adv_w = 100, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 620, .adv_w = 91, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 635, .adv_w = 79, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 648, .adv_w = 104, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 663, .adv_w = 97, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 678, .adv_w = 140, .box_w = 9, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 701, .adv_w = 93, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 716, .adv_w = 92, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 731, .adv_w = 80, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 744, .adv_w = 65, .box_w = 4, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 756, .adv_w = 71, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 769, .adv_w = 60, .box_w = 3, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 778, .adv_w = 85, .box_w = 5, .box_h = 4, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 783, .adv_w = 66, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 785, .adv_w = 53, .box_w = 3, .box_h = 3, .ofs_x = 0, .ofs_y = 7},
    {.bitmap_index = 788, .adv_w = 79, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 797, .adv_w = 86, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 810, .adv_w = 79, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 819, .adv_w = 85, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 832, .adv_w = 81, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 841, .adv_w = 55, .box_w = 4, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 851, .adv_w = 85, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 866, .adv_w = 85, .box_w = 5, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 879, .adv_w = 44, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 887, .adv_w = 44, .box_w = 4, .box_h = 12, .ofs_x = -1, .ofs_y = -2},
    {.bitmap_index = 899, .adv_w = 82, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 914, .adv_w = 44, .box_w = 3, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 922, .adv_w = 131, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 936, .adv_w = 84, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 945, .adv_w = 82, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 954, .adv_w = 86, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 966, .adv_w = 85, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 978, .adv_w = 62, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 985, .adv_w = 72, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 994, .adv_w = 59, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1003, .adv_w = 84, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1012, .adv_w = 74, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1021, .adv_w = 113, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1034, .adv_w = 75, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1043, .adv_w = 75, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1055, .adv_w = 67, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1062, .adv_w = 58, .box_w = 4, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1074, .adv_w = 46, .box_w = 3, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1083, .adv_w = 62, .box_w = 4, .box_h = 12, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 1095, .adv_w = 86, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 3}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Pair left and right glyphs for kerning*/
static const uint8_t kern_pair_glyph_ids[] =
{
    3, 34,
    3, 36,
    3, 40,
    3, 43,
    3, 48,
    3, 50,
    3, 66,
    3, 68,
    3, 69,
    3, 70,
    3, 72,
    3, 80,
    3, 82,
    3, 84,
    7, 53,
    8, 34,
    8, 36,
    8, 40,
    8, 43,
    8, 48,
    8, 50,
    8, 66,
    8, 68,
    8, 69,
    8, 70,
    8, 72,
    8, 80,
    8, 82,
    8, 84,
    11, 43,
    11, 57,
    13, 35,
    13, 36,
    13, 37,
    13, 38,
    13, 39,
    13, 40,
    13, 41,
    13, 42,
    13, 44,
    13, 45,
    13, 47,
    13, 48,
    13, 49,
    13, 50,
    13, 51,
    13, 53,
    13, 54,
    13, 55,
    13, 56,
    13, 58,
    13, 68,
    13, 69,
    13, 70,
    13, 80,
    13, 82,
    13, 85,
    13, 87,
    13, 88,
    13, 90,
    14, 53,
    14, 55,
    14, 56,
    14, 57,
    14, 58,
    15, 35,
    15, 36,
    15, 37,
    15, 38,
    15, 39,
    15, 40,
    15, 41,
    15, 42,
    15, 44,
    15, 45,
    15, 47,
    15, 48,
    15, 49,
    15, 50,
    15, 51,
    15, 53,
    15, 54,
    15, 55,
    15, 56,
    15, 58,
    15, 68,
    15, 69,
    15, 70,
    15, 80,
    15, 82,
    15, 85,
    15, 87,
    15, 88,
    15, 90,
    16, 36,
    16, 40,
    16, 48,
    16, 50,
    16, 52,
    16, 68,
    16, 69,
    16, 70,
    16, 80,
    16, 82,
    17, 24,
    17, 57,
    17, 58,
    18, 24,
    19, 21,
    21, 18,
    21, 24,
    22, 24,
    23, 18,
    23, 19,
    23, 24,
    23, 26,
    23, 58,
    24, 21,
    25, 24,
    25, 58,
    26, 24,
    27, 53,
    27, 55,
    27, 56,
    27, 58,
    28, 53,
    28, 55,
    28, 56,
    28, 58,
    34, 3,
    34, 8,
    34, 36,
    34, 40,
    34, 48,
    34, 52,
    34, 53,
    34, 54,
    34, 55,
    34, 56,
    34, 58,
    34, 71,
    34, 85,
    34, 87,
    34, 88,
    34, 90,
    35, 3,
    35, 8,
    35, 13,
    35, 15,
    35, 34,
    35, 53,
    35, 55,
    35, 56,
    35, 57,
    35, 58,
    36, 3,
    36, 8,
    36, 13,
    36, 15,
    36, 57,
    36, 58,
    37, 3,
    37, 8,
    37, 13,
    37, 15,
    37, 24,
    37, 34,
    37, 43,
    37, 53,
    37, 55,
    37, 57,
    37, 58,
    37, 59,
    37, 61,
    38, 68,
    38, 69,
    38, 70,
    38, 72,
    38, 80,
    38, 82,
    38, 84,
    39, 3,
    39, 8,
    39, 13,
    39, 15,
    39, 27,
    39, 28,
    39, 34,
    39, 43,
    39, 52,
    39, 66,
    39, 68,
    39, 69,
    39, 70,
    39, 72,
    39, 80,
    39, 82,
    39, 84,
    39, 89,
    39, 90,
    39, 91,
    40, 3,
    40, 8,
    40, 53,
    40, 55,
    40, 56,
    40, 58,
    43, 13,
    43, 15,
    44, 14,
    44, 36,
    44, 40,
    44, 48,
    44, 50,
    44, 52,
    44, 54,
    44, 66,
    44, 68,
    44, 69,
    44, 70,
    44, 72,
    44, 80,
    44, 82,
    44, 84,
    44, 85,
    44, 86,
    44, 87,
    44, 88,
    44, 90,
    45, 3,
    45, 8,
    45, 11,
    45, 14,
    45, 18,
    45, 24,
    45, 36,
    45, 40,
    45, 48,
    45, 50,
    45, 52,
    45, 53,
    45, 54,
    45, 55,
    45, 56,
    45, 58,
    45, 61,
    45, 68,
    45, 69,
    45, 70,
    45, 71,
    45, 72,
    45, 80,
    45, 82,
    45, 84,
    45, 85,
    45, 88,
    45, 90,
    46, 52,
    46, 53,
    46, 55,
    46, 58,
    48, 3,
    48, 8,
    48, 13,
    48, 15,
    48, 24,
    48, 34,
    48, 43,
    48, 53,
    48, 55,
    48, 57,
    48, 58,
    48, 59,
    48, 61,
    49, 13,
    49, 14,
    49, 15,
    49, 16,
    49, 21,
    49, 24,
    49, 34,
    49, 43,
    49, 52,
    49, 57,
    49, 59,
    49, 61,
    49, 66,
    49, 68,
    49, 69,
    49, 70,
    49, 72,
    49, 78,
    49, 79,
    49, 80,
    49, 82,
    49, 83,
    49, 84,
    49, 91,
    50, 3,
    50, 8,
    50, 13,
    50, 15,
    50, 24,
    50, 34,
    50, 43,
    50, 53,
    50, 55,
    50, 57,
    50, 58,
    50, 59,
    50, 61,
    51, 36,
    51, 40,
    51, 48,
    51, 50,
    51, 52,
    51, 53,
    51, 54,
    51, 55,
    51, 56,
    51, 58,
    51, 61,
    51, 66,
    51, 68,
    51, 69,
    51, 70,
    51, 80,
    51, 82,
    51, 88,
    51, 90,
    51, 91,
    52, 16,
    52, 34,
    52, 46,
    52, 52,
    52, 55,
    52, 57,
    52, 58,
    52, 61,
    53, 7,
    53, 13,
    53, 14,
    53, 15,
    53, 16,
    53, 27,
    53, 28,
    53, 34,
    53, 36,
    53, 40,
    53, 43,
    53, 46,
    53, 48,
    53, 50,
    53, 66,
    53, 68,
    53, 69,
    53, 70,
    53, 71,
    53, 72,
    53, 75,
    53, 78,
    53, 79,
    53, 80,
    53, 81,
    53, 82,
    53, 83,
    53, 84,
    53, 85,
    53, 86,
    53, 88,
    53, 89,
    53, 90,
    53, 91,
    54, 13,
    54, 15,
    54, 16,
    54, 34,
    55, 13,
    55, 14,
    55, 15,
    55, 16,
    55, 27,
    55, 28,
    55, 34,
    55, 36,
    55, 40,
    55, 43,
    55, 46,
    55, 48,
    55, 50,
    55, 52,
    55, 66,
    55, 68,
    55, 69,
    55, 70,
    55, 72,
    55, 78,
    55, 79,
    55, 80,
    55, 81,
    55, 82,
    55, 83,
    55, 84,
    55, 85,
    55, 86,
    55, 87,
    55, 88,
    55, 89,
    55, 90,
    55, 91,
    56, 13,
    56, 14,
    56, 15,
    56, 16,
    56, 27,
    56, 28,
    56, 34,
    56, 36,
    56, 40,
    56, 48,
    56, 50,
    56, 52,
    56, 61,
    56, 66,
    56, 68,
    56, 69,
    56, 70,
    56, 71,
    56, 72,
    56, 78,
    56, 79,
    56, 80,
    56, 81,
    56, 82,
    56, 83,
    56, 84,
    56, 87,
    56, 88,
    56, 89,
    56, 91,
    57, 11,
    57, 14,
    57, 17,
    57, 36,
    57, 40,
    57, 48,
    57, 50,
    57, 52,
    57, 61,
    57, 66,
    57, 68,
    57, 69,
    57, 70,
    57, 71,
    57, 72,
    57, 80,
    57, 82,
    57, 84,
    57, 85,
    57, 87,
    57, 88,
    57, 90,
    58, 13,
    58, 14,
    58, 15,
    58, 16,
    58, 17,
    58, 21,
    58, 23,
    58, 25,
    58, 27,
    58, 28,
    58, 34,
    58, 36,
    58, 40,
    58, 43,
    58, 46,
    58, 48,
    58, 50,
    58, 52,
    58, 66,
    58, 68,
    58, 69,
    58, 70,
    58, 71,
    58, 72,
    58, 78,
    58, 79,
    58, 80,
    58, 81,
    58, 82,
    58, 83,
    58, 84,
    58, 86,
    58, 88,
    58, 89,
    58, 90,
    58, 91,
    59, 36,
    59, 40,
    59, 48,
    59, 50,
    59, 52,
    59, 66,
    59, 68,
    59, 69,
    59, 70,
    59, 80,
    59, 82,
    61, 36,
    61, 40,
    61, 48,
    61, 50,
    61, 52,
    61, 53,
    61, 54,
    61, 55,
    61, 56,
    61, 58,
    66, 3,
    66, 8,
    66, 53,
    66, 55,
    66, 56,
    66, 58,
    66, 85,
    66, 87,
    66, 88,
    66, 90,
    67, 3,
    67, 8,
    67, 13,
    67, 15,
    67, 53,
    67, 55,
    67, 56,
    67, 58,
    67, 72,
    67, 75,
    67, 88,
    67, 89,
    68, 3,
    68, 8,
    68, 13,
    68, 15,
    68, 53,
    68, 56,
    68, 58,
    68, 68,
    68, 69,
    68, 70,
    68, 75,
    68, 80,
    68, 82,
    69, 53,
    69, 55,
    69, 56,
    69, 58,
    70, 3,
    70, 8,
    70, 53,
    70, 55,
    70, 56,
    70, 58,
    70, 75,
    70, 88,
    70, 89,
    71, 15,
    71, 27,
    71, 28,
    71, 34,
    71, 66,
    71, 68,
    71, 69,
    71, 70,
    71, 71,
    71, 74,
    71, 75,
    71, 80,
    71, 82,
    71, 86,
    72, 53,
    72, 55,
    72, 57,
    72, 58,
    72, 59,
    72, 68,
    72, 69,
    72, 70,
    72, 72,
    72, 75,
    72, 80,
    72, 82,
    72, 85,
    72, 86,
    72, 88,
    72, 90,
    73, 3,
    73, 8,
    73, 53,
    73, 55,
    73, 56,
    73, 58,
    76, 3,
    76, 8,
    76, 53,
    76, 56,
    76, 58,
    76, 68,
    76, 69,
    76, 70,
    76, 72,
    76, 80,
    76, 82,
    76, 84,
    78, 3,
    78, 8,
    78, 53,
    78, 55,
    78, 56,
    78, 58,
    79, 3,
    79, 8,
    79, 53,
    79, 55,
    79, 56,
    79, 58,
    80, 3,
    80, 8,
    80, 13,
    80, 15,
    80, 53,
    80, 55,
    80, 56,
    80, 58,
    80, 72,
    80, 75,
    80, 88,
    80, 89,
    81, 3,
    81, 8,
    81, 13,
    81, 15,
    81, 53,
    81, 55,
    81, 56,
    81, 58,
    81, 72,
    81, 75,
    81, 88,
    81, 89,
    82, 3,
    82, 8,
    82, 53,
    82, 55,
    82, 58,
    83, 13,
    83, 15,
    83, 34,
    83, 43,
    83, 53,
    83, 57,
    83, 58,
    83, 59,
    83, 66,
    83, 68,
    83, 69,
    83, 70,
    83, 72,
    83, 80,
    83, 82,
    84, 3,
    84, 8,
    84, 53,
    84, 55,
    84, 56,
    84, 57,
    84, 58,
    85, 3,
    85, 8,
    85, 52,
    85, 53,
    85, 55,
    85, 58,
    85, 72,
    86, 3,
    86, 8,
    86, 53,
    86, 55,
    86, 56,
    86, 58,
    87, 13,
    87, 15,
    87, 34,
    87, 43,
    87, 53,
    87, 55,
    87, 56,
    87, 57,
    87, 58,
    87, 59,
    87, 66,
    87, 72,
    88, 13,
    88, 15,
    88, 34,
    88, 43,
    88, 53,
    88, 55,
    88, 56,
    88, 57,
    88, 58,
    88, 59,
    88, 66,
    88, 72,
    89, 53,
    89, 55,
    89, 56,
    89, 58,
    89, 68,
    89, 69,
    89, 70,
    89, 80,
    89, 82,
    90, 13,
    90, 15,
    90, 34,
    90, 43,
    90, 53,
    90, 55,
    90, 56,
    90, 57,
    90, 58,
    90, 59,
    90, 66,
    90, 72,
    91, 53,
    91, 55,
    91, 56,
    91, 58,
    91, 68,
    91, 69,
    91, 70,
    91, 72,
    91, 80,
    91, 82
};

/* Kerning between the respective left and right glyphs
 * 4.4 format which needs to scaled with `kern_scale`*/
static const int8_t kern_pair_values[] =
{
    -12, -1, -1, -6, -1, -1, -2, -4,
    -4, -4, -6, -4, -4, -2, -2, -12,
    -1, -1, -6, -1, -1, -2, -4, -4,
    -4, -6, -4, -4, -2, -2, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -10, -1,
    -8, -7, -13, -1, -1, -1, -1, -1,
    -2, -3, -1, -2, -3, -5, -1, -1,
    -3, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -10, -1, -8, -7, -13, -1, -1, -1,
    -1, -1, -2, -3, -1, -2, -1, -1,
    -1, -1, -2, -4, -4, -4, -4, -4,
    -2, -1, -1, -2, -1, -1, -3, -1,
    -1, -1, -2, -1, -1, -6, -3, -1,
    -2, -12, -1, -2, -8, -12, -1, -2,
    -8, -12, -12, -1, -1, -1, -1, -8,
    -1, -7, -5, -8, -3, -2, -1, -3,
    -3, -3, -3, -1, -1, 0, -4, -5,
    0, -4, -1, -1, -1, -1, -1, -1,
    0, -2, -2, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -2, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -12,
    -12, -2, -2, -7, -6, 0, -4, -3,
    -3, -3, -2, -3, -3, -3, -2, -2,
    -2, -2, -2, -3, -1, 0, -2, -5,
    -5, -2, -3, -3, -3, -2, -1, 0,
    -1, -2, -2, -2, -1, -2, -2, -1,
    -1, 0, -1, -2, -5, -6, -6, -8,
    -3, -2, -1, -1, -1, -1, -2, -1,
    -15, -2, -14, -9, -17, -6, -1, -1,
    -1, -2, -1, -1, -1, -1, -2, -6,
    -9, -1, -2, -1, -1, -2, -2, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -2, -20, -17, -21, -10, -5, -1,
    -9, -4, -1, -2, -5, -2, -5, -4,
    -4, -4, -3, -1, -1, -4, 0, -1,
    -2, -1, -2, -2, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -2, -1,
    -1, -1, -1, 0, -3, 0, -1, -1,
    -3, -2, -2, -2, -2, -2, -2, -2,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -2, -1, -10, -3, -10, -4,
    -12, -12, -8, -1, -1, -4, -2, -1,
    -1, -12, -12, -12, -12, -6, -12, -2,
    -11, -11, -12, -11, 0, -11, -12, -3,
    -9, -12, -10, -12, -10, -1, -1, -2,
    -1, -9, -5, -9, -1, -2, -2, -7,
    -1, -1, -4, -1, -1, -1, 0, -12,
    -8, -8, -8, -9, -4, -4, -8, -6,
    -6, -4, -8, -2, -5, -4, -4, -4,
    -6, -8, -7, -1, -7, -2, -2, -2,
    -5, 0, 0, 0, 0, 0, -1, -7,
    -5, -5, -5, -1, -4, -2, -2, -5,
    -2, 0, -2, -3, -2, -2, -2, -2,
    -2, -1, -1, -1, -1, -1, -1, -1,
    -1, -2, -2, -2, -2, -4, -2, -2,
    -2, -4, -4, -4, -6, -6, -13, -3,
    -13, -3, -1, -2, -1, -1, -9, -9,
    -8, -1, -1, -4, -1, -1, -1, -1,
    -13, -12, -12, -12, -5, -11, -9, -9,
    -12, -8, -3, -9, -9, -8, -7, -8,
    -6, -5, -1, -1, -1, -1, 0, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -2, -4, -2, -4, -3, -4, -5,
    -5, -11, -9, -6, -13, 0, -1, -1,
    0, -4, -4, -1, -1, -12, -8, -5,
    -12, 0, -1, 0, -2, -3, -3, 0,
    0, -9, 0, -8, -1, -1, -1, -1,
    -1, -1, 6, 2, 2, 4, -1, -1,
    -12, -6, -2, -10, -1, 0, -2, -2,
    -1, -1, -1, -2, -4, -4, -4, -6,
    -2, -2, -4, -4, -2, -7, -2, -4,
    -2, -1, -1, -1, -1, -2, 2, -1,
    -1, 1, 0, 1, 1, -6, -6, -11,
    -6, -2, -9, -1, -1, -6, -1, -5,
    -1, -1, -1, -1, -1, -1, -1, -6,
    -6, -11, -6, -2, -9, -6, -6, -11,
    -6, -2, -9, -4, -4, -1, -1, -12,
    -8, -5, -12, 0, -1, 0, -2, -4,
    -4, -1, -1, -12, -8, -5, -12, 0,
    -1, 0, -2, -1, -1, -10, -4, -7,
    -14, -14, -5, -2, -10, -4, -3, -4,
    -2, -1, -1, -1, -1, -1, -2, -2,
    -2, -12, -8, -1, -2, -11, -2, -2,
    0, -5, -2, -2, -1, -2, -2, -11,
    -4, -1, -8, -3, -4, -3, -2, -13,
    -4, -2, -4, -6, -4, -1, -1, -3,
    -4, -2, -6, -12, -4, -2, -6, -6,
    -1, -1, -1, -10, -4, -2, -8, -2,
    -2, -2, -2, -2, -6, -6, -3, -2,
    -13, -4, -2, -4, -6, -4, -1, -1,
    -10, -3, -1, -5, -2, -2, -2, -1,
    -2, -2
};

/*Collect the kern pair's data in one place*/
static const lv_font_fmt_txt_kern_pair_t kern_pairs =
{
    .glyph_ids = kern_pair_glyph_ids,
    .values = kern_pair_values,
    .pair_cnt = 746,
    .glyph_ids_size = 0
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR <= 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_pairs,
    .kern_scale = 16,
    .cmap_num = 1,
    .bpp = 2,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR <= 8
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t ui_font_OswaldRegular12p2 = {
#else
lv_font_t ui_font_OswaldRegular12p2 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 13,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if UI_FONT_OSWALDREGULAR12P2*/

