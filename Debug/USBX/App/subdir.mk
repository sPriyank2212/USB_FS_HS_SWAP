################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../USBX/App/app_usbx_host.c \
../USBX/App/ux_host_keyboard.c \
../USBX/App/ux_host_mouse.c \
../USBX/App/ux_host_msc.c 

OBJS += \
./USBX/App/app_usbx_host.o \
./USBX/App/ux_host_keyboard.o \
./USBX/App/ux_host_mouse.o \
./USBX/App/ux_host_msc.o 

C_DEPS += \
./USBX/App/app_usbx_host.d \
./USBX/App/ux_host_keyboard.d \
./USBX/App/ux_host_mouse.d \
./USBX/App/ux_host_msc.d 


# Each subdirectory must supply rules for building sources it contributes
USBX/App/%.o USBX/App/%.su USBX/App/%.cyclo: ../USBX/App/%.c USBX/App/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../USB_HOST/App -I../USB_HOST/Target -I../Middlewares/Third_Party/FatFs/src -I../Middlewares/ST/STM32_USB_Host_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc -I../Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc -I../Middlewares/ST/usbx/common/core/inc -I../Middlewares/ST/usbx/ports/cortex_m4/gnu/inc -I../Middlewares/ST/usbx/common/usbx_stm32_host_controllers -I../Middlewares/ST/threadx/include -I../Middlewares/ST/filex/common/inc -I../USBX/App -I../USBX/Target -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-USBX-2f-App

clean-USBX-2f-App:
	-$(RM) ./USBX/App/app_usbx_host.cyclo ./USBX/App/app_usbx_host.d ./USBX/App/app_usbx_host.o ./USBX/App/app_usbx_host.su ./USBX/App/ux_host_keyboard.cyclo ./USBX/App/ux_host_keyboard.d ./USBX/App/ux_host_keyboard.o ./USBX/App/ux_host_keyboard.su ./USBX/App/ux_host_mouse.cyclo ./USBX/App/ux_host_mouse.d ./USBX/App/ux_host_mouse.o ./USBX/App/ux_host_mouse.su ./USBX/App/ux_host_msc.cyclo ./USBX/App/ux_host_msc.d ./USBX/App/ux_host_msc.o ./USBX/App/ux_host_msc.su

.PHONY: clean-USBX-2f-App

