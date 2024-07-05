#ifndef SD_CARD_H_
#define SD_CARD_H_

/*********************
 *      INCLUDES
 *********************/

#include <stdint.h>

#include "driver/gpio.h"
#include "driver/sdspi_host.h"
#include "driver/sdmmc_types.h"

#include "sdmmc_cmd.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*********************
 *      DEFINES
 *********************/

#define SD_CARD_TAG "[sd_card]"

/**********************
 *   PUBLIC DATA
 **********************/

extern const char *MOUNT_POINT;

/**********************
 *   PUBLIC FUNCTIONS
 **********************/

void configSDcard();

esp_err_t mountSDCard(char *mount_point, sdmmc_card_t *card);
esp_err_t sd_card_read_sectors(sdmmc_card_t* card, void* dst,
        size_t start_block, size_t block_count);
esp_err_t sd_card_write_sectors(sdmmc_card_t* card, const void* src,
        size_t start_block, size_t block_count);

#ifdef __cplusplus
}
#endif

#endif /* SD_CARD_H_ */