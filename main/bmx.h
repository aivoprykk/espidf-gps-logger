/** 
 * bmx.h - bmx280 i2c temperature sensor task
*/
#ifndef CE55CEAF_756D_4FCB_A3D6_C8246CDDEC4F
#define CE55CEAF_756D_4FCB_A3D6_C8246CDDEC4F

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CONFIG_BMX_ENABLE)

typedef struct {
    int32_t temp;
    uint32_t press, humid;
    uint32_t millis;
    uint32_t elapsed;
    int8_t status;
} bmx_t;
int init_bmx();
void deinit_bmx();
bmx_t * bmx_readings();
void bmx_readings_i(int32_t * temp, uint32_t * press, uint32_t * humid);
void bmx_readings_f(float * temp, float * press, float * humid);

#endif

#ifdef __cplusplus
}
#endif

#endif /* CE55CEAF_756D_4FCB_A3D6_C8246CDDEC4F */
