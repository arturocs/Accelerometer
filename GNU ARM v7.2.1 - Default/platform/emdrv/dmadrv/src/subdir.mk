################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../platform/emdrv/dmadrv/src/dmadrv.c 

OBJS += \
./platform/emdrv/dmadrv/src/dmadrv.o 

C_DEPS += \
./platform/emdrv/dmadrv/src/dmadrv.d 


# Each subdirectory must supply rules for building sources it contributes
platform/emdrv/dmadrv/src/dmadrv.o: ../platform/emdrv/dmadrv/src/dmadrv.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM C Compiler'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m4 -mthumb -std=c99 '-D__STACK_SIZE=0x800' '-D__StackLimit=0x20000000' '-D__HEAP_SIZE=0xD00' '-DHAL_CONFIG=1' '-DEFR32MG12P332F1024GL125=1' -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\Device\SiliconLabs\EFR32MG12P\Include" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\emlib\src" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\hardware\kit\common\halconfig" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\CMSIS\Include" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\emlib\inc" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\hardware\kit\common\drivers" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\protocol\bluetooth\ble_stack\inc\common" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\hardware\kit\EFR32MG12_BRD4162A\config" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\emdrv\sleep\src" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\app\bluetooth\common\util" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\hardware\kit\common\bsp" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\service\sleeptimer\config" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\emdrv\sleep\inc" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\emdrv\common\inc" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\radio\rail_lib\chip\efr32\efr32xg1x" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\radio\rail_lib\common" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\emdrv\gpiointerrupt\inc" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\protocol\bluetooth\ble_stack\inc\soc" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\service\sleeptimer\src" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\emdrv\uartdrv\inc" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\bootloader\api" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\Device\SiliconLabs\EFR32MG12P\Source\GCC" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\common\inc" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\halconfig\inc\hal-config" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\radio\rail_lib\protocol\ieee802154" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\service\sleeptimer\inc" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\Device\SiliconLabs\EFR32MG12P\Source" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\radio\rail_lib\protocol\ble" -I"C:\Users\Arturo\SimplicityStudio\v4_workspace\Copy of soc-empty_4\platform\bootloader" -O2 -Wall -c -fmessage-length=0 -ffunction-sections -fdata-sections -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -MMD -MP -MF"platform/emdrv/dmadrv/src/dmadrv.d" -MT"platform/emdrv/dmadrv/src/dmadrv.o" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


