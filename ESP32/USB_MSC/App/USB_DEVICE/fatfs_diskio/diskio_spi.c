/*********************
 *      INCLUDES
 *********************/

#include "diskio_spi.h"

#include "ffconf.h"
#include "ff.h"
#include "sdmmc_cmd.h"
#include "esp_log.h"

/**********************
 *   PRIVATE DATA
 **********************/

static sdmmc_card_t* s_cards[FF_VOLUMES] = { NULL };
static bool s_disk_status_check_en[FF_VOLUMES] = { };

/***********************************
 *   PRIVATE FUNCTIONS PROTOTYPE
 **********************************/



/**********************
 *   PUBLIC FUNCTIONS
 **********************/

void ff_spi_set_disk_status_check(BYTE pdrv, bool enable)
{

}

void ff_diskio_register_spi(unsigned char pdrv, sdmmc_card_t* card)
{
    static const ff_diskio_impl_t sdmmc_impl = 
    {
        .init = &ff_sdmmc_initialize,
        .status = &ff_sdmmc_status,
        .read = &ff_sdmmc_read,
        .write = &ff_sdmmc_write,
        .ioctl = &ff_sdmmc_ioctl
    };
    s_cards[pdrv] = card;
    s_disk_status_check_en[pdrv] = false;
    ff_diskio_register(pdrv, &sdmmc_impl);
}

BYTE ff_diskio_get_pdrv_card(const sdmmc_card_t* card)
{

}

/***********************************
 *   PRIVATE FUNCTIONS
 **********************************/

