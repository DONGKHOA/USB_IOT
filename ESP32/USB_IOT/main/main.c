/*********************
 *      INCLUDES
 *********************/

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_log.h"

#include "lwip/dns.h"

#include "ftp.h"
#include "wifi.h"
#include "nvs_rw.h"

/***********************************
 *   PRIVATE DATA
 ***********************************/

static EventGroupHandle_t xEventTask;

static char *MOUNT_POINT = "/root";

static volatile uint8_t ssid[32] = "Chon Rieng Cafe";
static volatile uint8_t pass[32] = "dinhenoikhecuoiduyen";

/*********************
 *      DEFINES
 *********************/
#define MAIN_TAG  "MAIN"
#define FTP_TASK_FINISH_BIT 1 << 0


uint64_t mp_hal_ticks_ms() 
{
	uint64_t time_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
	return time_ms;
}

/**********************
 *   PUBLIC FUNCTIONS
 **********************/

void ftp_task (void *pvParameters)
{

	ESP_LOGI("[Ftp]", "ftp_task start");

	uint64_t elapsed, time_ms = mp_hal_ticks_ms();
	// Initialize ftp, create rx buffer and mutex
	if (!ftp_init()) {
		ESP_LOGE("[Ftp]", "Init Error");
		xEventGroupSetBits(xEventTask, FTP_TASK_FINISH_BIT);
		vTaskDelete(NULL);
	}

	// We have network connection, enable ftp
	ftp_enable();

	time_ms = mp_hal_ticks_ms();
	while (1) 
    {
		// Calculate time between two ftp_run() calls
		elapsed = mp_hal_ticks_ms() - time_ms;
		time_ms = mp_hal_ticks_ms();

		int res = ftp_run(elapsed);
		if (res < 0) {
			if (res == -1) {
				ESP_LOGE("[Ftp]", "\nRun Error");
			}
			// -2 is returned if Ftp stop was requested by user
			break;
		}

		vTaskDelay(1);

	} // end while

	ESP_LOGW("[Ftp]", "\nTask terminated!");
	xEventGroupSetBits(xEventTask, FTP_TASK_FINISH_BIT);
	vTaskDelete(NULL);
}

void app_main(void)
{
    NVS_Init();
    WIFI_StaInit();
	WIFI_Connect((uint8_t *)ssid, (uint8_t *)pass);

	// xEventTask = xEventGroupCreate();
	// xTaskCreate(ftp_task, "FTP", 1024*6, NULL, 2, NULL);
	// xEventGroupWaitBits( xEventTask,
	// 	FTP_TASK_FINISH_BIT, /* The bits within the event group to wait for. */
	// 	pdTRUE, /* BIT_0 should be cleared before returning. */
	// 	pdFALSE, /* Don't wait for both bits, either bit will do. */
	// 	portMAX_DELAY);/* Wait forever. */
	// ESP_LOGE(MAIN_TAG, "ftp_task finish");
}