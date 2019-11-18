################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/LCD5110S.c \
../src/main.c \
../src/spi.c 

OBJS += \
./src/LCD5110S.o \
./src/main.o \
./src/spi.o 

C_DEPS += \
./src/LCD5110S.d \
./src/main.d \
./src/spi.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -DSTM32F1 -DSTM32F103C8Tx -DSTM32 -DUSE_STDPERIPH_DRIVER -DSTM32F10X_MD -DDEBUG -I"C:/STM32/Eclipse_Projects/STM32F103_LCD5110SPI/inc" -I"C:/STM32/Eclipse_Projects/STM32F103_LCD5110SPI/CMSIS/core" -I"C:/STM32/Eclipse_Projects/STM32F103_LCD5110SPI/StdPeriph_Driver/inc" -I"C:/STM32/Eclipse_Projects/STM32F103_LCD5110SPI/CMSIS/device" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


