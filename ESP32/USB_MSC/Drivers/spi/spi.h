#ifndef SPI_H_
#define SPI_H_

/*********************
 *      INCLUDES
 *********************/

#include <stdint.h>

#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "esp_err.h"
#include "esp_log.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*********************
 *      DEFINES
 *********************/

#define SPI_TAG "[spi]"

/**********************
 *      TYPEDEFS
 **********************/

typedef struct _spi_config_t
{
    gpio_num_t pin_mosi;
    gpio_num_t pin_clk;
    gpio_num_t pin_miso;
    gpio_num_t pin_cs;
    uint32_t frequency;
} spi_config_t;

/**********************
 *   PUBLIC FUNCTIONS
 **********************/

static inline esp_err_t 
SPI_Master_DMA_Init(spi_config_t *spi_cfg, spi_bus_config_t *bus_cfg, 
                        int host_spi)
{
    esp_err_t ret;

    ESP_LOGI(SPI_TAG, "Using SPI peripheral");
    ESP_LOGI(SPI_TAG, "SDSPI_MOSI=%d", spi_cfg->pin_mosi);
    ESP_LOGI(SPI_TAG, "SDSPI_MISO=%d", spi_cfg->pin_miso);
    ESP_LOGI(SPI_TAG, "SDSPI_CLK=%d", spi_cfg->pin_clk);
    ESP_LOGI(SPI_TAG, "SDSPI_CS=%d", spi_cfg->pin_cs);

    bus_cfg->mosi_io_num = spi_cfg->pin_mosi;
    bus_cfg->miso_io_num = spi_cfg->pin_miso;
    bus_cfg->sclk_io_num = spi_cfg->pin_clk;
    bus_cfg->quadwp_io_num = -1;
    bus_cfg->quadhd_io_num = -1;
    bus_cfg->max_transfer_sz = spi_cfg->frequency;

    ret = spi_bus_initialize(host_spi, bus_cfg, SPI_DMA_CH_AUTO);
	if (ret != ESP_OK)
	{
		ESP_LOGE(SPI_TAG, "Failed to initialize bus.");
		return ret;
	}

    return ret;
}

#ifdef __cplusplus
}
#endif

#endif /* SPI_H_ */