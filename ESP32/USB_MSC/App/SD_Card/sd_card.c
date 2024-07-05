/*********************
 *      INCLUDES
 *********************/

#include "sd_card.h"

#include "esp_vfs_fat.h"
#include "esp_log.h"
#include "esp_dma_utils.h"
#include "esp_timer.h"

#include "spi.h"
#include <string.h>
#include <inttypes.h>

#include "driver/sdmmc_defs.h"
#include "../sdmmc_common.h"

/**********************
 *   	 DATA
 **********************/

const char *MOUNT_POINT = "/data";

/***********************************
 *   PRIVATE DATA
 ***********************************/

static spi_config_t spi_sd_card = 
{
	.frequency = 4000,
	.pin_mosi = GPIO_NUM_35,
	.pin_miso = GPIO_NUM_37,
	.pin_clk = GPIO_NUM_36,
	.pin_cs = GPIO_NUM_14,
};

static esp_err_t ret;

/***********************************
 *   PRIVATE FUNCTIONS PROTOTYPE
 **********************************/

static esp_err_t sd_card_read_sectors_dma(sdmmc_card_t* card, void* dst,
        size_t start_block, size_t block_count, size_t buffer_len);
static esp_err_t sd_card_write_sectors_dma(sdmmc_card_t* card, const void* src,
        size_t start_block, size_t block_count, size_t buffer_len);
static esp_err_t sd_card_send_cmd_send_status(sdmmc_card_t* card, 
		uint32_t* out_status);
static esp_err_t sd_card_send_cmd(sdmmc_card_t* card, sdmmc_command_t* cmd);

/**********************
 *   PUBLIC FUNCTIONS
 **********************/

