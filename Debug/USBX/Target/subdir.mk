################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USBX/Target/usbx_stm32_port.c 

OBJS += \
./USBX/Target/usbx_stm32_port.o 

C_DEPS += \
./USBX/Target/usbx_stm32_port.d 


# Each subdirectory must supply rules for building sources it contributes
USBX/Target/%.o USBX/Target/%.su USBX/Target/%.cyclo: ../USBX/Target/%.c USBX/Target/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I../Middlewares/ST/usbx/common/core/inc -I../Middlewares/ST/usbx/ports/cortex_m4/gnu/inc -I../Middlewares/ST/usbx/common/usbx_stm32_host_controllers -I../Middlewares/ST/threadx/include -I../Middlewares/ST/filex/common/inc -I../USBX/App -I../USBX/Target -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-USBX-2f-Target

clean-USBX-2f-Target:
	-$(RM) ./USBX/Target/usbx_stm32_port.cyclo ./USBX/Target/usbx_stm32_port.d ./USBX/Target/usbx_stm32_port.o ./USBX/Target/usbx_stm32_port.su

.PHONY: clean-USBX-2f-Target

