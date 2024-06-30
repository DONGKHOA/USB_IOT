################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Component/UART/uart.c 

OBJS += \
./Component/UART/uart.o 

C_DEPS += \
./Component/UART/uart.d 


# Each subdirectory must supply rules for building sources it contributes
Component/UART/%.o Component/UART/%.su Component/UART/%.cyclo: ../Component/UART/%.c Component/UART/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc -I"D:/USB_IOT/STM32/Component/File_Handling" -I"D:/USB_IOT/STM32/Component/UART" -I"D:/USB_IOT/STM32/Component" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Component-2f-UART

clean-Component-2f-UART:
	-$(RM) ./Component/UART/uart.cyclo ./Component/UART/uart.d ./Component/UART/uart.o ./Component/UART/uart.su

.PHONY: clean-Component-2f-UART

