################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/lcd.c \
../src/main.c 

OBJS += \
./src/lcd.o \
./src/main.o 

C_DEPS += \
./src/lcd.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DSTM32F1 -DSTM32F103C8Tx -DSTM32 -DUSE_STDPERIPH_DRIVER -DSTM32F10X_MD -DDEBUG -I"C:/STM32/Eclipse_Projects/STM32F103_LCDx/inc" -I"C:/STM32/Eclipse_Projects/STM32F103_LCDx/CMSIS/core" -I"C:/STM32/Eclipse_Projects/STM32F103_LCDx/StdPeriph_Driver/inc" -I"C:/STM32/Eclipse_Projects/STM32F103_LCDx/CMSIS/device" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


