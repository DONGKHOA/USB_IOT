################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Components/UART/uart.c 

OBJS += \
./Components/UART/uart.o 

C_DEPS += \
./Components/UART/uart.d 


# Each subdirectory must supply rules for building sources it contributes
Components/UART/%.o Components/UART/%.su Components/UART/%.cyclo: ../Components/UART/%.c Components/UART/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/USB_IOT/STM32/Components/File_Handling" -I"D:/USB_IOT/STM32/Components/UART" -I"D:/USB_IOT/STM32/Components" -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Components-2f-UART

clean-Components-2f-UART:
	-$(RM) ./Components/UART/uart.cyclo ./Components/UART/uart.d ./Components/UART/uart.o ./Components/UART/uart.su

.PHONY: clean-Components-2f-UART

