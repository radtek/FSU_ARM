################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/GetCPUMem.c \
../Src/myComm.c \
../Src/user_can.c \
../Src/user_led.c 

CPP_SRCS += \
../Src/AlarmHdl.cpp \
../Src/AlmFilter.cpp \
../Src/AppConfig.cpp \
../Src/AutoIncNum.cpp \
../Src/B_Interface.cpp \
../Src/DB.cpp \
../Src/DevComDatParse.cpp \
../Src/DevComm.cpp \
../Src/Device.cpp \
../Src/DevsSemaphores.cpp \
../Src/FSU.cpp \
../Src/Gatherer.cpp \
../Src/HFAlarm.cpp \
../Src/Log.cpp \
../Src/LoginLogout.cpp \
../Src/MultiThrdSerial.cpp \
../Src/Pipe.cpp \
../Src/PrtclDat.cpp \
../Src/ReqDevsDats.cpp \
../Src/ReqThresholds.cpp \
../Src/SerialPort.cpp \
../Src/func.cpp \
../Src/protocolX.cpp \
../Src/saveDatInTime.cpp \
../Src/soap.cpp \
../Src/test.cpp \
../Src/typeConvert.cpp \
../Src/xmlHdl.cpp 

OBJS += \
./Src/AlarmHdl.o \
./Src/AlmFilter.o \
./Src/AppConfig.o \
./Src/AutoIncNum.o \
./Src/B_Interface.o \
./Src/DB.o \
./Src/DevComDatParse.o \
./Src/DevComm.o \
./Src/Device.o \
./Src/DevsSemaphores.o \
./Src/FSU.o \
./Src/Gatherer.o \
./Src/GetCPUMem.o \
./Src/HFAlarm.o \
./Src/Log.o \
./Src/LoginLogout.o \
./Src/MultiThrdSerial.o \
./Src/Pipe.o \
./Src/PrtclDat.o \
./Src/ReqDevsDats.o \
./Src/ReqThresholds.o \
./Src/SerialPort.o \
./Src/func.o \
./Src/myComm.o \
./Src/protocolX.o \
./Src/saveDatInTime.o \
./Src/soap.o \
./Src/test.o \
./Src/typeConvert.o \
./Src/user_can.o \
./Src/user_led.o \
./Src/xmlHdl.o 

C_DEPS += \
./Src/GetCPUMem.d \
./Src/myComm.d \
./Src/user_can.d \
./Src/user_led.d 

CPP_DEPS += \
./Src/AlarmHdl.d \
./Src/AlmFilter.d \
./Src/AppConfig.d \
./Src/AutoIncNum.d \
./Src/B_Interface.d \
./Src/DB.d \
./Src/DevComDatParse.d \
./Src/DevComm.d \
./Src/Device.d \
./Src/DevsSemaphores.d \
./Src/FSU.d \
./Src/Gatherer.d \
./Src/HFAlarm.d \
./Src/Log.d \
./Src/LoginLogout.d \
./Src/MultiThrdSerial.d \
./Src/Pipe.d \
./Src/PrtclDat.d \
./Src/ReqDevsDats.d \
./Src/ReqThresholds.d \
./Src/SerialPort.d \
./Src/func.d \
./Src/protocolX.d \
./Src/saveDatInTime.d \
./Src/soap.d \
./Src/test.d \
./Src/typeConvert.d \
./Src/xmlHdl.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-none-linux-gnueabi-g++ -I"/home/vmuser/workspace/FSU_ARM/Inc" -I"/home/vmuser/workspace/FSU_ARM/Inc/libxml" -I"/home/vmuser/workspace/FSU_ARM/Inc/sqlite3" -I"/home/vmuser/workspace/FSU_ARM/Inc/gSoapSTD" -I"/home/vmuser/workspace/FSU_ARM/Inc/Msg" -O0 -g3 -rdynamic -Wall -c -fmessage-length=0 -std=c++11 --static -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Src/GetCPUMem.o: ../Src/GetCPUMem.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-linux-gnueabi-g++ -std=c11 -I"/home/vmuser/workspace/FSU_ARM/Inc" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"Src/GetCPUMem.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Src/myComm.o: ../Src/myComm.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-linux-gnueabi-g++ -I"/home/vmuser/workspace/FSU_ARM/Inc" -O0 -g3 -Wall -c -fmessage-length=0 -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"Src/myComm.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Src/user_can.o: ../Src/user_can.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-linux-gnueabi-gcc -I"/home/vmuser/workspace/FSU_ARM/Inc" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"Src/user_can.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-linux-gnueabi-gcc -std=c11 -I"/home/vmuser/workspace/FSU_ARM/Inc" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


