################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../community-computation-commons.c \
../community-computation-weighted-sequential.c \
../community-computation-weighted.c \
../community-development.c \
../community-exchange.c \
../dynamic-graph.c \
../dynamic-weighted-graph.c \
../execution-handler.c \
../input-handler.c \
../main.c \
../parse-args.c \
../shared-graph.c \
../sorted-linked-list.c \
../temporary-community-edge.c \
../tmp.c \
../utilities.c 

OBJS += \
./community-computation-commons.o \
./community-computation-weighted-sequential.o \
./community-computation-weighted.o \
./community-development.o \
./community-exchange.o \
./dynamic-graph.o \
./dynamic-weighted-graph.o \
./execution-handler.o \
./input-handler.o \
./main.o \
./parse-args.o \
./shared-graph.o \
./sorted-linked-list.o \
./temporary-community-edge.o \
./tmp.o \
./utilities.o 

C_DEPS += \
./community-computation-commons.d \
./community-computation-weighted-sequential.d \
./community-computation-weighted.d \
./community-development.d \
./community-exchange.d \
./dynamic-graph.d \
./dynamic-weighted-graph.d \
./execution-handler.d \
./input-handler.d \
./main.d \
./parse-args.d \
./shared-graph.d \
./sorted-linked-list.d \
./temporary-community-edge.d \
./tmp.d \
./utilities.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -fopenmp -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


