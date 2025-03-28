#ifndef AB1949FE_3AC0_440D_8410_A7A9667AFC32
#define AB1949FE_3AC0_440D_8410_A7A9667AFC32

#ifdef __cplusplus
extern "C" {
#endif

#include "sdkconfig.h"

typedef struct pin_configuration_s {
    const char** names;
    const int* pins;
    const int* modes;
#if defined(CONFIG_ENABLE_ADC_FEATURE)
    const int *adc_channels;
    const int *adc_units;
#endif
} pin_configuration_t;

void check_pins(struct pin_configuration_s *config, const int pin_count);

#ifdef __cplusplus
}
#endif

#endif /* AB1949FE_3AC0_440D_8410_A7A9667AFC32 */
