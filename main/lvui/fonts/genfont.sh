cd $(dirname $0)
d=4;
[ "$1" != "" ] && d=$1
sd=../../srcfonts
for i in 16 24 30 36 60 84; do lv_font_conv --bpp ${d} --size ${i} --font $sd/OpenSans-Bold.ttf  -o ./ui_font_OpenSansBold${i}p${d}.c --format lvgl -r 0x20-0x7f --no-compress --no-prefilter; done
for i in 16; do lv_font_conv --bpp ${d} --size ${i} --font $sd/SF_Distant_Galaxy.ttf -o ./ui_font_SFDistantGalaxyRegular${i}p${d}.c --format lvgl -r 0x20-0x7f --no-compress --no-prefilter; done
for i in 14 16 20 24 36 48 60 84 96 100 108; do lv_font_conv --bpp $d --size $i --font $sd/Oswald-Regular.ttf -o ./ui_font_OswaldRegular${i}p${d}.c --format lvgl -r 0x20-0x7f --no-compress --no-prefilter; done
for i in 16; do lv_font_conv --bpp ${d} --size ${i} --font $sd/OpenSans_SemiCondensed-SemiBold.ttf -o ./ui_font_OpenSansSemiBold${i}p${d}.c --format lvgl -r 0x20-0x7f --no-compress --no-prefilter; done
