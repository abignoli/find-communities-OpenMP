################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../binary-search-tree.c \
../community-computation-commons.c \
../community-computation-weighted-sequential.c \
../community-computation-weighted.c \
../community-development.c \
../community-exchange.c \
../dynamic-graph.c \
../dynamic-weighted-graph.c \
../main-test.c \
../main.c \
../parse-args.c \
../shared-graph.c \
../sorted-linked-list.c \
../temporary-community-edge.c \
../tmp.c \
../utilities.c 

OBJS += \
./binary-search-tree.o \
./community-computation-commons.o \
./community-computation-weighted-sequential.o \
./community-computation-weighted.o \
./community-development.o \
./community-exchange.o \
./dynamic-graph.o \
./dynamic-weighted-graph.o \
./main-test.o \
./main.o \
./parse-args.o \
./shared-graph.o \
./sorted-linked-list.o \
./temporary-community-edge.o \
./tmp.o \
./utilities.o 

C_DEPS += \
./binary-search-tree.d \
./community-computation-commons.d \
./community-computation-weighted-sequential.d \
./community-computation-weighted.d \
./community-development.d \
./community-exchange.d \
./dynamic-graph.d \
./dynamic-weighted-graph.d \
./main-test.d \
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
	gcc -O2 -g -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

