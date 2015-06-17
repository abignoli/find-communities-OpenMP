################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Copy\ of\ version-parallel-naive-partitioning.c \
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
../tmp.c \
../utilities.c \
../version-parallel-naive-partitioning.c \
../version-parallel-sort-select-chunks.c 

OBJS += \
./Copy\ of\ version-parallel-naive-partitioning.o \
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
./tmp.o \
./utilities.o \
./version-parallel-naive-partitioning.o \
./version-parallel-sort-select-chunks.o 

C_DEPS += \
./Copy\ of\ version-parallel-naive-partitioning.d \
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
./tmp.d \
./utilities.d \
./version-parallel-naive-partitioning.d \
./version-parallel-sort-select-chunks.d 


# Each subdirectory must supply rules for building sources it contributes
Copy\ of\ version-parallel-naive-partitioning.o: ../Copy\ of\ version-parallel-naive-partitioning.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -fopenmp -O3 -g3 -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"Copy of version-parallel-naive-partitioning.d" -MT"Copy\ of\ version-parallel-naive-partitioning.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -fopenmp -O3 -g3 -pg -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


