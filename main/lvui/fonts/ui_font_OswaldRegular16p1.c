/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --bpp 1 --size 16 --font /Users/aivo/Dropbox/gt31/squareline_proj_2_13_01/assets/Oswald-Regular.ttf -o ./ui_font_OswaldRegular16p1.c --format lvgl -r 0x20-0x7f --no-compress --no-prefilter
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

#ifndef UI_FONT_OSWALDREGULAR16P1
#define UI_FONT_OSWALDREGULAR16P1 1
#endif

#if UI_FONT_OSWALDREGULAR16P1

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xaa, 0xaa, 0xa8, 0xf0,

    /* U+0022 "\"" */
    0xaa, 0xaa,

    /* U+0023 "#" */
    0x36, 0x6c, 0x99, 0x36, 0x5f, 0xdb, 0x36, 0xfe,
    0x9b, 0x36, 0x4d, 0x9b, 0x0,

    /* U+0024 "$" */
    0x10, 0xf3, 0xf6, 0x3c, 0x78, 0x18, 0x3c, 0x3c,
    0x38, 0x3e, 0x3c, 0x7c, 0xdf, 0x8,

    /* U+0025 "%" */
    0x71, 0x86, 0xcc, 0x36, 0x61, 0xb2, 0xd, 0xb0,
    0x6d, 0x81, 0xcc, 0x0, 0xce, 0x6, 0xd8, 0x36,
    0xc1, 0x36, 0x19, 0xb0, 0xcd, 0x86, 0x38,

    /* U+0026 "&" */
    0x78, 0xcc, 0xcc, 0xcc, 0xd8, 0x78, 0x30, 0x73,
    0x53, 0xdb, 0xce, 0xce, 0xcf, 0x7b,

    /* U+0027 "'" */
    0xaa,

    /* U+0028 "(" */
    0x2b, 0x6d, 0xb6, 0xdb, 0x6d, 0xb2, 0x60,

    /* U+0029 ")" */
    0xc6, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x36,
    0xc0,

    /* U+002A "*" */
    0x21, 0x7e, 0xc5, 0x28,

    /* U+002B "+" */
    0x30, 0xc3, 0x3f, 0x30, 0xc3, 0x0,

    /* U+002C "," */
    0xf7,

    /* U+002D "-" */
    0xf0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0x8, 0x46, 0x31, 0x8, 0xc6, 0x21, 0x18, 0xc4,
    0x20,

    /* U+0030 "0" */
    0x38, 0xcb, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc7,
    0x8f, 0x1e, 0x36, 0xc7, 0x0,

    /* U+0031 "1" */
    0x3f, 0xf3, 0x33, 0x33, 0x33, 0x33, 0x33,

    /* U+0032 "2" */
    0x7d, 0xcb, 0x1e, 0x30, 0x61, 0xc3, 0xe, 0x18,
    0x60, 0xc3, 0xe, 0x1f, 0x80,

    /* U+0033 "3" */
    0x7d, 0x8f, 0x18, 0x30, 0x63, 0x87, 0x7, 0x6,
    0xf, 0x1e, 0x3c, 0x4f, 0x0,

    /* U+0034 "4" */
    0x1c, 0x38, 0x71, 0xe2, 0xcd, 0x9b, 0x66, 0xcd,
    0xfc, 0x30, 0x60, 0xc1, 0x80,

    /* U+0035 "5" */
    0xfb, 0xec, 0x30, 0xc3, 0xec, 0xc3, 0xc, 0x3c,
    0xf3, 0xc9, 0xe0,

    /* U+0036 "6" */
    0x79, 0x3c, 0xf3, 0xc3, 0xf, 0xb3, 0xcf, 0x3c,
    0xf3, 0x49, 0xe0,

    /* U+0037 "7" */
    0xf8, 0xc6, 0x31, 0x8, 0xc6, 0x31, 0x8, 0x46,
    0x30,

    /* U+0038 "8" */
    0x7b, 0x3c, 0xf3, 0xcd, 0x23, 0x12, 0xcf, 0x3c,
    0xf3, 0xcd, 0xe0,

    /* U+0039 "9" */
    0x79, 0x2c, 0xf3, 0xcf, 0x3c, 0xdf, 0xc, 0x3c,
    0xf3, 0xc9, 0xe0,

    /* U+003A ":" */
    0xf0, 0x3c,

    /* U+003B ";" */
    0xf0, 0xf, 0x70,

    /* U+003C "<" */
    0x8, 0xdd, 0x8c, 0x38, 0x60,

    /* U+003D "=" */
    0xf8, 0x1, 0xf0,

    /* U+003E ">" */
    0x6, 0x1c, 0x73, 0xf3, 0x0,

    /* U+003F "?" */
    0x7b, 0x3c, 0xe3, 0xc, 0x71, 0x8c, 0x20, 0x80,
    0x0, 0x20, 0x80,

    /* U+0040 "@" */
    0x7, 0xc0, 0xc1, 0xc, 0x4, 0xc0, 0x26, 0x7c,
    0xe7, 0x67, 0x33, 0x3b, 0x11, 0xd8, 0x8e, 0xcc,
    0xb6, 0x65, 0x9d, 0xc6, 0x0, 0x30, 0x0, 0xe0,
    0x1, 0xf8,

    /* U+0041 "A" */
    0x18, 0x18, 0x18, 0x38, 0x3c, 0x2c, 0x24, 0x24,
    0x64, 0x7e, 0x66, 0x46, 0x42, 0xc2,

    /* U+0042 "B" */
    0xfd, 0x8f, 0x1e, 0x3c, 0x79, 0xbe, 0x66, 0xc7,
    0x8f, 0x1e, 0x3c, 0xdf, 0x80,

    /* U+0043 "C" */
    0x3c, 0x8f, 0x1e, 0x3c, 0x78, 0x30, 0x60, 0xc1,
    0x8f, 0x1e, 0x34, 0x67, 0x80,

    /* U+0044 "D" */
    0xf9, 0x9b, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc7,
    0x8f, 0x1e, 0x3c, 0xdf, 0x0,

    /* U+0045 "E" */
    0xfe, 0x31, 0x8c, 0x63, 0xf8, 0xc6, 0x31, 0x8c,
    0x7c,

    /* U+0046 "F" */
    0xfe, 0x31, 0x8c, 0x63, 0xd8, 0xc6, 0x31, 0x8c,
    0x60,

    /* U+0047 "G" */
    0x7c, 0x8f, 0x1e, 0x3c, 0x18, 0x30, 0x67, 0xc7,
    0x8f, 0x1e, 0x36, 0xef, 0x40,

    /* U+0048 "H" */
    0xc7, 0x8f, 0x1e, 0x3c, 0x78, 0xff, 0xe3, 0xc7,
    0x8f, 0x1e, 0x3c, 0x78, 0xc0,

    /* U+0049 "I" */
    0xff, 0xff, 0xff, 0xf0,

    /* U+004A "J" */
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3e,

    /* U+004B "K" */
    0xc5, 0x9b, 0x36, 0xcd, 0x9e, 0x3c, 0x7c, 0xd9,
    0x93, 0x36, 0x6c, 0x78, 0xc0,

    /* U+004C "L" */
    0xc6, 0x31, 0x8c, 0x63, 0x18, 0xc6, 0x31, 0x8c,
    0x7c,

    /* U+004D "M" */
    0xe3, 0xf1, 0xf8, 0xfc, 0x7e, 0x3f, 0x1f, 0x57,
    0xab, 0xd5, 0xea, 0xf5, 0x79, 0x3c, 0x9e, 0x4c,

    /* U+004E "N" */
    0xc7, 0x8f, 0x9f, 0x3e, 0x7e, 0xf5, 0xeb, 0xdf,
    0x9f, 0x3e, 0x3c, 0x78, 0xc0,

    /* U+004F "O" */
    0x78, 0x8b, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc7,
    0x8f, 0x1e, 0x34, 0x4f, 0x0,

    /* U+0050 "P" */
    0xfd, 0x8f, 0x1e, 0x3c, 0x78, 0xff, 0x60, 0xc1,
    0x83, 0x6, 0xc, 0x18, 0x0,

    /* U+0051 "Q" */
    0x7c, 0x8b, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc7,
    0x8f, 0x1e, 0x34, 0x4f, 0x3, 0x2, 0x0,

    /* U+0052 "R" */
    0xfd, 0x9f, 0x1e, 0x3c, 0x79, 0xff, 0x64, 0xcd,
    0x9b, 0x36, 0x3c, 0x78, 0xc0,

    /* U+0053 "S" */
    0x79, 0x9b, 0x1e, 0x2e, 0xe, 0x1e, 0x1e, 0x1e,
    0x1f, 0x1e, 0x36, 0x67, 0x80,

    /* U+0054 "T" */
    0xfc, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3,
    0xc, 0x30, 0xc0,

    /* U+0055 "U" */
    0xc7, 0x8f, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc7,
    0x8f, 0x1e, 0x34, 0x47, 0x80,

    /* U+0056 "V" */
    0xc3, 0x42, 0x62, 0x66, 0x66, 0x26, 0x24, 0x34,
    0x3c, 0x3c, 0x1c, 0x18, 0x18, 0x18,

    /* U+0057 "W" */
    0x8c, 0x63, 0x38, 0xcf, 0x33, 0xdc, 0xb7, 0x25,
    0x69, 0x4a, 0x53, 0x94, 0xe7, 0x39, 0xcc, 0x63,
    0x8, 0x40,

    /* U+0058 "X" */
    0xc2, 0x66, 0x64, 0x2c, 0x3c, 0x38, 0x18, 0x18,
    0x38, 0x2c, 0x6c, 0x66, 0x46, 0xc2,

    /* U+0059 "Y" */
    0xc3, 0x42, 0x66, 0x66, 0x24, 0x3c, 0x3c, 0x18,
    0x18, 0x18, 0x18, 0x18, 0x18, 0x18,

    /* U+005A "Z" */
    0xfc, 0x31, 0x86, 0x18, 0xc3, 0xc, 0x61, 0x86,
    0x30, 0xc3, 0xf0,

    /* U+005B "[" */
    0xfc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    0xf0,

    /* U+005C "\\" */
    0x42, 0x18, 0xc2, 0x10, 0xc6, 0x10, 0x86, 0x30,
    0x84,

    /* U+005D "]" */
    0xf3, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0xf0,

    /* U+005E "^" */
    0x21, 0xc5, 0x36, 0xda, 0x20,

    /* U+005F "_" */
    0xfc,

    /* U+0060 "`" */
    0xd9, 0x20,

    /* U+0061 "a" */
    0x76, 0xf6, 0x33, 0xbf, 0x7b, 0xdb, 0xc0,

    /* U+0062 "b" */
    0xc3, 0xc, 0x30, 0xfb, 0x3c, 0xf3, 0xcf, 0x3c,
    0xf3, 0xcf, 0xe0,

    /* U+0063 "c" */
    0x76, 0xf7, 0xbc, 0x63, 0x7b, 0xdb, 0x80,

    /* U+0064 "d" */
    0xc, 0x30, 0xc3, 0x7f, 0x3c, 0xf3, 0xcf, 0x3c,
    0xf3, 0xcd, 0xf0,

    /* U+0065 "e" */
    0x7b, 0x3c, 0xf3, 0xff, 0xc, 0x33, 0xcd, 0xe0,

    /* U+0066 "f" */
    0x36, 0x66, 0xf6, 0x66, 0x66, 0x66, 0x66,

    /* U+0067 "g" */
    0x7a, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x78, 0xc0,
    0xfc, 0x7e, 0xc6, 0xc6, 0x7c,

    /* U+0068 "h" */
    0xc3, 0xc, 0x30, 0xdf, 0xbc, 0xf3, 0xcf, 0x3c,
    0xf3, 0xcf, 0x30,

    /* U+0069 "i" */
    0xf0, 0xff, 0xff, 0xf0,

    /* U+006A "j" */
    0x6c, 0x6, 0xdb, 0x6d, 0xb6, 0xdb, 0xc0,

    /* U+006B "k" */
    0xc3, 0xc, 0x30, 0xcf, 0x2d, 0xbc, 0xf3, 0x6d,
    0xb2, 0xcf, 0x30,

    /* U+006C "l" */
    0xff, 0xff, 0xff, 0xf0,

    /* U+006D "m" */
    0xdd, 0xbb, 0xbc, 0xcf, 0x33, 0xcc, 0xf3, 0x3c,
    0xcf, 0x33, 0xcc, 0xf3, 0x30,

    /* U+006E "n" */
    0xfe, 0xf7, 0xbd, 0xef, 0x7b, 0xde, 0xc0,

    /* U+006F "o" */
    0x7b, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0xcd, 0xe0,

    /* U+0070 "p" */
    0xfb, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0xcf, 0xec,
    0x30, 0xc0,

    /* U+0071 "q" */
    0x7f, 0x3c, 0xf3, 0xcf, 0x3c, 0xf3, 0xcd, 0xf0,
    0xc3, 0xc,

    /* U+0072 "r" */
    0xfc, 0xcc, 0xcc, 0xcc, 0xcc,

    /* U+0073 "s" */
    0x76, 0xf5, 0xc7, 0x38, 0xfb, 0xdb, 0x80,

    /* U+0074 "t" */
    0x66, 0x6f, 0x66, 0x66, 0x66, 0x66, 0x30,

    /* U+0075 "u" */
    0xde, 0xf7, 0xbd, 0xef, 0x7b, 0xdf, 0xc0,

    /* U+0076 "v" */
    0xcd, 0x34, 0x92, 0x69, 0xe7, 0x8e, 0x30, 0xc0,

    /* U+0077 "w" */
    0x49, 0xa6, 0xd3, 0x6b, 0xa5, 0x53, 0xa9, 0xd4,
    0x66, 0x33, 0x19, 0x80,

    /* U+0078 "x" */
    0xcd, 0x27, 0x8e, 0x30, 0xc3, 0x9e, 0x4b, 0x30,

    /* U+0079 "y" */
    0x4d, 0x34, 0xdb, 0x69, 0xa3, 0x8e, 0x38, 0xc3,
    0xc, 0x60,

    /* U+007A "z" */
    0xf3, 0x22, 0x66, 0x4c, 0xcf,

    /* U+007B "{" */
    0x36, 0x66, 0x66, 0x66, 0xc6, 0x66, 0x66, 0x66,
    0x30,

    /* U+007C "|" */
    0xff, 0xff, 0xff, 0xff, 0xc0,

    /* U+007D "}" */
    0xc6, 0x66, 0x66, 0x66, 0x36, 0x66, 0x66, 0x66,
    0xc0,

    /* U+007E "~" */
    0xe0, 0xe0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 59, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 53, .box_w = 2, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 5, .adv_w = 68, .box_w = 4, .box_h = 4, .ofs_x = 1, .ofs_y = 10},
    {.bitmap_index = 7, .adv_w = 124, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 20, .adv_w = 126, .box_w = 7, .box_h = 16, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 34, .adv_w = 228, .box_w = 13, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 57, .adv_w = 155, .box_w = 8, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 71, .adv_w = 33, .box_w = 2, .box_h = 4, .ofs_x = 0, .ofs_y = 10},
    {.bitmap_index = 72, .adv_w = 76, .box_w = 3, .box_h = 17, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 79, .adv_w = 67, .box_w = 4, .box_h = 17, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 88, .adv_w = 102, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 92, .adv_w = 106, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 98, .adv_w = 48, .box_w = 2, .box_h = 4, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 99, .adv_w = 78, .box_w = 4, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 100, .adv_w = 48, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 101, .adv_w = 94, .box_w = 5, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 110, .adv_w = 132, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 123, .adv_w = 97, .box_w = 4, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 130, .adv_w = 122, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 143, .adv_w = 122, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 156, .adv_w = 124, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 169, .adv_w = 122, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 180, .adv_w = 129, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 191, .adv_w = 99, .box_w = 5, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 200, .adv_w = 128, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 211, .adv_w = 129, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 222, .adv_w = 50, .box_w = 2, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 224, .adv_w = 55, .box_w = 2, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 227, .adv_w = 95, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 232, .adv_w = 106, .box_w = 5, .box_h = 4, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 235, .adv_w = 95, .box_w = 5, .box_h = 7, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 240, .adv_w = 124, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 251, .adv_w = 231, .box_w = 13, .box_h = 16, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 277, .adv_w = 126, .box_w = 8, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 291, .adv_w = 134, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 304, .adv_w = 132, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 317, .adv_w = 135, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 330, .adv_w = 104, .box_w = 5, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 339, .adv_w = 100, .box_w = 5, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 348, .adv_w = 137, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 361, .adv_w = 144, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 374, .adv_w = 63, .box_w = 2, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 378, .adv_w = 77, .box_w = 4, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 385, .adv_w = 127, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 398, .adv_w = 102, .box_w = 5, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 407, .adv_w = 169, .box_w = 9, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 423, .adv_w = 135, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 436, .adv_w = 138, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 449, .adv_w = 123, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 462, .adv_w = 138, .box_w = 7, .box_h = 17, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 477, .adv_w = 133, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 490, .adv_w = 121, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 503, .adv_w = 106, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 514, .adv_w = 139, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 527, .adv_w = 129, .box_w = 8, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 541, .adv_w = 187, .box_w = 10, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 559, .adv_w = 123, .box_w = 8, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 573, .adv_w = 123, .box_w = 8, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 587, .adv_w = 107, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 598, .adv_w = 87, .box_w = 4, .box_h = 17, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 607, .adv_w = 94, .box_w = 5, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 616, .adv_w = 79, .box_w = 4, .box_h = 17, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 625, .adv_w = 113, .box_w = 6, .box_h = 6, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 630, .adv_w = 88, .box_w = 6, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 631, .adv_w = 71, .box_w = 3, .box_h = 4, .ofs_x = 1, .ofs_y = 10},
    {.bitmap_index = 633, .adv_w = 105, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 640, .adv_w = 114, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 651, .adv_w = 105, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 658, .adv_w = 113, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 669, .adv_w = 108, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 677, .adv_w = 73, .box_w = 4, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 684, .adv_w = 113, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 697, .adv_w = 114, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 708, .adv_w = 59, .box_w = 2, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 712, .adv_w = 59, .box_w = 3, .box_h = 17, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 719, .adv_w = 110, .box_w = 6, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 730, .adv_w = 59, .box_w = 2, .box_h = 14, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 734, .adv_w = 174, .box_w = 10, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 747, .adv_w = 112, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 754, .adv_w = 109, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 762, .adv_w = 114, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 772, .adv_w = 113, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 782, .adv_w = 82, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 787, .adv_w = 96, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 794, .adv_w = 79, .box_w = 4, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 801, .adv_w = 113, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 808, .adv_w = 99, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 816, .adv_w = 151, .box_w = 9, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 828, .adv_w = 100, .box_w = 6, .box_h = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 836, .adv_w = 100, .box_w = 6, .box_h = 13, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 846, .adv_w = 89, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 851, .adv_w = 77, .box_w = 4, .box_h = 17, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 860, .adv_w = 61, .box_w = 2, .box_h = 17, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 865, .adv_w = 82, .box_w = 4, .box_h = 17, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 874, .adv_w = 114, .box_w = 6, .box_h = 2, .ofs_x = 1, .ofs_y = 6}
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
    -16, -2, -2, -8, -2, -2, -3, -6,
    -6, -6, -8, -6, -6, -3, -3, -16,
    -2, -2, -8, -2, -2, -3, -6, -6,
    -6, -8, -6, -6, -3, -3, -2, -1,
    -2, -1, -1, -1, -2, -1, -1, -1,
    -1, -1, -2, -1, -2, -1, -14, -2,
    -11, -9, -17, -2, -2, -2, -2, -2,
    -3, -4, -2, -3, -4, -7, -1, -2,
    -4, -1, -2, -1, -1, -1, -2, -1,
    -1, -1, -1, -1, -2, -1, -2, -1,
    -14, -2, -11, -9, -17, -2, -2, -2,
    -2, -2, -3, -4, -2, -3, -2, -2,
    -2, -2, -3, -5, -5, -5, -5, -5,
    -3, -1, -2, -3, -2, -2, -4, -2,
    -1, -1, -3, -1, -1, -8, -4, -1,
    -3, -16, -2, -3, -11, -16, -2, -3,
    -11, -16, -16, -1, -1, -1, -1, -10,
    -1, -10, -7, -10, -4, -3, -2, -4,
    -4, -4, -4, -2, -2, 0, -6, -6,
    -1, -5, -2, -2, -2, -1, -1, -1,
    -1, -3, -3, -2, -2, -1, -1, -1,
    -1, -1, -2, -2, -1, -3, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -16,
    -16, -3, -3, -9, -8, -1, -5, -4,
    -4, -4, -2, -4, -4, -4, -3, -3,
    -3, -3, -3, -4, -2, 0, -3, -7,
    -7, -3, -5, -5, -5, -3, -2, 0,
    -1, -3, -3, -3, -1, -3, -3, -1,
    -2, -1, -1, -3, -7, -7, -7, -11,
    -4, -3, -1, -2, -2, -2, -2, -1,
    -20, -2, -19, -13, -23, -8, -1, -1,
    -1, -3, -1, -1, -1, -1, -3, -8,
    -12, -1, -2, -1, -1, -3, -3, -2,
    -2, -1, -1, -1, -1, -1, -2, -2,
    -1, -3, -26, -22, -28, -14, -7, -1,
    -12, -5, -1, -3, -7, -3, -6, -5,
    -5, -5, -4, -2, -2, -5, 0, -2,
    -3, -2, -3, -3, -2, -2, -1, -1,
    -1, -1, -1, -2, -2, -1, -3, -1,
    -1, -1, -1, -1, -4, -1, -1, -1,
    -4, -3, -3, -2, -2, -2, -2, -2,
    -2, -1, -2, -2, -1, -1, -2, -1,
    -1, -1, -3, -1, -14, -4, -14, -5,
    -16, -16, -10, -1, -1, -6, -2, -1,
    -1, -17, -16, -16, -16, -8, -17, -3,
    -15, -15, -16, -15, 0, -15, -17, -4,
    -12, -16, -13, -15, -14, -2, -2, -3,
    -1, -12, -7, -12, -2, -3, -3, -10,
    -1, -1, -5, -1, -1, -1, -1, -15,
    -10, -10, -10, -13, -5, -5, -10, -8,
    -8, -5, -10, -3, -7, -5, -5, -5,
    -8, -10, -9, -1, -9, -3, -3, -3,
    -7, 0, 0, 0, 0, -1, -2, -10,
    -7, -7, -7, -1, -6, -3, -3, -7,
    -3, 0, -3, -4, -3, -3, -3, -2,
    -3, -2, -1, -2, -2, -2, -2, -2,
    -2, -3, -3, -3, -3, -5, -3, -3,
    -3, -5, -5, -5, -8, -8, -17, -4,
    -17, -4, -2, -3, -1, -1, -12, -12,
    -10, -2, -2, -5, -1, -2, -2, -1,
    -18, -15, -15, -15, -7, -15, -12, -12,
    -15, -10, -4, -12, -12, -11, -9, -10,
    -8, -7, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -2, -2, -2,
    -2, -3, -5, -3, -5, -4, -5, -7,
    -7, -15, -12, -8, -18, 0, -1, -1,
    -1, -6, -6, -2, -2, -16, -10, -7,
    -15, 0, -1, 0, -3, -4, -4, 0,
    0, -12, -1, -11, -1, -1, -1, -1,
    -1, -1, 8, 3, 3, 5, -2, -2,
    -16, -8, -2, -14, -1, 0, -2, -3,
    -1, -1, -1, -3, -5, -5, -5, -8,
    -2, -2, -5, -5, -3, -9, -3, -5,
    -3, -2, -1, -1, -1, -3, 3, -1,
    -1, 1, 0, 1, 1, -8, -8, -15,
    -7, -3, -12, -2, -2, -8, -1, -7,
    -2, -2, -2, -1, -2, -2, -1, -8,
    -8, -15, -7, -3, -12, -8, -8, -15,
    -7, -3, -12, -6, -6, -2, -2, -16,
    -10, -7, -15, 0, -1, 0, -3, -6,
    -6, -2, -2, -16, -10, -7, -15, 0,
    -1, 0, -3, -2, -2, -13, -6, -9,
    -19, -19, -7, -3, -14, -5, -4, -5,
    -2, -1, -1, -1, -2, -1, -3, -3,
    -3, -16, -10, -2, -3, -15, -3, -3,
    -1, -7, -3, -3, -1, -3, -3, -14,
    -5, -1, -11, -4, -5, -4, -3, -18,
    -5, -3, -5, -8, -5, -1, -1, -4,
    -5, -3, -8, -16, -5, -3, -8, -8,
    -2, -2, -1, -13, -5, -3, -10, -3,
    -3, -3, -3, -3, -7, -7, -4, -3,
    -18, -5, -3, -5, -8, -5, -1, -1,
    -13, -4, -2, -7, -2, -2, -2, -1,
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

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
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
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t ui_font_OswaldRegular16p1 = {
#else
lv_font_t ui_font_OswaldRegular16p1 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 18,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if UI_FONT_OSWALDREGULAR16P1*/

