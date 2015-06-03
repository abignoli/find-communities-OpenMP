################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../community-development.c \
../dynamic-graph.c \
../dynamic-weighted-graph.c \
../main.c \
../shared-graph.c \
../sorted-linked-list.c \
../utilities.c 

OBJS += \
./community-development.o \
./dynamic-graph.o \
./dynamic-weighted-graph.o \
./main.o \
./shared-graph.o \
./sorted-linked-list.o \
./utilities.o 

C_DEPS += \
./community-development.d \
./dynamic-graph.d \
./dynamic-weighted-graph.d \
./main.d \
./shared-graph.d \
./sorted-linked-list.d \
./utilities.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


