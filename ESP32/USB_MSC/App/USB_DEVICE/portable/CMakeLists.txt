set(component_srcs "usb_msc_storage.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "inc"
                       REQUIRES spi SD_Card "vfs" "driver" "sdmmc" "fatfs" "wear_levelling"
                       )