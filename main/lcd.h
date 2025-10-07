#ifndef AE577623_35A6_43A5_9250_F68AA337A0DF
#define AE577623_35A6_43A5_9250_F68AA337A0DF

#ifdef __cplusplus
extern "C" {
#endif

#include "logger_common.h"

#include <stdint.h>
#include "sdkconfig.h"

struct display_s *lcd_init();
void lcd_deinit();

#ifdef __cplusplus
}
#endif

#endif /* AE577623_35A6_43A5_9250_F68AA337A0DF */
