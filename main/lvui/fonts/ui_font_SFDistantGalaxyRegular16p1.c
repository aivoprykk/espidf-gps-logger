/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --bpp 1 --size 16 --font /Users/aivo/Downloads/squareline/squareline_proj_1_54_01_s/assets/SF_Distant_Galaxy.ttf -o ./ui_font_SFDistantGalaxyRegular16p1.c --format lvgl -r 0x20-0x7f --no-compress --no-prefilter
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

#ifndef UI_FONT_SFDISTANTGALAXYREGULAR16P1
#define UI_FONT_SFDISTANTGALAXYREGULAR16P1 1
#endif

#if UI_FONT_SFDISTANTGALAXYREGULAR16P1

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xff, 0xfc, 0xfc,

    /* U+0022 "\"" */
    0xff, 0xff, 0xd2, 0x0,

    /* U+0023 "#" */
    0x19, 0x86, 0x67, 0xfd, 0xff, 0x1b, 0xc, 0xc7,
    0xff, 0xfe, 0x33, 0xd, 0x80,

    /* U+0024 "$" */
    0xe, 0x1, 0xc0, 0x7f, 0x9f, 0xf7, 0xfe, 0xf8,
    0x1f, 0xc1, 0xfc, 0x1f, 0x81, 0xf3, 0xfe, 0x7f,
    0xcf, 0xf0, 0x1c, 0x3, 0x80,

    /* U+0025 "%" */
    0x73, 0x6, 0xdc, 0x36, 0xc1, 0xbe, 0x7, 0x70,
    0x3, 0x0, 0x3b, 0xc1, 0xf3, 0xd, 0x98, 0xec,
    0xc7, 0x3c,

    /* U+0026 "&" */
    0x3f, 0xf7, 0xff, 0x7f, 0xff, 0x0, 0x7e, 0x7,
    0xe0, 0x7e, 0xff, 0xf, 0x7f, 0xf7, 0xff, 0x3f,
    0xf0,

    /* U+0027 "'" */
    0xff, 0xa0,

    /* U+0028 "(" */
    0x37, 0x7e, 0xee, 0xee, 0xee, 0xe7, 0x73,

    /* U+0029 ")" */
    0xce, 0xe7, 0x77, 0x77, 0x77, 0x7e, 0xec,

    /* U+002A "*" */
    0x33, 0xf7, 0xbf, 0x30,

    /* U+002B "+" */
    0x37, 0xfe, 0x63, 0x0,

    /* U+002C "," */
    0xff, 0xa0,

    /* U+002D "-" */
    0xff, 0xc0,

    /* U+002E "." */
    0xff, 0x80,

    /* U+002F "/" */
    0xe, 0x1c, 0x30, 0xe1, 0xc7, 0xe, 0x1c, 0x70,
    0xe1, 0xc7, 0xe, 0x0,

    /* U+0030 "0" */
    0xf, 0x3, 0xfc, 0x7f, 0xef, 0x9f, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0x9f, 0x7f, 0xe3, 0xfc, 0xf,
    0x0,

    /* U+0031 "1" */
    0x7f, 0xfe, 0xf7, 0xbd, 0xef, 0x7b, 0xde,

    /* U+0032 "2" */
    0xff, 0xcf, 0xfe, 0xff, 0xe0, 0xe, 0x3f, 0xe7,
    0xfe, 0xff, 0xcf, 0x0, 0xff, 0xff, 0xff, 0xff,
    0xf0,

    /* U+0033 "3" */
    0xff, 0x9f, 0xfb, 0xff, 0x0, 0xf3, 0xfc, 0x7f,
    0x8f, 0xf0, 0xe, 0xff, 0xdf, 0xfb, 0xfe, 0x0,

    /* U+0034 "4" */
    0x1, 0xe7, 0x9e, 0x79, 0xe7, 0x9e, 0x79, 0xe7,
    0x9e, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x1e, 0x1,
    0xe0,

    /* U+0035 "5" */
    0xff, 0xef, 0xfe, 0xff, 0xef, 0x0, 0xff, 0xcf,
    0xfe, 0xff, 0xe0, 0xf, 0xff, 0xef, 0xfe, 0xff,
    0xc0,

    /* U+0036 "6" */
    0x3f, 0xe7, 0xfe, 0xff, 0xee, 0x0, 0xff, 0xcf,
    0xfe, 0xff, 0xef, 0xf, 0xff, 0xe7, 0xfe, 0x3f,
    0xc0,

    /* U+0037 "7" */
    0xff, 0xff, 0xfb, 0xff, 0x3, 0xc0, 0xf0, 0x3e,
    0x7, 0x81, 0xf0, 0x3c, 0xf, 0x3, 0xe0, 0x0,

    /* U+0038 "8" */
    0x3f, 0x87, 0xfe, 0x7f, 0xe7, 0xe, 0x7f, 0xe7,
    0xfc, 0xff, 0xef, 0xe, 0xff, 0xe7, 0xfe, 0x3f,
    0xc0,

    /* U+0039 "9" */
    0x3f, 0xc7, 0xfe, 0xff, 0xef, 0xf, 0xff, 0xf7,
    0xff, 0x3f, 0xf0, 0xf, 0xff, 0xef, 0xfe, 0xff,
    0xc0,

    /* U+003A ":" */
    0xff, 0x81, 0xff,

    /* U+003B ";" */
    0xff, 0x81, 0xff, 0x40,

    /* U+003C "<" */
    0x77, 0x6e, 0x67, 0x70,

    /* U+003D "=" */
    0xff, 0xfc, 0x7, 0xff, 0xe0,

    /* U+003E ">" */
    0x67, 0x73, 0x77, 0x60,

    /* U+003F "?" */
    0xff, 0x3f, 0xef, 0xf8, 0xe, 0x3f, 0x9f, 0xef,
    0xf3, 0x80, 0xe0, 0x38, 0xe, 0x0,

    /* U+0040 "@" */
    0x3e, 0x31, 0xb0, 0x73, 0x99, 0x4d, 0xf7, 0xde,
    0xc6, 0x3c, 0x0,

    /* U+0041 "A" */
    0x1f, 0xc1, 0xfc, 0x3f, 0xc3, 0xfc, 0x3d, 0xe3,
    0xde, 0x7d, 0xe7, 0xfe, 0x7f, 0xf7, 0xff, 0xf8,
    0xf0,

    /* U+0042 "B" */
    0xff, 0x9f, 0xfb, 0xff, 0xf8, 0xff, 0x1f, 0xff,
    0xbf, 0xff, 0x8f, 0xff, 0xff, 0xff, 0xfe, 0x0,

    /* U+0043 "C" */
    0x1f, 0xcf, 0xf7, 0xff, 0xe0, 0xf0, 0x3c, 0xf,
    0x3, 0xe0, 0x7f, 0xcf, 0xf1, 0xfc,

    /* U+0044 "D" */
    0xff, 0xf, 0xfc, 0xff, 0xef, 0x1f, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0x1f, 0xff, 0xef, 0xfc, 0xff,
    0x0,

    /* U+0045 "E" */
    0xff, 0xff, 0xff, 0xff, 0xf8, 0xf, 0xf1, 0xfe,
    0x3f, 0xc7, 0x80, 0xff, 0xff, 0xff, 0xff, 0x80,

    /* U+0046 "F" */
    0xff, 0xff, 0xff, 0xff, 0xf8, 0xf, 0xf1, 0xfe,
    0x3f, 0xc7, 0x80, 0xf0, 0x1e, 0x3, 0xc0, 0x0,

    /* U+0047 "G" */
    0x1f, 0x7, 0xf1, 0xfc, 0x7c, 0xf, 0x1, 0xe3,
    0xfc, 0x7f, 0xcf, 0x7f, 0xe7, 0xfc, 0x7f, 0x80,

    /* U+0048 "H" */
    0xf1, 0xfe, 0x3f, 0xc7, 0xf8, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x8f, 0xf1, 0xfe, 0x3f, 0xc7, 0x80,

    /* U+0049 "I" */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf0,

    /* U+004A "J" */
    0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc7, 0x8f, 0x1e,
    0x3c, 0x7f, 0xf7, 0xcf, 0x0,

    /* U+004B "K" */
    0xf3, 0xef, 0x3e, 0xf7, 0xcf, 0xf8, 0xff, 0x8f,
    0xf0, 0xff, 0xf, 0xf8, 0xff, 0xff, 0x3f, 0xf1,
    0xf0,

    /* U+004C "L" */
    0xf0, 0x3c, 0xf, 0x3, 0xc0, 0xf0, 0x3c, 0xf,
    0x3, 0xc0, 0xff, 0xff, 0xff, 0xfc,

    /* U+004D "M" */
    0xf0, 0x7f, 0xc7, 0xff, 0x7f, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf7, 0x7f, 0xbb,
    0xfc, 0x9e,

    /* U+004E "N" */
    0xe1, 0xff, 0x3f, 0xf7, 0xff, 0xff, 0xff, 0xff,
    0xfd, 0xff, 0x9f, 0xf1, 0xfe, 0x3f, 0xc7, 0x80,

    /* U+004F "O" */
    0xf, 0x3, 0xfc, 0x7f, 0xef, 0x9f, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0x9f, 0x7f, 0xe3, 0xfc, 0xf,
    0x0,

    /* U+0050 "P" */
    0xff, 0x9f, 0xfb, 0xff, 0x80, 0xf0, 0x1f, 0xff,
    0xff, 0xf7, 0xfc, 0xf0, 0x1e, 0x3, 0xc0, 0x0,

    /* U+0051 "Q" */
    0xf, 0x3, 0xfc, 0x7f, 0xef, 0x9f, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0x9f, 0x7f, 0xf3, 0xff, 0x1f,
    0xf0,

    /* U+0052 "R" */
    0xff, 0x8f, 0xfc, 0xff, 0xe0, 0x1e, 0x1, 0xef,
    0xfe, 0xff, 0xcf, 0xf8, 0xff, 0xff, 0x3f, 0xf1,
    0xf0,

    /* U+0053 "S" */
    0x1f, 0xe7, 0xfc, 0xff, 0xbe, 0x3, 0xf0, 0x7f,
    0x7, 0xe0, 0x7c, 0xff, 0x9f, 0xf3, 0xfc, 0x0,

    /* U+0054 "T" */
    0xff, 0xff, 0xff, 0xff, 0x87, 0x80, 0xf0, 0x1e,
    0x3, 0xc0, 0x78, 0xf, 0x1, 0xe0, 0x3c, 0x0,

    /* U+0055 "U" */
    0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0x9f, 0x7f, 0xe3, 0xfc, 0xf,
    0x0,

    /* U+0056 "V" */
    0xf8, 0xf7, 0x8f, 0x78, 0xf7, 0x9e, 0x7d, 0xe3,
    0xde, 0x3d, 0xe3, 0xfc, 0x3f, 0xc1, 0xfc, 0x1f,
    0xc0,

    /* U+0057 "W" */
    0xfb, 0xe7, 0xbd, 0xf7, 0x9e, 0xfb, 0xcf, 0xff,
    0xe7, 0xff, 0xf1, 0xff, 0xf0, 0xff, 0xf8, 0x7f,
    0xfc, 0x3f, 0x7e, 0xf, 0xbe, 0x7, 0xdf, 0x0,

    /* U+0058 "X" */
    0x7c, 0xf9, 0xf3, 0xe3, 0xff, 0x7, 0xf8, 0xf,
    0xc0, 0x1e, 0x0, 0xfc, 0x7, 0xf8, 0x3f, 0xf1,
    0xf3, 0xe7, 0xcf, 0x80,

    /* U+0059 "Y" */
    0xf9, 0xf7, 0x9e, 0x7f, 0xe3, 0xfc, 0x3f, 0xc1,
    0xf8, 0x1f, 0x80, 0xf0, 0xf, 0x0, 0xf0, 0xf,
    0x0,

    /* U+005A "Z" */
    0xff, 0xef, 0xfe, 0xff, 0xc0, 0x78, 0xf, 0x81,
    0xf0, 0x1f, 0x3, 0xe0, 0x3f, 0xf7, 0xff, 0xff,
    0xf0,

    /* U+005B "[" */
    0xff, 0xfe, 0xee, 0xee, 0xee, 0xef, 0xff,

    /* U+005C "\\" */
    0xe1, 0xc1, 0xc3, 0x87, 0x7, 0xe, 0x1c, 0x1c,
    0x38, 0x30, 0x70, 0xe0,

    /* U+005D "]" */
    0xff, 0xfe, 0x73, 0x9c, 0xe7, 0x39, 0xcf, 0xff,
    0xfc,

    /* U+005E "^" */
    0x18, 0x70, 0xb3, 0x24, 0x60,

    /* U+005F "_" */
    0xff, 0xc0,

    /* U+0060 "`" */
    0x43, 0xe0,

    /* U+0061 "a" */
    0x1f, 0xc1, 0xfc, 0x3f, 0xc3, 0xfc, 0x3d, 0xe3,
    0xde, 0x7d, 0xe7, 0xfe, 0x7f, 0xf7, 0xff, 0xf8,
    0xf0,

    /* U+0062 "b" */
    0xff, 0x9f, 0xfb, 0xff, 0xf8, 0xff, 0x1f, 0xff,
    0xbf, 0xff, 0x8f, 0xff, 0xff, 0xff, 0xfe, 0x0,

    /* U+0063 "c" */
    0x1f, 0xcf, 0xf7, 0xff, 0xe0, 0xf0, 0x3c, 0xf,
    0x3, 0xe0, 0x7f, 0xcf, 0xf1, 0xfc,

    /* U+0064 "d" */
    0xff, 0xf, 0xfc, 0xff, 0xef, 0x1f, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0x1f, 0xff, 0xef, 0xfc, 0xff,
    0x0,

    /* U+0065 "e" */
    0xff, 0xff, 0xff, 0xff, 0xf8, 0xf, 0xf1, 0xfe,
    0x3f, 0xc7, 0x80, 0xff, 0xff, 0xff, 0xff, 0x80,

    /* U+0066 "f" */
    0xff, 0xff, 0xff, 0xff, 0xf8, 0xf, 0xf1, 0xfe,
    0x3f, 0xc7, 0x80, 0xf0, 0x1e, 0x3, 0xc0, 0x0,

    /* U+0067 "g" */
    0x1f, 0xe7, 0xfd, 0xff, 0xfc, 0xf, 0x1, 0xe3,
    0xfc, 0x7f, 0xcf, 0x7f, 0xe7, 0xfc, 0x7f, 0x80,

    /* U+0068 "h" */
    0xf1, 0xfe, 0x3f, 0xc7, 0xf8, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x8f, 0xf1, 0xfe, 0x3f, 0xc7, 0x80,

    /* U+0069 "i" */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf0,

    /* U+006A "j" */
    0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xc7, 0x8f, 0x1e,
    0x3c, 0x7f, 0xf7, 0xcf, 0x0,

    /* U+006B "k" */
    0xf3, 0xef, 0x3e, 0xf7, 0xcf, 0xf8, 0xff, 0x8f,
    0xf0, 0xff, 0xf, 0xf8, 0xff, 0xff, 0x3f, 0xf1,
    0xf0,

    /* U+006C "l" */
    0xf0, 0x3c, 0xf, 0x3, 0xc0, 0xf0, 0x3c, 0xf,
    0x3, 0xc0, 0xff, 0xff, 0xff, 0xfc,

    /* U+006D "m" */
    0xf0, 0x7f, 0xc7, 0xff, 0x7f, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf7, 0x7f, 0xbb,
    0xfc, 0x9e,

    /* U+006E "n" */
    0xf1, 0xff, 0x3f, 0xf7, 0xfe, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xbf, 0xf7, 0xfe, 0x7f, 0xc7, 0x80,

    /* U+006F "o" */
    0xf, 0x3, 0xfc, 0x7f, 0xef, 0x9f, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0x9f, 0x7f, 0xe3, 0xfc, 0xf,
    0x0,

    /* U+0070 "p" */
    0xff, 0x9f, 0xfb, 0xff, 0xf8, 0xff, 0x1f, 0xff,
    0xff, 0xf7, 0xfc, 0xf0, 0x1e, 0x3, 0xc0, 0x0,

    /* U+0071 "q" */
    0xf, 0x3, 0xfc, 0x7f, 0xef, 0x9f, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0x9f, 0x7f, 0xf3, 0xff, 0x1f,
    0xf0,

    /* U+0072 "r" */
    0xff, 0x8f, 0xfc, 0xff, 0xef, 0x1e, 0xf1, 0xef,
    0xfe, 0xff, 0xcf, 0xf8, 0xff, 0xff, 0x3f, 0xf1,
    0xf0,

    /* U+0073 "s" */
    0x1f, 0xe7, 0xfc, 0xff, 0xbe, 0x3, 0xf0, 0x7f,
    0x7, 0xe0, 0x7c, 0xff, 0x9f, 0xf3, 0xfc, 0x0,

    /* U+0074 "t" */
    0xff, 0xff, 0xff, 0xff, 0x87, 0x80, 0xf0, 0x1e,
    0x3, 0xc0, 0x78, 0xf, 0x1, 0xe0, 0x3c, 0x0,

    /* U+0075 "u" */
    0xf0, 0xff, 0xf, 0xf0, 0xff, 0xf, 0xf0, 0xff,
    0xf, 0xf0, 0xff, 0x9f, 0x7f, 0xe3, 0xfc, 0xf,
    0x0,

    /* U+0076 "v" */
    0xf8, 0xf7, 0x8f, 0x78, 0xf7, 0x9e, 0x7d, 0xe3,
    0xde, 0x3d, 0xe3, 0xfc, 0x3f, 0xc1, 0xfc, 0x1f,
    0xc0,

    /* U+0077 "w" */
    0xfb, 0xe7, 0xbd, 0xf7, 0x9e, 0xfb, 0xcf, 0xff,
    0xe7, 0xff, 0xf1, 0xff, 0xf0, 0xff, 0xf8, 0x7f,
    0xfc, 0x3f, 0x7e, 0xf, 0xbe, 0x7, 0xdf, 0x0,

    /* U+0078 "x" */
    0x7c, 0xf9, 0xf3, 0xe3, 0xff, 0x7, 0xf8, 0xf,
    0xc0, 0x1e, 0x0, 0xfc, 0x7, 0xf8, 0x3f, 0xf1,
    0xf3, 0xe7, 0xcf, 0x80,

    /* U+0079 "y" */
    0xf9, 0xf7, 0x9e, 0x7f, 0xe3, 0xfc, 0x3f, 0xc1,
    0xf8, 0x1f, 0x80, 0xf0, 0xf, 0x0, 0xf0, 0xf,
    0x0,

    /* U+007A "z" */
    0xff, 0xef, 0xfe, 0xff, 0xc0, 0x78, 0xf, 0x81,
    0xf0, 0x1f, 0x3, 0xe0, 0x3f, 0xf7, 0xff, 0xff,
    0xf0,

    /* U+007B "{" */
    0x1c, 0xf3, 0xce, 0x38, 0xe3, 0xbc, 0x38, 0xe3,
    0x8f, 0x3c, 0x70,

    /* U+007C "|" */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xc0,

    /* U+007D "}" */
    0xf3, 0xef, 0x8e, 0x38, 0xe3, 0x87, 0x38, 0xe3,
    0xbe, 0xfb, 0xc0,

    /* U+007E "~" */
    0xe1, 0xc0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 96, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 53, .box_w = 2, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 107, .box_w = 6, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 8, .adv_w = 170, .box_w = 10, .box_h = 10, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 21, .adv_w = 181, .box_w = 11, .box_h = 15, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 42, .adv_w = 216, .box_w = 13, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 60, .adv_w = 181, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 77, .adv_w = 53, .box_w = 3, .box_h = 5, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 79, .adv_w = 75, .box_w = 4, .box_h = 14, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 86, .adv_w = 75, .box_w = 4, .box_h = 14, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 93, .adv_w = 97, .box_w = 6, .box_h = 5, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 97, .adv_w = 96, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 101, .adv_w = 53, .box_w = 3, .box_h = 5, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 103, .adv_w = 96, .box_w = 5, .box_h = 2, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 105, .adv_w = 53, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 107, .adv_w = 113, .box_w = 7, .box_h = 13, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 119, .adv_w = 192, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 136, .adv_w = 92, .box_w = 5, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 143, .adv_w = 192, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 160, .adv_w = 181, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 176, .adv_w = 192, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 193, .adv_w = 192, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 210, .adv_w = 192, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 227, .adv_w = 192, .box_w = 11, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 243, .adv_w = 192, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 260, .adv_w = 192, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 277, .adv_w = 53, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 280, .adv_w = 53, .box_w = 3, .box_h = 10, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 284, .adv_w = 75, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 288, .adv_w = 117, .box_w = 7, .box_h = 5, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 293, .adv_w = 75, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 297, .adv_w = 160, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 311, .adv_w = 156, .box_w = 9, .box_h = 9, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 322, .adv_w = 203, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 339, .adv_w = 192, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 355, .adv_w = 171, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 369, .adv_w = 192, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 386, .adv_w = 181, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 402, .adv_w = 181, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 418, .adv_w = 192, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 434, .adv_w = 192, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 450, .adv_w = 75, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 456, .adv_w = 112, .box_w = 7, .box_h = 14, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 469, .adv_w = 192, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 486, .adv_w = 171, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 500, .adv_w = 213, .box_w = 13, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 518, .adv_w = 192, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 534, .adv_w = 192, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 551, .adv_w = 192, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 567, .adv_w = 203, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 584, .adv_w = 203, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 601, .adv_w = 181, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 617, .adv_w = 192, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 633, .adv_w = 192, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 650, .adv_w = 203, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 667, .adv_w = 275, .box_w = 17, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 691, .adv_w = 224, .box_w = 14, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 711, .adv_w = 211, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 728, .adv_w = 192, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 745, .adv_w = 75, .box_w = 4, .box_h = 14, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 752, .adv_w = 113, .box_w = 7, .box_h = 13, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 764, .adv_w = 75, .box_w = 5, .box_h = 14, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 773, .adv_w = 122, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 6},
    {.bitmap_index = 778, .adv_w = 85, .box_w = 5, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 780, .adv_w = 107, .box_w = 6, .box_h = 2, .ofs_x = 0, .ofs_y = 12},
    {.bitmap_index = 782, .adv_w = 203, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 799, .adv_w = 192, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 815, .adv_w = 171, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 829, .adv_w = 192, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 846, .adv_w = 181, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 862, .adv_w = 181, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 878, .adv_w = 192, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 894, .adv_w = 192, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 910, .adv_w = 75, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 916, .adv_w = 112, .box_w = 7, .box_h = 14, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 929, .adv_w = 192, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 946, .adv_w = 171, .box_w = 10, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 960, .adv_w = 213, .box_w = 13, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 978, .adv_w = 192, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 994, .adv_w = 192, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1011, .adv_w = 192, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1027, .adv_w = 203, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1044, .adv_w = 203, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1061, .adv_w = 181, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1077, .adv_w = 192, .box_w = 11, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1093, .adv_w = 192, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1110, .adv_w = 203, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1127, .adv_w = 275, .box_w = 17, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1151, .adv_w = 224, .box_w = 14, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1171, .adv_w = 211, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1188, .adv_w = 192, .box_w = 12, .box_h = 11, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1205, .adv_w = 96, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1216, .adv_w = 59, .box_w = 3, .box_h = 14, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 1222, .adv_w = 96, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 1233, .adv_w = 96, .box_w = 5, .box_h = 2, .ofs_x = 1, .ofs_y = 12}
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


