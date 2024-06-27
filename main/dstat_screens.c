#include "dstat_screens.h"

#include "private.h"
#include "str.h"
#include "context.h"

extern struct context_s m_context;
extern struct context_rtc_s m_context_rtc;
static struct gps_context_s * m_gps_data = &m_context.gps;

static double get_avg(double *b) {
    return (b[9] + b[8] + b[7] + b[6] + b[5]) / 5;
}

static double S10_display_last(void) {
    return m_gps_data->S10.display_last_run;
}
static double S10_display_max(void) {
    return m_gps_data->S10.display_max_speed;
}
static double S10_display_avg(void) {
    return m_gps_data->S10.avg_5runs;
}
static size_t S10_display_max_time(char *p1) {
    return time_to_char_hm(m_gps_data->S10.time_hour[9], m_gps_data->S10.time_min[9], p1);
}
static double S10_r1_display(void) {
    return m_gps_data->S10.display_speed[9];
}
static double S10_r2_display(void) {
    return m_gps_data->S10.display_speed[8];
}
static double S10_r3_display(void) {
    return m_gps_data->S10.display_speed[7];
}
static double S10_r4_display(void) {
    return m_gps_data->S10.display_speed[6];
}
static double S10_r5_display(void) {
    return m_gps_data->S10.display_speed[5];
}
static double S2_display_last(void) {
    return m_gps_data->S2.display_last_run;
}
static double S2_display_max(void) {
    return m_gps_data->S2.display_max_speed;
}
static double S2_display_avg(void) {
    return m_gps_data->S2.avg_5runs;
}
static size_t S2_display_max_time(char *p1) {
    return time_to_char_hm(m_gps_data->S2.time_hour[9], m_gps_data->S2.time_min[9], p1);
}
static double S1800_display_last(void) {
    return m_gps_data->S1800.display_last_run;
}
static double S1800_display_max(void) {
    return m_gps_data->S1800.display_max_speed;
}
static double S1800_display_avg(void) {
    return m_gps_data->S1800.avg_5runs;
}
static size_t S1800_display_max_time(char *p1) {
    return time_to_char_hm(m_gps_data->S1800.time_hour[9], m_gps_data->S1800.time_min[9], p1);
}
static double S3600_display_last(void) {
    return m_gps_data->S3600.display_last_run;
}
static double S3600_display_max(void) {
    return m_gps_data->S3600.display_max_speed;
}
static double S3600_display_avg(void) {
    return m_gps_data->S3600.avg_5runs;
}
static size_t S3600_display_max_time(char *p1) {
    return time_to_char_hm(m_gps_data->S3600.time_hour[9], m_gps_data->S3600.time_min[9], p1);
}
static double M250_display_last(void) {
    return m_gps_data->M250.m_max_speed;
}
static double M250_display_max(void) {
    return m_gps_data->M250.display_max_speed;
}

static double M250_display_avg(void) {
    return get_avg(m_gps_data->M250.avg_speed);
}
static size_t M250_display_max_time(char *p1) {
    return time_to_char_hm(m_gps_data->M250.time_hour[9], m_gps_data->M250.time_min[9], p1);
}
static double M500_display_last(void) {
    return m_gps_data->M500.m_max_speed;
}
static double M500_display_max(void) {
    return m_gps_data->M500.display_max_speed;
}
static double M500_display_avg(void) {
    return get_avg(m_gps_data->M500.avg_speed);
}
static size_t M500_display_max_time(char *p1) {
    return time_to_char_hm(m_gps_data->M500.time_hour[9], m_gps_data->M500.time_min[9], p1);
}
static double M1852_display_last(void) {
    return m_gps_data->M1852.m_max_speed;
}
static double M1852_display_max(void) {
    return m_gps_data->M1852.display_max_speed;
}
static double M1852_display_avg(void) {
    return get_avg(m_gps_data->M1852.avg_speed);
}
static size_t M1852_display_max_time(char *p1) {
    return time_to_char_hm(m_gps_data->M1852.time_hour[9], m_gps_data->M1852.time_min[9], p1);
}
static double M100_display_last(void) {
    return m_gps_data->M100.m_max_speed;
}
static double M100_display_max(void) {
    return m_gps_data->M100.display_max_speed;
}
static double M100_display_avg(void) {
    return get_avg(m_gps_data->M100.avg_speed);
}
static size_t M100_display_max_time(char *p1) {
    return time_to_char_hm(m_gps_data->M100.time_hour[9], m_gps_data->M100.time_min[9], p1);
}
static double A500_display_last(void) {
    return m_gps_data->A500.alfa_speed_max;
}
static double A500_display_max(void) {
    return m_gps_data->A500.display_max_speed;
}
static double A500_display_avg(void) {
    return get_avg(m_gps_data->A500.avg_speed);
}
static size_t A500_display_max_time(char *p1) {
    return time_to_char_hm(m_gps_data->A500.time_hour[9], m_gps_data->A500.time_min[9], p1);
}
static double A500_r1_display(void) {
    return m_gps_data->A500.avg_speed[9];
}
static double A500_r2_display(void) {
    return m_gps_data->A500.avg_speed[8];
}
static double A500_r3_display(void) {
    return m_gps_data->A500.avg_speed[7];
}
static double A500_r4_display(void) {
    return m_gps_data->A500.avg_speed[6];
}
static double A500_r5_display(void) {
    return m_gps_data->A500.avg_speed[5];
}
static double distance(void) {
    return m_gps_data->Ublox.total_distance / 1000000;
}

