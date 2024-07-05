set(component_srcs "diskio_spi.c")

idf_component_register(SRCS "${component_srcs}"
                       INCLUDE_DIRS "inc"
                       REQUIRES "driver" "sdmmc" "esp_log"
                       )