/*Map glyph_ids to kern left classes*/
static const uint8_t kern_left_class_mapping[] =
{
    0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 2, 0, 3, 4, 5, 6, 7,
    8, 9, 10, 0, 0, 0, 0, 0,
    0, 0, 11, 12, 13, 2, 0, 14,
    15, 0, 0, 0, 16, 17, 0, 0,
    2, 18, 19, 20, 21, 22, 23, 24,
    24, 25, 26, 27, 0, 0, 0, 0,
    0, 0, 11, 12, 13, 2, 0, 14,
    0, 0, 0, 0, 16, 17, 0, 0,
    2, 18, 19, 20, 21, 22, 23, 24,
    24, 25, 26, 27, 0, 0, 0, 0
};

/*Map glyph_ids to kern right classes*/
static const uint8_t kern_right_class_mapping[] =
{
    0, 0, 0, 0, 0, 1, 0, 2,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 3, 4, 5, 6, 7, 0, 8,
    9, 10, 11, 0, 0, 0, 0, 0,
    0, 0, 12, 0, 3, 0, 0, 0,
    3, 0, 0, 13, 0, 0, 0, 0,
    3, 14, 3, 14, 15, 16, 17, 18,
    18, 19, 20, 21, 0, 0, 0, 0,
    0, 0, 12, 0, 3, 0, 0, 0,
    3, 0, 0, 13, 0, 0, 0, 0,
    3, 0, 3, 0, 15, 16, 17, 18,
    18, 19, 20, 21, 0, 0, 0, 0
};

