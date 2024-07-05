/*********************
 *      INCLUDES
 *********************/

#include "sd_card.h"

#include "esp_vfs_fat.h"
#include "esp_log.h"

/*********************
 *      DEFINES
 *********************/

const char *MOUNT_POINT = "/data";

esp_err_t mountSDCARD(char *mount_point, sdmmc_card_t *card)
{
	ESP_LOGI(SD_CARD_TAG, "Initializing SDCARD file system");
	esp_err_t ret;
	// Options for mounting the filesystem.
	// If format_if_mount_failed is set to true, SD card will be partitioned and
	// formatted in case when mounting fails.
	esp_vfs_fat_sdmmc_mount_config_t mount_config = {
		.format_if_mount_failed = true,
		.max_files = 4, // maximum number of files which can be open at the same time
		.allocation_unit_size = 4096};

	// Use settings defined above to initialize SD card and mount FAT filesystem.
	// Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
	// Please check its source code and implement error recovery when developing
	// production applications.
	ESP_LOGI(SD_CARD_TAG, "Using SPI peripheral");
	ESP_LOGI(SD_CARD_TAG, "SDSPI_MOSI=%d", GPIO_NUM_35);
	ESP_LOGI(SD_CARD_TAG, "SDSPI_MISO=%d", GPIO_NUM_37);
	ESP_LOGI(SD_CARD_TAG, "SDSPI_CLK=%d", GPIO_NUM_36);
	ESP_LOGI(SD_CARD_TAG, "SDSPI_CS=%d", GPIO_NUM_14);

	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	spi_bus_config_t bus_cfg = {
		.mosi_io_num = GPIO_NUM_35,
		.miso_io_num = GPIO_NUM_37,
		.sclk_io_num = GPIO_NUM_36,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 4000,
	};
	ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
	if (ret != ESP_OK)
	{
		ESP_LOGE(SD_CARD_TAG, "Failed to initialize bus.");
		return ret;
	}
	// This initializes the slot without card detect (CD) and write protect (WP) signals.
	// Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
	sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
	slot_config.gpio_cs = GPIO_NUM_14;
	slot_config.host_id = host.slot;

	ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
	ESP_LOGI(SD_CARD_TAG, "esp_vfs_fat_sdspi_mount=%d", ret);

	if (ret != ESP_OK)
	{
		if (ret == ESP_FAIL)
		{
			ESP_LOGE(SD_CARD_TAG, "Failed to mount filesystem. "
						  "If you want the card to be formatted, set format_if_mount_failed = true.");
		}
		else
		{
			ESP_LOGE(SD_CARD_TAG, "Failed to initialize the card (%s). "
						  "Make sure SD card lines have pull-up resistors in place.",
					 esp_err_to_name(ret));
		}

		return ret;
	}

	// Card has been initialized, print its properties
	sdmmc_card_print_info(stdout, card);
	ESP_LOGI(SD_CARD_TAG, "Mounte SD card on %s", mount_point);
	return ret;
}