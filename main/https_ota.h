#ifndef OTA_H
#define OTA_H

#include <stdint.h>
#include "strbf.h"

#ifdef __cplusplus
extern "C" {
#endif


#define OTA_HOST_SIZE 24
#define OTA_PATH_SIZE 64
#define OTA_REQ_SIZE 256

struct m_ota_ctx {
    char host[OTA_HOST_SIZE];
    uint16_t port;
    uint32_t last_ota_check;
    uint32_t delay_in_sec;
    char path[OTA_PATH_SIZE];
    char req[OTA_REQ_SIZE];
    struct strbf_s request;
    unsigned char ok;
};

void ota_get_task(void *pvParameters);
void ota_setup();

#ifdef __cplusplus
}
#endif
#endif