/*Kern values between classes*/
static const int8_t kern_class_values[] =
{
    -20, 0, 0, 0, -7, 0, -15, 0,
    0, 0, 0, -24, -36, 0, -17, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -8, 0, -6, 0, 0,
    -13, -37, 0, 0, -7, 0, -13, -34,
    -31, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -6, 0, -19, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -16, 0, 0, 0, 0,
    0, -18, -17, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -6, 0, 0, 0,
    -8, 0, 0, -8, 0, 0, -7, -9,
    0, 0, 0, 0, -6, 0, 0, 0,
    0, -8, 0, -6, -7, -19, 0, 0,
    -10, 0, -7, -9, -11, -7, 0, 0,
    0, -6, 0, 0, 0, 0, -8, 0,
    -6, -7, -18, 0, 0, -10, 0, -7,
    -9, -11, -7, -41, -20, -35, -21, -13,
    -6, -36, -21, -7, -23, -23, -41, -44,
    0, -41, -9, 0, -6, -7, -10, -6,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, -16, 0, 0, 0,
    0, -8, -17, -20, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, -16, 0, 0, 0, 0, 0, -18,
    -17, 0, 0, 0, -13, -15, 0, 0,
    -10, 0, 0, 0, -6, 0, 0, 0,
    0, -27, -11, -34, 0, -39, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -7,
    0, 0, -6, -18, 0, 0, -9, 0,
    -14, -19, -26, -6, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, -6,
    0, -23, 0, -10, 0, -13, -6, -21,
    0, -6, -6, 0, -30, -44, 0, -23,
    -9, 0, -5, -7, -10, -6, 0, 0,
    0, -17, 0, 0, 0, 0, 0, 0,
    -7, 0, 0, 0, 0, -27, 0, -18,
    0, -37, 0, 0, 0, 0, -14, 0,
    0, 0, 0, 0, 0, -7, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    -8, 0, -11, -24, -6, -8, -11, 0,
    -9, 0, -15, -7, -11, -6, -8, -58,
    -9, -32, -10, -65, -8, -20, 0, 0,
    0, -11, 0, -17, 0, 0, 0, 0,
    -24, -42, 0, -20, -7, 0, -13, -28,
    -26, 0, 0, 0, 0, -12, 0, 0,
    0, 0, 0, 0, -5, 0, 0, 0,
    0, -15, 0, -21, 0, -39, 0, 0,
    0, 0, -6, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, -16, -10, 0,
    -16, 0, -29, 0, -17, 0, 0, 0,
    -7, 0, -15, 0, 0, 0, 0, -24,
    -36, 0, -17, -43, 0, 0, 0, 0,
    0, -20, 0, -7, 0, -10, 0, -19,
    0, 0, 0, 0, -27, -41, 0, -20,
    -6, 0, 0, 0, -7, 0, 0, 0,
    0, 0, 0, 0, -6, 0, 0, 0,
    0, -11, -35, 0, 0, 0, 0, 0,
    0, 0, 0, -23, 0, -13, -8, -6,
    0, -23, 0, 0, -8, -7, -34, -36,
    0, -23, 0, 0, 0, 0, 0, 0,
    0, -18, -34, -16, 0, 0, -25, -18,
    0, -17, -8, 0, -6, 0, 0, 0,
    0, 0, 0, 0, 0, -37, -17, -31,
    -19, -10, 0, -33, -17, 0, -19, -19,
    -39, -42, 0, -37, -7, 0, 0, 0,
    -8, 0, 0, 0, 0, -15, 0, 0,
    0, 0, 0, 0, -7, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0
};


/*Collect the kern class' data in one place*/
static const lv_font_fmt_txt_kern_classes_t kern_classes =
{
    .class_pair_values   = kern_class_values,
    .left_class_mapping  = kern_left_class_mapping,
    .right_class_mapping = kern_right_class_mapping,
    .left_class_cnt      = 27,
    .right_class_cnt     = 21,
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
    .kern_dsc = &kern_classes,
    .kern_scale = 16,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 1,
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
const lv_font_t ui_font_SFDistantGalaxyRegular16p1 = {
#else
lv_font_t ui_font_SFDistantGalaxyRegular16p1 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 17,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if UI_FONT_SFDISTANTGALAXYREGULAR16P1*/

