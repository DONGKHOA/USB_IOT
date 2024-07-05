#ifndef USB_MSC_STORAGE_H_
#define USB_MSC_STORAGE_H_

/*********************
 *      INCLUDES
 *********************/

#include <stdint.h>

#include "esp_err.h"
#include "esp_vfs_fat.h"

#include "driver/sdmmc_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*********************
 *      DEFINES
 *********************/

#define USB_DEVICE_MSC_TAG "[usb_device_msc]"

/**********************
 *      TYPEDEFS
 **********************/

typedef struct 
{
    bool is_mounted;                        /*!< Flag if storage is mounted or not */
} usb_msc_event_mount_changed_data_t;

/**
 * @brief Types of MSC events
 */
typedef enum 
{
    USB_MSC_EVENT_MOUNT_CHANGED,        /*!< Event type AFTER mount/unmount operation is successfully finished */
    USB_MSC_EVENT_PREMOUNT_CHANGED      /*!< Event type BEFORE mount/unmount operation is started */
} usb_msc_event_type_t;

/**
 * @brief Describes an event passing to the input of a callbacks
 */
typedef struct 
{
    usb_msc_event_type_t type; /*!< Event type */
    union 
    {
        usb_msc_event_mount_changed_data_t mount_changed_data; /*!< Data input of the callback */
    };
} usb_msc_event_t;

/**
 * @brief MSC callback that is delivered whenever a specific event occurs.
 */
typedef void(*usb_msc_callback_t)(usb_msc_event_t *event);

/**
 * @brief Configuration structure for spi sd card initialization
 *
 * User configurable parameters that are used while
 * initializing the spi sd card media.
 */

typedef struct 
{
    sdmmc_card_t *card;                            /*!< Pointer to sdmmc card configuration structure */
    usb_msc_callback_t callback_mount_changed;     /*!< Pointer to the function callback that will be delivered AFTER mount/unmount operation is successfully finished */
    usb_msc_callback_t callback_premount_changed;  /*!< Pointer to the function callback that will be delivered BEFORE mount/unmount operation is started */
    const esp_vfs_fat_mount_config_t mount_config; /*!< FATFS mount config */ 
} usb_msc_spi_config_t;


/**********************
 *   PUBLIC FUNCTIONS
 **********************/

esp_err_t usb_msc_storage_init_spi_sd_card(const usb_msc_spi_config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* USB_MSC_STORAGE_H_ */