screen_f_t avail_fields[] = {
    {.id = 0, .type = 0, .value.num = S10_display_last, .name = "10sLST", .abbr="10sLL", .name_len = 7},
    {.id = 1, .type = 0, .value.num = S10_display_max, .name = "10sMAX", .abbr="10sM", .name_len = 6},
    {.id = 2, .type = 0, .value.num = S10_display_avg, .name = "10sAVG", .abbr="10sA", .name_len = 6},
    {.id = 3, .type = 1, .value.timestr = S10_display_max_time, .name = "10sMTM", .abbr="10sT", .name_len = 8},
    {.id = 4, .type = 0, .value.num = S2_display_last, .name = "2sLST", .abbr="2sL", .name_len = 6},
    {.id = 5, .type = 0, .value.num = S2_display_max, .name = "2sMAX", .abbr="2sM", .name_len = 5},
    {.id = 6, .type = 0, .value.num = S2_display_avg, .name = "2sAVG", .abbr="2sA", .name_len = 5},
    {.id = 7, .type = 1, .value.timestr = S2_display_max_time, .name = "2sMaxTm", .abbr="2sT", .name_len = 7},
    {.id = 8, .type = 0, .value.num = M500_display_last, .name = "500mLST", .abbr="500mL", .name_len = 8},
    {.id = 9, .type = 0, .value.num = M500_display_max, .name = "500mMAX", .abbr="500mM", .name_len = 7},
    {.id = 10, .type = 0, .value.num = M500_display_avg, .name = "500mAVG", .abbr="500mA", .name_len = 7},
    {.id = 11, .type = 1, .value.timestr = M500_display_max_time, .name = "500mMTM", .abbr="500mT", .name_len = 9},
    {.id = 12, .type = 0, .value.num = M250_display_last, .name = "250mLST", .abbr="250mL", .name_len = 8},
    {.id = 13, .type = 0, .value.num = M250_display_max, .name = "250mMAX", .abbr="250mM", .name_len = 7},
    {.id = 14, .type = 0, .value.num = M250_display_avg, .name = "250mAVG", .abbr="250mA", .name_len = 7},
    {.id = 15, .type = 1, .value.timestr = M250_display_max_time, .name = "250mMTM", .abbr="250mT", .name_len = 9},
    {.id = 16, .type = 0, .value.num = S10_r1_display, .name = "10sR1", .abbr="R1", .name_len = 5},
    {.id = 17, .type = 0, .value.num = S10_r2_display, .name = "10sR2", .abbr="R2", .name_len = 5},
    {.id = 18, .type = 0, .value.num = S10_r3_display, .name = "10sR3", .abbr="R3", .name_len = 5},
    {.id = 19, .type = 0, .value.num = S10_r4_display, .name = "10sR4", .abbr="R4", .name_len = 5},
    {.id = 20, .type = 0, .value.num = S10_r5_display, .name = "10sR5", .abbr="R5", .name_len = 5},
    {.id = 21, .type = 0, .value.num = M1852_display_last, .name = "NmLST", .abbr="NmL", .name_len = 6},
    {.id = 22, .type = 0, .value.num = M1852_display_max, .name = "NmMAX", .abbr="NmM", .name_len = 5},
    {.id = 23, .type = 0, .value.num = M1852_display_avg, .name = "NmAVG", .abbr="NmA", .name_len = 5},
    {.id = 24, .type = 1, .value.timestr = M1852_display_max_time, .name = "NmMTM", .abbr="NmT", .name_len = 7},
    {.id = 25, .type = 0, .value.num = A500_display_last, .name = "A500LST", .abbr="A500L", .name_len = 8},
    {.id = 26, .type = 0, .value.num = A500_display_max, .name = "A500MAX", .abbr="A500M", .name_len = 7},
    {.id = 27, .type = 0, .value.num = A500_display_avg, .name = "A500AVG", .abbr="A500A", .name_len = 7},
    {.id = 28, .type = 1, .value.timestr = A500_display_max_time, .name = "A500MTM", .abbr="A500T", .name_len = 9},
    {.id = 29, .type = 0, .value.num = M100_display_last, .name = "100mLST", .abbr="100mL", .name_len = 8},
    {.id = 30, .type = 0, .value.num = M100_display_max, .name = "100mMAX", .abbr="100mM", .name_len = 7},
    {.id = 31, .type = 0, .value.num = M100_display_avg, .name = "100mAVG", .abbr="100mA", .name_len = 7},
    {.id = 32, .type = 1, .value.timestr = M100_display_max_time, .name = "100mMTM", .abbr="100mT", .name_len = 9},
    {.id = 33, .type = 0, .value.num = S1800_display_last, .name = "1/2hLST", .abbr="1/2hL", .name_len = 8},
    {.id = 34, .type = 0, .value.num = S1800_display_max, .name = "1/2hMAX", .abbr="1/2hM", .name_len = 7},
    {.id = 35, .type = 0, .value.num = S1800_display_avg, .name = "1/2hAVG", .abbr="1/2hA", .name_len = 7},
    {.id = 36, .type = 1, .value.timestr = S1800_display_max_time, .name = "1/2hMTM", .abbr="1/2hT", .name_len = 9},
    {.id = 37, .type = 0, .value.num = S3600_display_last, .name = "1hLST", .abbr="1hL", .name_len = 6},
    {.id = 38, .type = 0, .value.num = S3600_display_max, .name = "1hMAX", .abbr="1hM", .name_len = 5},
    {.id = 39, .type = 0, .value.num = S3600_display_avg, .name = "1hAVG", .abbr="1hA", .name_len = 5},
    {.id = 40, .type = 1, .value.timestr = S3600_display_max_time, .name = "1hMTM", .abbr="1hT", .name_len = 7},
    {.id = 41, .type = 0, .value.num = distance, .name = "Dist", .abbr="Dst", .name_len = 4},
    {.id = 42, .type = 0, .value.num = A500_r1_display, .name = "A500R1", .abbr="R1", .name_len = 6},
    {.id = 43, .type = 0, .value.num = A500_r2_display, .name = "A500R2", .abbr="R2", .name_len = 6},
    {.id = 44, .type = 0, .value.num = A500_r3_display, .name = "A500R3", .abbr="R3", .name_len = 6},
    {.id = 45, .type = 0, .value.num = A500_r4_display, .name = "A500R4", .abbr="R4", .name_len = 6},
    {.id = 46, .type = 0, .value.num = A500_r5_display, .name = "A500R5", .abbr="R5", .name_len = 6},
};

