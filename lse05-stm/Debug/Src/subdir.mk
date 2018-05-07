################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/fsm.c \
../Src/l3gd20.c \
../Src/lsm303dlhc.c \
../Src/main.c \
../Src/parser.c \
../Src/ring_buf.c \
../Src/sensor.c \
../Src/stm32f411e_discovery.c \
../Src/stm32f411e_discovery_accelerometer.c \
../Src/stm32f411e_discovery_gyroscope.c \
../Src/stm32f4xx_hal_msp.c \
../Src/stm32f4xx_it.c \
../Src/system_stm32f4xx.c \
../Src/uart.c \
../Src/usb.c \
../Src/usb_device.c \
../Src/usbd_cdc_if.c \
../Src/usbd_conf.c \
../Src/usbd_desc.c 

OBJS += \
./Src/fsm.o \
./Src/l3gd20.o \
./Src/lsm303dlhc.o \
./Src/main.o \
./Src/parser.o \
./Src/ring_buf.o \
./Src/sensor.o \
./Src/stm32f411e_discovery.o \
./Src/stm32f411e_discovery_accelerometer.o \
./Src/stm32f411e_discovery_gyroscope.o \
./Src/stm32f4xx_hal_msp.o \
./Src/stm32f4xx_it.o \
./Src/system_stm32f4xx.o \
./Src/uart.o \
./Src/usb.o \
./Src/usb_device.o \
./Src/usbd_cdc_if.o \
./Src/usbd_conf.o \
./Src/usbd_desc.o 

C_DEPS += \
./Src/fsm.d \
./Src/l3gd20.d \
./Src/lsm303dlhc.d \
./Src/main.d \
./Src/parser.d \
./Src/ring_buf.d \
./Src/sensor.d \
./Src/stm32f411e_discovery.d \
./Src/stm32f411e_discovery_accelerometer.d \
./Src/stm32f411e_discovery_gyroscope.d \
./Src/stm32f4xx_hal_msp.d \
./Src/stm32f4xx_it.d \
./Src/system_stm32f4xx.d \
./Src/uart.d \
./Src/usb.d \
./Src/usb_device.d \
./Src/usbd_cdc_if.d \
./Src/usbd_conf.d \
./Src/usbd_desc.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DUSE_HAL_DRIVER -DSTM32F411xE '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -I"/home/pablo/MISE/LSEL/proyecto-final-lse05/lse05-stm/Inc" -I"/home/pablo/MISE/LSEL/proyecto-final-lse05/lse05-stm/Drivers/STM32F4xx_HAL_Driver/Inc" -I"/home/pablo/MISE/LSEL/proyecto-final-lse05/lse05-stm/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/home/pablo/MISE/LSEL/proyecto-final-lse05/lse05-stm/Middlewares/ST/STM32_USB_Device_Library/Core/Inc" -I"/home/pablo/MISE/LSEL/proyecto-final-lse05/lse05-stm/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc" -I"/home/pablo/MISE/LSEL/proyecto-final-lse05/lse05-stm/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"/home/pablo/MISE/LSEL/proyecto-final-lse05/lse05-stm/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


