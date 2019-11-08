################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../CMSIS/device/system_stm32f10x.c 

OBJS += \
./CMSIS/device/system_stm32f10x.o 

C_DEPS += \
./CMSIS/device/system_stm32f10x.d 


# Each subdirectory must supply rules for building sources it contributes
CMSIS/device/%.o: ../CMSIS/device/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DSTM32F1 -DSTM32F103C8Tx -DSTM32 -DUSE_STDPERIPH_DRIVER -DSTM32F10X_MD -DDEBUG -I"C:/STM32/Eclipse_Projects/STM32F103_UARTx/inc" -I"C:/STM32/Eclipse_Projects/STM32F103_UARTx/CMSIS/core" -I"C:/STM32/Eclipse_Projects/STM32F103_UARTx/StdPeriph_Driver/inc" -I"C:/STM32/Eclipse_Projects/STM32F103_UARTx/CMSIS/device" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


