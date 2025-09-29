#ifndef __CONFIG_HELPER_H
#define __CONFIG_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

typedef enum {
    CFG_SAVE_OK = 0,
    CFG_SAVE_ERR = -1,
} CFG_SAVE_STATUS_;

#define HAL_FLASH_OP_TIMEOUT 1000U /* 1 s */
#define VALID_CONF_MARKER 0xDEADBEEF

void config_get(ETX_CONFIG_ *etx_config);
void config_load_defaults(ETX_CONFIG_ *etx_config);
CFG_SAVE_STATUS_ config_save(ETX_CONFIG_ *etx_config);

#ifdef __cplusplus
}
#endif

#endif /* __CONFIG_HELPER_H */