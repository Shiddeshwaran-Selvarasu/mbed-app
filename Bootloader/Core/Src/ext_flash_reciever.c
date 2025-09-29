#include "ext_flash_reciever.h"
#include "conf_helper.h"
#include "logger.h"

ETX_DL_EX_ etx_app_download_and_flash(ETX_CONFIG_ *config) {
    if (config != NULL) {
        config->reboot_reason = ETX_NORMAL_BOOT;
        config->is_app_bootable = true;
        config->is_app_flashed = true;
        if (config_save(config) != CFG_SAVE_OK) {
            LOG_ERROR("Failed to save updated configuration\r\n");
        }
        return ETX_DL_EX_OK;
    }

    // TODO: implement OTA logic here...
    return ETX_DL_EX_ERR;
}