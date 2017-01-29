################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Src/gSoapSTD/fsuC.cpp \
../Src/gSoapSTD/fsuFSUServiceSoapBindingService.cpp \
../Src/gSoapSTD/scC.cpp \
../Src/gSoapSTD/scSCServiceSoapBindingProxy.cpp 

OBJS += \
./Src/gSoapSTD/fsuC.o \
./Src/gSoapSTD/fsuFSUServiceSoapBindingService.o \
./Src/gSoapSTD/scC.o \
./Src/gSoapSTD/scSCServiceSoapBindingProxy.o 

CPP_DEPS += \
./Src/gSoapSTD/fsuC.d \
./Src/gSoapSTD/fsuFSUServiceSoapBindingService.d \
./Src/gSoapSTD/scC.d \
./Src/gSoapSTD/scSCServiceSoapBindingProxy.d 


# Each subdirectory must supply rules for building sources it contributes
Src/gSoapSTD/%.o: ../Src/gSoapSTD/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-none-linux-gnueabi-g++ -I"/home/vmuser/workspace/FSU_ARM/Inc" -I"/home/vmuser/workspace/FSU_ARM/Inc/libxml" -I"/home/vmuser/workspace/FSU_ARM/Inc/sqlite3" -I"/home/vmuser/workspace/FSU_ARM/Inc/gSoapSTD" -I"/home/vmuser/workspace/FSU_ARM/Inc/Msg" -O0 -g3 -rdynamic -Wall -c -fmessage-length=0 -std=c++11 --static -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


