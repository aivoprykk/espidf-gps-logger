/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --bpp 1 --size 16 --font /Users/aivo/Downloads/squareline/squareline_proj_1_54_01_s/assets/OpenSans_SemiCondensed-SemiBold.ttf -o ./ui_font_OpenSansSemiBold16p1.c --format lvgl -r 0x20-0x7f --no-compress --no-prefilter
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

#ifndef UI_FONT_OPENSANSSEMIBOLD16P1
#define UI_FONT_OPENSANSSEMIBOLD16P1 1
#endif

#if UI_FONT_OPENSANSSEMIBOLD16P1

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xff, 0xf,

    /* U+0022 "\"" */
    0xde, 0xf7, 0xb0,

    /* U+0023 "#" */
    0x12, 0x32, 0x32, 0x36, 0xff, 0x24, 0x24, 0xff,
    0x6c, 0x4c, 0x4c, 0x48,

    /* U+0024 "$" */
    0x10, 0x7b, 0xf6, 0x8d, 0x1e, 0xf, 0xf, 0x17,
    0x2f, 0xf0, 0x81, 0x0,

    /* U+0025 "%" */
    0x70, 0x8d, 0x98, 0xd9, 0xd, 0xb0, 0xda, 0xd,
    0xae, 0x75, 0xb0, 0x5b, 0x9, 0xb0, 0x9b, 0x19,
    0xb1, 0xe,

    /* U+0026 "&" */
    0x38, 0x36, 0x1b, 0xd, 0x83, 0x81, 0x81, 0xc7,
    0xf2, 0xcf, 0x63, 0xb3, 0xcf, 0xf0,

    /* U+0027 "'" */
    0xff,

    /* U+0028 "(" */
    0x36, 0x66, 0xcc, 0xcc, 0xcc, 0x46, 0x63,

    /* U+0029 ")" */
    0xc4, 0x66, 0x33, 0x33, 0x33, 0x66, 0x6c,

    /* U+002A "*" */
    0x30, 0xce, 0xff, 0x71, 0x64, 0x0,

    /* U+002B "+" */
    0x30, 0x60, 0xc1, 0x8f, 0xe6, 0xc, 0x18,

    /* U+002C "," */
    0xfa,

    /* U+002D "-" */
    0xf0,

    /* U+002E "." */
    0xf0,

    /* U+002F "/" */
    0xc, 0x30, 0x86, 0x10, 0xc3, 0x8, 0x61, 0x84,
    0x30,

    /* U+0030 "0" */
    0x38, 0xdb, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc7,
    0x8d, 0xb1, 0xc0,

    /* U+0031 "1" */
    0x19, 0xde, 0xb1, 0x8c, 0x63, 0x18, 0xc6, 0x30,

    /* U+0032 "2" */
    0x7d, 0x8c, 0x18, 0x30, 0x61, 0xc3, 0xc, 0x30,
    0xe1, 0x87, 0xf0,

    /* U+0033 "3" */
    0x7c, 0xc, 0x18, 0x30, 0xc7, 0x3, 0x3, 0x6,
    0xe, 0x37, 0xc0,

    /* U+0034 "4" */
    0xc, 0x1c, 0x1c, 0x3c, 0x2c, 0x6c, 0x4c, 0xcc,
    0xff, 0xc, 0xc, 0xc,

    /* U+0035 "5" */
    0x7c, 0xc1, 0x82, 0x4, 0xf, 0x83, 0x83, 0x6,
    0xe, 0x37, 0xc0,

    /* U+0036 "6" */
    0x1c, 0xc1, 0x86, 0xc, 0x1f, 0xbb, 0xe3, 0xc7,
    0x8d, 0xb1, 0xe0,

    /* U+0037 "7" */
    0xfe, 0xc, 0x10, 0x60, 0xc1, 0x6, 0xc, 0x30,
    0x60, 0xc3, 0x0,

    /* U+0038 "8" */
    0x7d, 0x8f, 0x1e, 0x36, 0xc7, 0x1f, 0x67, 0xc7,
    0x8f, 0x1b, 0xe0,

    /* U+0039 "9" */
    0x78, 0xdb, 0x1e, 0x3c, 0x79, 0xdf, 0x83, 0x6,
    0x18, 0x33, 0x80,

    /* U+003A ":" */
    0xf0, 0x3, 0xc0,

    /* U+003B ";" */
    0xf0, 0x3, 0xe8,

    /* U+003C "<" */
    0x2, 0x1c, 0xe3, 0xe, 0x7, 0x3, 0x81,

    /* U+003D "=" */
    0xfe, 0x0, 0x0, 0xf, 0xe0,

    /* U+003E ">" */
    0x81, 0xc0, 0xe0, 0x70, 0xe7, 0x38, 0x40,

    /* U+003F "?" */
    0xfa, 0x30, 0xc3, 0x1c, 0xe7, 0x18, 0x0, 0x6,
    0x18,

    /* U+0040 "@" */
    0x1f, 0x6, 0x31, 0x83, 0x37, 0xbd, 0xb7, 0xb6,
    0xf6, 0xde, 0xdb, 0xdb, 0x59, 0xb9, 0x80, 0x38,
    0x1, 0xf0,

    /* U+0041 "A" */
    0x1c, 0xe, 0x5, 0x6, 0xc3, 0x61, 0x90, 0x8c,
    0xfe, 0x63, 0x30, 0xb0, 0x78, 0x30,

    /* U+0042 "B" */
    0xfd, 0x8f, 0x1e, 0x3c, 0x7f, 0x33, 0xe3, 0xc7,
    0x8f, 0x3f, 0xe0,

    /* U+0043 "C" */
    0x1e, 0xc5, 0x86, 0xc, 0x18, 0x30, 0x60, 0xc0,
    0xc1, 0x81, 0xf0,

    /* U+0044 "D" */
    0xf8, 0xc6, 0xc6, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,
    0xc3, 0xc6, 0xc6, 0xf8,

    /* U+0045 "E" */
    0xff, 0xc, 0x30, 0xc3, 0xfc, 0x30, 0xc3, 0xc,
    0x3f,

    /* U+0046 "F" */
    0xff, 0xc, 0x30, 0xc3, 0xf, 0xf0, 0xc3, 0xc,
    0x30,

    /* U+0047 "G" */
    0x1f, 0x70, 0x60, 0xc0, 0xc0, 0xc0, 0xcf, 0xc3,
    0xc3, 0x63, 0x63, 0x3f,

    /* U+0048 "H" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3,
    0xc3, 0xc3, 0xc3, 0xc3,

    /* U+0049 "I" */
    0xff, 0xff, 0xff,

    /* U+004A "J" */
    0x18, 0xc6, 0x31, 0x8c, 0x63, 0x18, 0xc6, 0x31,
    0x8f, 0xc0,

    /* U+004B "K" */
    0xc6, 0xc6, 0xcc, 0xd8, 0xd8, 0xf0, 0xf8, 0xd8,
    0xcc, 0xcc, 0xc6, 0xc3,

    /* U+004C "L" */
    0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30, 0xc3, 0xc,
    0x3f,

    /* U+004D "M" */
    0xe0, 0xfc, 0x1f, 0xc7, 0xf8, 0xfd, 0x17, 0xa2,
    0xf6, 0xde, 0xd3, 0xca, 0x79, 0xcf, 0x39, 0xe2,
    0x30,

    /* U+004E "N" */
    0xe1, 0xf0, 0xfc, 0x7e, 0x3d, 0x9e, 0xcf, 0x37,
    0x9b, 0xc7, 0xe3, 0xf0, 0xf8, 0x70,

    /* U+004F "O" */
    0x3e, 0x31, 0x98, 0xd8, 0x3c, 0x1e, 0xf, 0x7,
    0x83, 0xc1, 0xb1, 0x98, 0xc7, 0xc0,

    /* U+0050 "P" */
    0xf9, 0x9f, 0x1e, 0x3c, 0x79, 0xbe, 0x60, 0xc1,
    0x83, 0x6, 0x0,

    /* U+0051 "Q" */
    0x3e, 0x31, 0x98, 0xd8, 0x3c, 0x1e, 0xf, 0x7,
    0x83, 0xc1, 0xb1, 0x98, 0xc7, 0xc0, 0x60, 0x18,
    0xe,

    /* U+0052 "R" */
    0xf8, 0xce, 0xc6, 0xc6, 0xc6, 0xcc, 0xf8, 0xc8,
    0xcc, 0xc4, 0xc6, 0xc7,

    /* U+0053 "S" */
    0x3e, 0x83, 0x6, 0xf, 0xf, 0xf, 0x7, 0x6,
    0xe, 0x37, 0xc0,

    /* U+0054 "T" */
    0xfe, 0x30, 0x60, 0xc1, 0x83, 0x6, 0xc, 0x18,
    0x30, 0x60, 0xc0,

    /* U+0055 "U" */
    0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,
    0xc3, 0xc3, 0x66, 0x3c,

    /* U+0056 "V" */
    0xc1, 0xe0, 0x98, 0xcc, 0x66, 0x31, 0x30, 0xd8,
    0x6c, 0x34, 0xe, 0x7, 0x3, 0x80,

    /* U+0057 "W" */
    0xc3, 0xe, 0x18, 0xd1, 0xc6, 0xce, 0x36, 0x59,
    0x32, 0xd8, 0xb2, 0xc5, 0x96, 0x3c, 0xb1, 0xc7,
    0xe, 0x38, 0x30, 0xc0,

    /* U+0058 "X" */
    0x43, 0x62, 0x26, 0x34, 0x1c, 0x18, 0x1c, 0x3c,
    0x36, 0x66, 0x63, 0xc3,

    /* U+0059 "Y" */
    0xc3, 0xc3, 0x66, 0x66, 0x3c, 0x3c, 0x18, 0x18,
    0x18, 0x18, 0x18, 0x18,

    /* U+005A "Z" */
    0xfe, 0xc, 0x30, 0x61, 0x83, 0xc, 0x18, 0x60,
    0xc3, 0x7, 0xf0,

    /* U+005B "[" */
    0xfc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcf,

    /* U+005C "\\" */
    0xc1, 0x6, 0x18, 0x20, 0xc3, 0x4, 0x18, 0x20,
    0xc3,

    /* U+005D "]" */
    0xf3, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3f,

    /* U+005E "^" */
    0x18, 0x18, 0x3c, 0x24, 0x66, 0x62, 0x43,

    /* U+005F "_" */
    0xfe,

    /* U+0060 "`" */
    0xd9, 0x80,

    /* U+0061 "a" */
    0x7c, 0x8c, 0x18, 0x37, 0xf8, 0xf1, 0xe7, 0x7e,

    /* U+0062 "b" */
    0xc1, 0x83, 0x7, 0xee, 0xd8, 0xf1, 0xe3, 0xc7,
    0x8f, 0xb7, 0xe0,

    /* U+0063 "c" */
    0x3d, 0x8c, 0x30, 0xc3, 0xc, 0x19, 0x3c,

    /* U+0064 "d" */
    0x6, 0xc, 0x1b, 0xf6, 0xf8, 0xf1, 0xe3, 0xc7,
    0x8d, 0xbb, 0xf0,

    /* U+0065 "e" */
    0x38, 0xdb, 0x1e, 0x3f, 0xf8, 0x30, 0x31, 0x3e,

    /* U+0066 "f" */
    0x3d, 0x86, 0x3e, 0x61, 0x86, 0x18, 0x61, 0x86,
    0x18,

    /* U+0067 "g" */
    0x7f, 0xc6, 0xc6, 0xc6, 0xc6, 0x3c, 0x40, 0x40,
    0x3e, 0xc3, 0xc3, 0xc7, 0x7c,

    /* U+0068 "h" */
    0xc1, 0x83, 0x7, 0xee, 0x78, 0xf1, 0xe3, 0xc7,
    0x8f, 0x1e, 0x30,

    /* U+0069 "i" */
    0xf3, 0xff, 0xff,

    /* U+006A "j" */
    0x33, 0x3, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3e,

    /* U+006B "k" */
    0xc1, 0x83, 0x6, 0x3c, 0xdb, 0x3c, 0x7c, 0xd9,
    0x9b, 0x36, 0x30,

    /* U+006C "l" */
    0xff, 0xff, 0xff,

    /* U+006D "m" */
    0xfd, 0xee, 0x73, 0xc6, 0x3c, 0x63, 0xc6, 0x3c,
    0x63, 0xc6, 0x3c, 0x63, 0xc6, 0x30,

    /* U+006E "n" */
    0xfd, 0xcf, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc6,

    /* U+006F "o" */
    0x38, 0xdb, 0x1e, 0x3c, 0x78, 0xf1, 0xb6, 0x38,

    /* U+0070 "p" */
    0xfd, 0xdb, 0x1e, 0x3c, 0x78, 0xf1, 0xf6, 0xfd,
    0x83, 0x6, 0xc, 0x0,

    /* U+0071 "q" */
    0x7e, 0xdf, 0x1e, 0x3c, 0x78, 0xf1, 0xb7, 0x7e,
    0xc, 0x18, 0x30, 0x60,

    /* U+0072 "r" */
    0xdf, 0xf9, 0x8c, 0x63, 0x18, 0xc0,

    /* U+0073 "s" */
    0x7f, 0xc, 0x3c, 0x78, 0x70, 0xe3, 0xf8,

    /* U+0074 "t" */
    0x23, 0x3e, 0xc6, 0x31, 0x8c, 0x63, 0xe,

    /* U+0075 "u" */
    0xc7, 0x8f, 0x1e, 0x3c, 0x78, 0xf1, 0xe7, 0x7e,

    /* U+0076 "v" */
    0xc7, 0x8d, 0x9b, 0x26, 0xc5, 0x8e, 0x1c, 0x18,

    /* U+0077 "w" */
    0xc6, 0x24, 0x66, 0x4e, 0x66, 0xb6, 0x6b, 0x46,
    0x94, 0x39, 0xc3, 0x9c, 0x31, 0x80,

    /* U+0078 "x" */
    0x66, 0xcc, 0xf1, 0xc1, 0x87, 0x8b, 0x33, 0xc6,

    /* U+0079 "y" */
    0xc7, 0x8f, 0x12, 0x66, 0xcd, 0xa, 0x1c, 0x38,
    0x20, 0xc1, 0xe, 0x0,

    /* U+007A "z" */
    0xf8, 0xc4, 0x62, 0x31, 0x98, 0xf8,

    /* U+007B "{" */
    0x19, 0x8c, 0x63, 0x1b, 0x6, 0x31, 0x8c, 0x63,
    0xc,

    /* U+007C "|" */
    0xff, 0xff, 0xff, 0xff,

    /* U+007D "}" */
    0xe0, 0xc3, 0xc, 0x30, 0xc0, 0xcc, 0x30, 0xc3,
    0xc, 0x33, 0x80,

    /* U+007E "~" */
    0xf3, 0x3c
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 57, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 66, .box_w = 2, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 108, .box_w = 5, .box_h = 4, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 7, .adv_w = 145, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 19, .adv_w = 131, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 31, .adv_w = 202, .box_w = 12, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 49, .adv_w = 162, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 63, .adv_w = 59, .box_w = 2, .box_h = 4, .ofs_x = 1, .ofs_y = 8},
    {.bitmap_index = 64, .adv_w = 79, .box_w = 4, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 71, .adv_w = 79, .box_w = 4, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 78, .adv_w = 124, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 84, .adv_w = 131, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 91, .adv_w = 65, .box_w = 2, .box_h = 4, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 92, .adv_w = 78, .box_w = 4, .box_h = 1, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 93, .adv_w = 66, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 94, .adv_w = 99, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 103, .adv_w = 131, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 114, .adv_w = 131, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 122, .adv_w = 131, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 133, .adv_w = 131, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 144, .adv_w = 131, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 156, .adv_w = 131, .box_w = 7, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 167, .adv_w = 131, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 178, .adv_w = 131, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 189, .adv_w = 131, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 200, .adv_w = 131, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 211, .adv_w = 66, .box_w = 2, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 214, .adv_w = 66, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 217, .adv_w = 131, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 224, .adv_w = 131, .box_w = 7, .box_h = 5, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 229, .adv_w = 131, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 236, .adv_w = 102, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 245, .adv_w = 202, .box_w = 11, .box_h = 13, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 263, .adv_w = 147, .box_w = 9, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 277, .adv_w = 148, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 288, .adv_w = 140, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 299, .adv_w = 162, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 311, .adv_w = 124, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 320, .adv_w = 119, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 329, .adv_w = 164, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 341, .adv_w = 167, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 353, .adv_w = 70, .box_w = 2, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 356, .adv_w = 69, .box_w = 5, .box_h = 15, .ofs_x = -1, .ofs_y = -3},
    {.bitmap_index = 366, .adv_w = 143, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 378, .adv_w = 119, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 387, .adv_w = 213, .box_w = 11, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 404, .adv_w = 176, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 418, .adv_w = 174, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 432, .adv_w = 138, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 443, .adv_w = 174, .box_w = 9, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 460, .adv_w = 144, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 472, .adv_w = 125, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 483, .adv_w = 124, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 494, .adv_w = 164, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 506, .adv_w = 140, .box_w = 9, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 520, .adv_w = 216, .box_w = 13, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 540, .adv_w = 135, .box_w = 8, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 552, .adv_w = 131, .box_w = 8, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 564, .adv_w = 124, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 575, .adv_w = 82, .box_w = 4, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 582, .adv_w = 99, .box_w = 6, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 591, .adv_w = 82, .box_w = 4, .box_h = 14, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 598, .adv_w = 133, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 605, .adv_w = 112, .box_w = 7, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 606, .adv_w = 78, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 10},
    {.bitmap_index = 608, .adv_w = 130, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 616, .adv_w = 141, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 627, .adv_w = 111, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 634, .adv_w = 141, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 645, .adv_w = 130, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 653, .adv_w = 81, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 662, .adv_w = 125, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 675, .adv_w = 142, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 686, .adv_w = 65, .box_w = 2, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 689, .adv_w = 65, .box_w = 4, .box_h = 16, .ofs_x = -1, .ofs_y = -4},
    {.bitmap_index = 697, .adv_w = 129, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 708, .adv_w = 64, .box_w = 2, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 711, .adv_w = 213, .box_w = 12, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 725, .adv_w = 142, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 733, .adv_w = 138, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 741, .adv_w = 141, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 753, .adv_w = 141, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = -4},
    {.bitmap_index = 765, .adv_w = 97, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 771, .adv_w = 108, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 778, .adv_w = 87, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 785, .adv_w = 142, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 793, .adv_w = 120, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 801, .adv_w = 186, .box_w = 12, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 815, .adv_w = 122, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 823, .adv_w = 120, .box_w = 7, .box_h = 13, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 835, .adv_w = 103, .box_w = 5, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 841, .adv_w = 96, .box_w = 5, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 850, .adv_w = 123, .box_w = 2, .box_h = 16, .ofs_x = 3, .ofs_y = -4},
    {.bitmap_index = 854, .adv_w = 96, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 865, .adv_w = 131, .box_w = 7, .box_h = 2, .ofs_x = 1, .ofs_y = 5}
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
    .kern_dsc = NULL,
    .kern_scale = 0,
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
const lv_font_t ui_font_OpenSansSemiBold16p1 = {
#else
lv_font_t ui_font_OpenSansSemiBold16p1 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 17,          /*The maximum line height required by the font*/
    .base_line = 4,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if UI_FONT_OPENSANSSEMIBOLD16P1*/

