################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/PitchDetectionAlg.c \
../Source/Recording.c \
../Source/main.c 

OBJS += \
./Source/PitchDetectionAlg.o \
./Source/Recording.o \
./Source/main.o 

C_DEPS += \
./Source/PitchDetectionAlg.d \
./Source/Recording.d \
./Source/main.d 


# Each subdirectory must supply rules for building sources it contributes
Source/%.o: ../Source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-gcc -I"C:\Users\PC\Google Drive\Eclipse workspace\BBBIO" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