esp_err_t mountSDCard(char *mount_point, sdmmc_card_t *card)
{
	ESP_LOGI(SD_CARD_TAG, "Initializing SDCARD file system");
	esp_err_t ret;
	// Options for mounting the filesystem.
	// If format_if_mount_failed is set to true, SD card will be partitioned and
	// formatted in case when mounting fails.
	esp_vfs_fat_sdmmc_mount_config_t mount_config = 
	{
		.format_if_mount_failed = true,
		.max_files = 4, // maximum number of files which can be open at the same time
		.allocation_unit_size = 4096
	};

	// Use settings defined above to initialize SD card and mount FAT filesystem.
	// Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
	// Please check its source code and implement error recovery when developing
	// production applications.

	spi_bus_config_t bus_cfg;
	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	SPI_Master_DMA_Init(&spi_sd_card, &bus_cfg, host.slot);

	// This initializes the slot without card detect (CD) and write protect (WP) signals.
	// Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
	sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
	slot_config.gpio_cs = spi_sd_card.pin_cs;
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

esp_err_t sd_card_read_sectors(sdmmc_card_t* card, void* dst,
        size_t start_block, size_t block_count)
{
    if (block_count == 0) {
        return ESP_OK;
    }

    ret = ESP_OK;
    size_t block_size = card->csd.sector_size;
    if (esp_dma_is_buffer_aligned(dst, block_size * block_count, 
			ESP_DMA_BUF_LOCATION_INTERNAL)) 
	{
        ret = sd_card_read_sectors_dma(card, dst, start_block, 
					block_count, block_size * block_count);
    } 
	else 
	{
        // SPI peripheral needs DMA-capable buffers. Split the read into
        // separate single block reads, if needed, and allocate a temporary
        // DMA-capable buffer.
        void *tmp_buf = NULL;
        size_t actual_size = 0;
        ret = esp_dma_malloc(block_size, 0, &tmp_buf, &actual_size);
        if (ret != ESP_OK) 
		{
            return ret;
        }
        uint8_t* cur_dst = (uint8_t*) dst;
        for (size_t i = 0; i < block_count; ++i) 
		{
            ret = sd_card_read_sectors_dma(card, tmp_buf, start_block + i, 
						1, actual_size);
            if (ret != ESP_OK) 
			{
                ESP_LOGD(SD_CARD_TAG, "%s: error 0x%x writing block %d+%d",
                        __func__, ret, start_block, i);
                break;
            }
            memcpy(cur_dst, tmp_buf, block_size);
            cur_dst += block_size;
        }
        free(tmp_buf);
    }
	return ret;
}

esp_err_t sd_card_write_sectors(sdmmc_card_t* card, const void* src,
        size_t start_block, size_t block_count)
{
    if (block_count == 0) 
	{
        return ESP_OK;
    }

    ret = ESP_OK;
    size_t block_size = card->csd.sector_size;

    if (esp_dma_is_buffer_aligned(src, block_size * block_count, 
				ESP_DMA_BUF_LOCATION_INTERNAL)) 
	{
        ret = sd_card_write_sectors_dma(card, src, start_block, 
					block_count, block_size * block_count);
    } 
	else 
	{
        // SPI peripheral needs DMA-capable buffers. Split the write into
        // separate single block writes, if needed, and allocate a temporary
        // DMA-capable buffer.
        void *tmp_buf = NULL;
        size_t actual_size = 0;
        ret = esp_dma_malloc(block_size, 0, &tmp_buf, &actual_size);

        if (ret != ESP_OK) 
		{
            return ret;
        }

        const uint8_t* cur_src = (const uint8_t*) src;
        for (size_t i = 0; i < block_count; ++i) 
		{
            memcpy(tmp_buf, cur_src, block_size);
            cur_src += block_size;
            ret = sd_card_write_sectors_dma(card, tmp_buf, start_block + i, 1, 
						actual_size);
            
			if (ret != ESP_OK) 
			{
                ESP_LOGD(SD_CARD_TAG, "%s: error 0x%x writing block %d+%d",
                        __func__, ret, start_block, i);
                break;
            }
        }
        free(tmp_buf);
    }
    return ret;
}


/***********************************
 *   PRIVATE FUNCTIONS
 **********************************/

static esp_err_t sd_card_send_cmd(sdmmc_card_t* card, sdmmc_command_t* cmd)
{
    if (card->host.command_timeout_ms != 0) 
	{
        cmd->timeout_ms = card->host.command_timeout_ms;
    } 
	else if (cmd->timeout_ms == 0) 
	{
        cmd->timeout_ms = SDMMC_DEFAULT_CMD_TIMEOUT_MS;
    }

    int slot = card->host.slot;

    ESP_LOGV(SD_CARD_TAG, "sending cmd slot=%d op=%" PRIu32 " arg=%" PRIx32 " flags=%x data=%p blklen=%" PRIu32 " datalen=%" PRIu32 " timeout=%" PRIu32,
            slot, cmd->opcode, cmd->arg, cmd->flags, cmd->data, (uint32_t) cmd->blklen, (uint32_t) cmd->datalen, cmd->timeout_ms);
    
	esp_err_t err = (*card->host.do_transaction)(slot, cmd);

    if (err != 0) 
	{
        ESP_LOGD(SD_CARD_TAG, "cmd=%" PRIu32 ", sdmmc_req_run returned 0x%x", cmd->opcode, err);
        return err;
    }

    int state = MMC_R1_CURRENT_STATE(cmd->response);

    ESP_LOGV(SD_CARD_TAG, "cmd response %08" PRIx32 " %08" PRIx32 " %08" PRIx32 " %08" PRIx32 " err=0x%x state=%d",
               cmd->response[0],
               cmd->response[1],
               cmd->response[2],
               cmd->response[3],
               cmd->error,
               state);
    return cmd->error;
}

static esp_err_t sd_card_send_cmd_send_status(sdmmc_card_t* card, 
				uint32_t* out_status)
{
    sdmmc_command_t cmd = 
	{
		.opcode = MMC_SEND_STATUS,
		.arg = MMC_ARG_RCA(card->rca),
		.flags = SCF_CMD_AC | SCF_RSP_R1
    };

    esp_err_t err = sd_card_send_cmd(card, &cmd);

    if (err != ESP_OK) 
	{
        return err;
    }

    if (out_status) 
	{
        if (host_is_spi(card)) 
		{
            *out_status = SD_SPI_R2(cmd.response);
        } 
		else 
		{
            *out_status = MMC_R1(cmd.response);
        }
    }

    return ESP_OK;
}

static esp_err_t sd_card_read_sectors_dma(sdmmc_card_t* card, void* dst,
        size_t start_block, size_t block_count, size_t buffer_len)
{
    if (start_block + block_count > card->csd.capacity) 
	{
        return ESP_ERR_INVALID_SIZE;
    }

    size_t block_size = card->csd.sector_size;

    sdmmc_command_t cmd = 
	{
            .flags = SCF_CMD_ADTC | SCF_CMD_READ | SCF_RSP_R1,
            .blklen = block_size,
            .data = (void*) dst,
            .datalen = block_count * block_size,
            .buflen = buffer_len,
    };
    if (block_count == 1) 
	{
        cmd.opcode = MMC_READ_BLOCK_SINGLE;
    } 
	else 
	{
        cmd.opcode = MMC_READ_BLOCK_MULTIPLE;
    }

    if (card->ocr & SD_OCR_SDHC_CAP) 
	{
        cmd.arg = start_block;
    } 
	else 
	{
        cmd.arg = start_block * block_size;
    }

    esp_err_t err = sd_card_send_cmd(card, &cmd);

    if (err != ESP_OK) 
	{
        ESP_LOGE(SD_CARD_TAG, "%s: sd_card_send_cmd returned 0x%x", __func__, err);
        return err;
    }

    uint32_t status = 0;
    size_t count = 0;
    int64_t yield_delay_us = 100 * 1000; // initially 100ms
    int64_t t0 = esp_timer_get_time();
    int64_t t1 = 0;

    /* SD mode: wait for the card to become idle based on R1 status */
    while (!host_is_spi(card) && !(status & MMC_R1_READY_FOR_DATA)) 
	{
        t1 = esp_timer_get_time();
        if (t1 - t0 > SDMMC_READY_FOR_DATA_TIMEOUT_US) 
		{
            ESP_LOGE(SD_CARD_TAG, "read sectors dma - timeout");
            return ESP_ERR_TIMEOUT;
        }

        if (t1 - t0 > yield_delay_us) 
		{
            yield_delay_us *= 2;
            vTaskDelay(1);
        }

        err = sd_card_send_cmd_send_status(card, &status);

        if (err != ESP_OK) 
		{
            ESP_LOGE(SD_CARD_TAG, "%s: sd_card_send_cmd_send_status returned 0x%x", __func__, err);
            return err;
        }

        if (++count % 16 == 0) 
		{
            ESP_LOGV(SD_CARD_TAG, "waiting for card to become ready (%d)", count);
        }
    }
    return ESP_OK;
}

static esp_err_t sd_card_write_sectors_dma(sdmmc_card_t* card, const void* src,
        size_t start_block, size_t block_count, size_t buffer_len)
{
    if (start_block + block_count > card->csd.capacity) 
	{
        return ESP_ERR_INVALID_SIZE;
    }

    size_t block_size = card->csd.sector_size;

    sdmmc_command_t cmd = 
	{
		.flags = SCF_CMD_ADTC | SCF_RSP_R1,
		.blklen = block_size,
		.data = (void*) src,
		.datalen = block_count * block_size,
		.buflen = buffer_len,
		.timeout_ms = SDMMC_WRITE_CMD_TIMEOUT_MS
    };

    if (block_count == 1) 
	{
        cmd.opcode = MMC_WRITE_BLOCK_SINGLE;
    } 
	else
	{
        cmd.opcode = MMC_WRITE_BLOCK_MULTIPLE;
    }

    if (card->ocr & SD_OCR_SDHC_CAP) 
	{
        cmd.arg = start_block;
    } 
	else 
	{
        cmd.arg = start_block * block_size;
    }
	
    esp_err_t err = sd_card_send_cmd(card, &cmd);
    
	if (err != ESP_OK) 
	{
        ESP_LOGE(SD_CARD_TAG, "%s: sd_card_send_cmd returned 0x%x", __func__, err);
        return err;
    }

    uint32_t status = 0;
    size_t count = 0;
    int64_t yield_delay_us = 100 * 1000; // initially 100ms
    int64_t t0 = esp_timer_get_time();
    int64_t t1 = 0;

    /* SD mode: wait for the card to become idle based on R1 status */
    while (!host_is_spi(card) && !(status & MMC_R1_READY_FOR_DATA)) 
	{
        t1 = esp_timer_get_time();
        if (t1 - t0 > SDMMC_READY_FOR_DATA_TIMEOUT_US) 
		{
            ESP_LOGE(SD_CARD_TAG, "write sectors dma - timeout");
            return ESP_ERR_TIMEOUT;
        }

        if (t1 - t0 > yield_delay_us) 
		{
            yield_delay_us *= 2;
            vTaskDelay(1);
        }

        err = sd_card_send_cmd_send_status(card, &status);

        if (err != ESP_OK) 
		{
            ESP_LOGE(SD_CARD_TAG, "%s: sd_card_send_cmd_send_status returned 0x%x", __func__, err);
            return err;
        }

        if (++count % 16 == 0) 
		{
            ESP_LOGV(SD_CARD_TAG, "waiting for card to become ready (%" PRIu32 ")", (uint32_t) count);
        }
    }
    /* SPI mode: although card busy indication is based on the busy token,
     * SD spec recommends that the host checks the results of programming by sending
     * SEND_STATUS command. Some of the conditions reported in SEND_STATUS are not
     * reported via a data error token.
     */
    if (host_is_spi(card)) 
	{
        err = sd_card_send_cmd_send_status(card, &status);

        if (err != ESP_OK) 
		{
            ESP_LOGE(SD_CARD_TAG, "%s: sd_card_send_cmd_send_status returned 0x%x", __func__, err);
            return err;
        }

        if (status & SD_SPI_R2_CARD_LOCKED) 
		{
            ESP_LOGE(SD_CARD_TAG, "%s: write failed, card is locked: r2=0x%04" PRIx32,
                     __func__, status);
            return ESP_ERR_INVALID_STATE;
        }

        if (status != 0) 
		{
            ESP_LOGE(SD_CARD_TAG, "%s: card status indicates an error after write operation: r2=0x%04" PRIx32,
                     __func__, status);
            return ESP_ERR_INVALID_RESPONSE;
        }
    }
    return ESP_OK;
}