stat_screen_t sc_screens[] = {
    { // 0 10s
        .cols = 12,
        .rows = 2,
        .num_fields = 3,
        .fields[0].field = &avail_fields[0],
        .fields[1].field = &avail_fields[1],
        .fields[2].field = &avail_fields[2],
    },
    { // 1 10sec avg 5runs
        .cols = 2,
        .rows = 3,
        .num_fields = 6,
        .fields[0].field = &avail_fields[2],  // 10s avg
        .fields[1].field = &avail_fields[16], // r1
        .fields[2].field = &avail_fields[17], // r2
        .fields[3].field = &avail_fields[18], // r3
        .fields[4].field = &avail_fields[19], // r4
        .fields[5].field = &avail_fields[20], // r5
    },
    { // 2 stats
        .cols = 2,
        .rows = 3,
        .num_fields = 6,
        .fields[0].field = &avail_fields[29], // 100m max
        .fields[1].field = &avail_fields[13], // 250m max
        .fields[2].field = &avail_fields[22], // Nm max
        .fields[3].field = &avail_fields[5],  // 2s max
        .fields[4].field = &avail_fields[38], // 1h max
        .fields[5].field = &avail_fields[41], // distance
    },
    { // 3 500m
        .cols = 12,
        .rows = 2,
        .num_fields = 3,
        .fields[0].field = &avail_fields[8],
        .fields[1].field = &avail_fields[9],
        .fields[2].field = &avail_fields[10],
    },
    { // 4 250m
        .cols = 12,
        .rows = 2,
        .num_fields = 3,
        .fields[0].field = &avail_fields[12],
        .fields[1].field = &avail_fields[13],
        .fields[2].field = &avail_fields[14],
    },
    { // 5
        .cols = 2,
        .rows = 3,
        .num_fields = 6,
        .fields[0].field = &avail_fields[27], // a500 avg
        .fields[1].field = &avail_fields[42], // r1
        .fields[2].field = &avail_fields[43], // r2
        .fields[3].field = &avail_fields[44], // r3
        .fields[4].field = &avail_fields[45], // r4
        .fields[5].field = &avail_fields[46], // r5
    },
};

uint8_t get_stat_screens_count(void) {
    return sizeof(sc_screens) / sizeof(stat_screen_t);
}
