################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
F:/OtvoreniProjekti/HotelUlm/fw/ULM_RC1/Common/common.c 

OBJS += \
./Common/common.o 

C_DEPS += \
./Common/common.d 


# Each subdirectory must supply rules for building sources it contributes
Common/common.o: F:/OtvoreniProjekti/HotelUlm/fw/ULM_RC1/Common/common.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DSTM32F030x6 -DDEBUG -DCARD_STACKER -DAPPLICATION -c -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/CMSIS/Include -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -Og -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Common/common.d" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

