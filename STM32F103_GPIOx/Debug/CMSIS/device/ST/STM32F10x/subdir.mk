################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CMSIS/device/ST/STM32F10x/system_stm32f10x.c 

OBJS += \
./CMSIS/device/ST/STM32F10x/system_stm32f10x.o 

C_DEPS += \
./CMSIS/device/ST/STM32F10x/system_stm32f10x.d 


# Each subdirectory must supply rules for building sources it contributes
CMSIS/device/ST/STM32F10x/%.o: ../CMSIS/device/ST/STM32F10x/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DSTM32F1 -DSTM32F103C8Tx -DSTM32 -DUSE_STDPERIPH_DRIVER -DSTM32F10X_MD -DDEBUG -I"../"D:/Micro/STM32/Eclipse_Projects/STM32F103_GPIOx/inc" -I"../"D:/Micro/STM32/Eclipse_Projects/STM32F103_GPIOx/CMSIS/core" -I"../"D:/Micro/STM32/Eclipse_Projects/STM32F103_GPIOx/CMSIS/device" -I"../"D:/Micro/STM32/Eclipse_Projects/STM32F103_GPIOx/StdPeriph_Driver/inc" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


