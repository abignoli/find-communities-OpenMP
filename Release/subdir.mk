################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../algorithm-executor.c \
../community-computation-commons.c \
../community-computation-weighted-sequential.c \
../community-computation-weighted.c \
../community-development.c \
../community-exchange.c \
../dynamic-graph.c \
../dynamic-weighted-graph.c \
../execution-briefing.c \
../execution-handler.c \
../execution-settings.c \
../input-handler.c \
../main.c \
../neighbor-computation-package.c \
../parse-args.c \
../shared-graph.c \
../sorted-linked-list.c \
../temporary-community-edge.c \
../utilities.c \
../version-parallel-naive-partitioning.c \
../version-parallel-sort-select-chunks.c \
../vertex-following.c 

OBJS += \
./algorithm-executor.o \
./community-computation-commons.o \
./community-computation-weighted-sequential.o \
./community-computation-weighted.o \
./community-development.o \
./community-exchange.o \
./dynamic-graph.o \
./dynamic-weighted-graph.o \
./execution-briefing.o \
./execution-handler.o \
./execution-settings.o \
./input-handler.o \
./main.o \
./neighbor-computation-package.o \
./parse-args.o \
./shared-graph.o \
./sorted-linked-list.o \
./temporary-community-edge.o \
./utilities.o \
./version-parallel-naive-partitioning.o \
./version-parallel-sort-select-chunks.o \
./vertex-following.o 

C_DEPS += \
./algorithm-executor.d \
./community-computation-commons.d \
./community-computation-weighted-sequential.d \
./community-computation-weighted.d \
./community-development.d \
./community-exchange.d \
./dynamic-graph.d \
./dynamic-weighted-graph.d \
./execution-briefing.d \
./execution-handler.d \
./execution-settings.d \
./input-handler.d \
./main.d \
./neighbor-computation-package.d \
./parse-args.d \
./shared-graph.d \
./sorted-linked-list.d \
./temporary-community-edge.d \
./utilities.d \
./version-parallel-naive-partitioning.d \
./version-parallel-sort-select-chunks.d \
./vertex-following.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -fopenmp -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


