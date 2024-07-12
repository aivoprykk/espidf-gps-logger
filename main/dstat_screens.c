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
static double S2_r1_display(void) {
    return m_gps_data->S2.display_speed[9];
}
static double S2_r2_display(void) {
    return m_gps_data->S2.display_speed[8];
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
static double S1800_r1_display(void) {
    return m_gps_data->S1800.display_speed[9];
}
static double S1800_r2_display(void) {
    return m_gps_data->S1800.display_speed[8];
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
static double M250_r1_display(void) {
    return m_gps_data->M250.display_speed[9];
}
static double M250_r2_display(void) {
    return m_gps_data->M250.display_speed[8];
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
static double M500_r1_display(void) {
    return m_gps_data->M500.display_speed[9];
}
static double M500_r2_display(void) {
    return m_gps_data->M500.display_speed[8];
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
static double M1852_r1_display(void) {
    return m_gps_data->M1852.display_speed[9];
}
static double M1852_r2_display(void) {
    return m_gps_data->M1852.display_speed[8];
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

const char s10[] = "10s";
const char s2[] = "2s";
const char m500[] = "500m";
const char m250[] = "250m";
const char m1852[] = "Nm";
const char a500[] = "A500";
const char m100[] = "100m";
const char s1800[] = ".5h";
const char s3600[] = "1h";

const screen_f_t avail_fields[] = {
    {0, 0, .value.num = S10_display_last, "10sLST", "L", s10},
    {1, 0, .value.num = S10_display_max, "10sMAX", "M", s10},
    {2, 0, .value.num = S10_display_avg, "10sAVG", "A", s10},
    {3, 1, .value.timestr = S10_display_max_time, "10sMTM", "T", s10},

    {4, 0, .value.num = S2_display_last, "2sLST", "L", s2},
    {5, 0, .value.num = S2_display_max, "2sMAX", "M", s2},
    {6, 0, .value.num = S2_display_avg, "2sAVG", "A", s2},
    {7, 1, .value.timestr = S2_display_max_time, "2sMTm", "T", s2},

    {8, 0, .value.num = M500_display_last, "500mLST", "L", m500},
    {9, 0, .value.num = M500_display_max, "500mMAX", "M", m500},
    {10, 0, .value.num = M500_display_avg, "500mAVG", "A", m500},
    {11, 1, .value.timestr = M500_display_max_time, "500mMTM", "T", m500},

    {12, 0, .value.num = M250_display_last, "250mLST", "L", m250},
    {13, 0, .value.num = M250_display_max, "250mMAX", "M", m250},
    {14, 0, .value.num = M250_display_avg, "250mAVG", "A", m250},
    {15, 1, .value.timestr = M250_display_max_time, "250mMTM", "T", m250},

    {16, 0, .value.num = S10_r1_display, "10sR1", "R1", s10},
    {17, 0, .value.num = S10_r2_display, "10sR2", "R2", s10},
    {18, 0, .value.num = S10_r3_display, "10sR3", "R3", s10},
    {19, 0, .value.num = S10_r4_display, "10sR4", "R4", s10},
    {20, 0, .value.num = S10_r5_display, "10sR5", "R5", s10},

    {21, 0, .value.num = M1852_display_last, "NmLST", "L", m1852},
    {22, 0, .value.num = M1852_display_max, "NmMAX", "M", m1852},
    {23, 0, .value.num = M1852_display_avg, "NmAVG", "A", m1852},
    {24, 1, .value.timestr = M1852_display_max_time, "NmMTM", "T", m1852},

    {25, 0, .value.num = A500_display_last, "A500LST", "L", a500},
    {26, 0, .value.num = A500_display_max, "A500MAX", "M", a500},
    {27, 0, .value.num = A500_display_avg, "A500AVG", "A", a500},
    {28, 1, .value.timestr = A500_display_max_time, "A500MTM", "T", a500},

    {29, 0, .value.num = M100_display_last, "100mLST", "L", m100},
    {30, 0, .value.num = M100_display_max, "100mMAX", "M", m100},
    {31, 0, .value.num = M100_display_avg, "100mAVG", "A", m100},
    {32, 1, .value.timestr = M100_display_max_time, "100mMTM", "T", m100},

    {33, 0, .value.num = S1800_display_last, ".5hLST", "L", s1800},
    {34, 0, .value.num = S1800_display_max, ".5hMAX", "M", s1800},
    {35, 0, .value.num = S1800_display_avg, ".5hAVG", "A", s1800},
    {36, 1, .value.timestr = S1800_display_max_time, ".5hMTM", "T", s1800},

    {37, 0, .value.num = S3600_display_last, "1hLST", "L", s3600},
    {38, 0, .value.num = S3600_display_max, "1hMAX", "M", s3600},
    {39, 0, .value.num = S3600_display_avg, "1hAVG", "A", s3600},
    {40, 1, .value.timestr = S3600_display_max_time, "1hMTM", "T", s3600},

    {41, 0, .value.num = distance, "Dist", "Dst", "Dst"},

    {42, 0, .value.num = A500_r1_display, "A500R1", "R1", a500},
    {43, 0, .value.num = A500_r2_display, "A500R2", "R2", a500},
    {44, 0, .value.num = A500_r3_display, "A500R3", "R3", a500},
    {45, 0, .value.num = A500_r4_display, "A500R4", "R4", a500},
    {46, 0, .value.num = A500_r5_display, "A500R5", "R5", a500},

    {47, 0, .value.num = S2_r1_display, "2sR1", "R1", s2},
    {48, 0, .value.num = S2_r2_display, "2sR2", "R2", s2},

    {49, 0, .value.num = S1800_r1_display, ".5hR1", "R1", s1800},
    {50, 0, .value.num = S1800_r2_display, ".5hR2", "R2", s1800},

    {51, 0, .value.num = M250_r1_display, "250mR1", "R1", m250},
    {52, 0, .value.num = M250_r2_display, "250mR2", "R2", m250},

    {53, 0, .value.num = M500_r1_display, "500mR1", "R1", m250},
    {54, 0, .value.num = M500_r2_display, "500mR2", "R2", m250},

    {55, 0, .value.num = M1852_r1_display, "NmmR1", "R1", m1852},
    {56, 0, .value.num = M1852_r2_display, "NmmR2", "R2", m1852},
};

const stat_screen_t sc_screens[] = {
    { // 0 10s
        .cols = 12,
        .rows = 2,
        .num_fields = 3,
        .fields[0].field = &avail_fields[0],
        .fields[1].field = &avail_fields[1],
        .fields[2].field = &avail_fields[2],
        .use_abbr = false,
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
        .use_abbr = true,
    },
    { // 2 stats
        .cols = 2,
        .rows = 3,
        .num_fields = 6,
        .fields[0].field = &avail_fields[30], // 100m max
        .fields[1].field = &avail_fields[13], // 250m max
        .fields[2].field = &avail_fields[22], // Nm max
        .fields[3].field = &avail_fields[5],  // 2s max
        .fields[4].field = &avail_fields[38], // 1h max
        .fields[5].field = &avail_fields[41], // distance
        .use_abbr = false,
    },
    { // 3 500m
        .cols = 12,
        .rows = 2,
        .num_fields = 3,
        .fields[0].field = &avail_fields[8],
        .fields[1].field = &avail_fields[9],
        .fields[2].field = &avail_fields[10],
        .use_abbr = false,
    },
    { // 4 250m
        .cols = 12,
        .rows = 2,
        .num_fields = 3,
        .fields[0].field = &avail_fields[12],
        .fields[1].field = &avail_fields[13],
        .fields[2].field = &avail_fields[14],
        .use_abbr = false,
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
        .use_abbr = true,
    },
};

uint8_t get_stat_screens_count(void) {
    return sizeof(sc_screens) / sizeof(stat_screen_t);
}
