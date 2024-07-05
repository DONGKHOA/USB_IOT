/*********************
 *      INCLUDES
 *********************/

#include "usb_msc_storage.h"
#include "wear_levelling.h"
#include "sd_card.h"

#include "esp_check.h"
#include "esp_vfs_fat.h"

#include "diskio_impl.h"
#include "diskio_wl.h"

#include "vfs_fat_internal.h"
#include "tinyusb.h"
#include "class/msc/msc_device.h"

/**********************
 *      TYPEDEFS
 **********************/

typedef struct 
{
    bool is_fat_mounted;
    const char *base_path;
    union 
    {
        wl_handle_t wl_handle;
        sdmmc_card_t *card;
    };
    esp_err_t (*mount)(BYTE pdrv);
    esp_err_t (*unmount)(void);
    uint32_t (*sector_count)(void);
    uint32_t (*sector_size)(void);
    esp_err_t (*read)(size_t sector_size, uint32_t lba, uint32_t offset, size_t size, void *dest);
    esp_err_t (*write)(size_t sector_size, size_t addr, uint32_t lba, uint32_t offset, size_t size, const void *src);
    // tusb_msc_callback_t callback_mount_changed;
    // tusb_msc_callback_t callback_premount_changed;
    int max_files;
} usb_msc_storage_handle_s; /*!< MSC object */

/* handle of tinyusb driver connected to application */
static usb_msc_storage_handle_s *s_storage_handle;

/***********************************
 *   PRIVATE FUNCTIONS PROTOTYPE
 **********************************/

static esp_err_t _mount_spi_sd(BYTE pdrv);
static esp_err_t _unmount_spi_sd(void);
static uint32_t _get_sector_count_spi_sd(void);
static uint32_t _get_sector_size_spi_sd(void);
static esp_err_t _read_sector_spi_sd(size_t sector_size,
                                    uint32_t lba,
                                    uint32_t offset,
                                    size_t size,
                                    void *dest);
static esp_err_t _read_sector_spi_sd(size_t sector_size,
                                    uint32_t lba,
                                    uint32_t offset,
                                    size_t size,
                                    void *dest);
static esp_err_t _write_sector_spi_sd(size_t sector_size,
                                    size_t addr,
                                    uint32_t lba,
                                    uint32_t offset,
                                    size_t size,
                                    const void *src);

/**********************
 *   PUBLIC FUNCTIONS
 **********************/

esp_err_t usb_msc_storage_init_spi_sd_card(const usb_msc_spi_config_t *config)
{
    assert(!s_storage_handle);
    s_storage_handle = (usb_msc_storage_handle_s *)malloc(sizeof(usb_msc_storage_handle_s));
    ESP_RETURN_ON_FALSE(s_storage_handle, ESP_ERR_NO_MEM, USB_DEVICE_MSC_TAG, "could not allocate new handle for storage");
    s_storage_handle->mount = &_mount_spi_sd;
    s_storage_handle->unmount = &_unmount_spi_sd;
    s_storage_handle->sector_count = &_get_sector_count_spi_sd;
    s_storage_handle->sector_size = &_get_sector_size_spi_sd;
    s_storage_handle->read = &_read_sector_spi_sd;
    s_storage_handle->write = &_write_sector_spi_sd;
    s_storage_handle->is_fat_mounted = false;
    s_storage_handle->base_path = NULL;
    s_storage_handle->card = config->card;
    // In case the user does not set mount_config.max_files
    // and for backward compatibility with versions <1.4.2
    // max_files is set to 2
    const int max_files = config->mount_config.max_files;
    s_storage_handle->max_files = max_files > 0 ? max_files : 2;

    // /* Callbacks setting up*/
    // if (config->callback_mount_changed) {
    //     usb_msc_register_callback(USB_MSC_EVENT_MOUNT_CHANGED, config->callback_mount_changed);
    // } else {
    //     usb_msc_unregister_callback(USB_MSC_EVENT_MOUNT_CHANGED);
    // }
    // if (config->callback_premount_changed) {
    //     usb_msc_register_callback(USB_MSC_EVENT_PREMOUNT_CHANGED, config->callback_premount_changed);
    // } else {
    //     usb_msc_unregister_callback(USB_MSC_EVENT_PREMOUNT_CHANGED);
    // }

    return ESP_OK;
}

/***********************************
 *   PRIVATE FUNCTIONS
 **********************************/

static esp_err_t _mount_spi_sd(BYTE pdrv)
{
    ff_diskio_register_sdmmc(pdrv, s_storage_handle->card);
    ff_sdmmc_set_disk_status_check(pdrv, false);
    return ESP_OK;
}

static esp_err_t _unmount_spi_sd(void)
{
    BYTE pdrv;
    pdrv = ff_diskio_get_pdrv_card(s_storage_handle->card);
    if (pdrv == 0xff) {
        ESP_LOGE(TAG, "Invalid state");
        return ESP_ERR_INVALID_STATE;
    }

    char drv[3] = {(char)('0' + pdrv), ':', 0};
    f_mount(0, drv, 0);
    ff_diskio_unregister(pdrv);

    return ESP_OK;
}

static uint32_t _get_sector_count_spi_sd(void)
{
    assert(s_storage_handle->card);
    return (uint32_t)s_storage_handle->card->csd.capacity;
}

static uint32_t _get_sector_size_spi_sd(void)
{
    assert(s_storage_handle->card);
    return (uint32_t)s_storage_handle->card->csd.sector_size;
}

static esp_err_t _read_sector_spi_sd(size_t sector_size,
                                    uint32_t lba,
                                    uint32_t offset,
                                    size_t size,
                                    void *dest)
{
    return sd_card_read_sectors(s_storage_handle->card, dest, lba, size / sector_size);
}

static esp_err_t _write_sector_spi_sd(size_t sector_size,
                                     size_t addr,
                                     uint32_t lba,
                                     uint32_t offset,
                                     size_t size,
                                     const void *src)
{
    return sd_card_write_sectors(s_storage_handle->card, src, lba, size / sector_size);
}