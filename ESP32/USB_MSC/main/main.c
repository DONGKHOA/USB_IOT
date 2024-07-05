#include <stdint.h>


#include "sd_card.h"

void app_main()
{
    sdmmc_card_t card;
    mountSDCard((char *)MOUNT_POINT, &card);
    
}