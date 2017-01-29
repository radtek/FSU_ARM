################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Src/Msg/InComeMsg.cpp \
../Src/Msg/Message.cpp \
../Src/Msg/MsgGetDatRtnDat.cpp \
../Src/Msg/MsgGetFsuInfoRtnDat.cpp \
../Src/Msg/MsgGetFsuLoginInfoRtnDat.cpp \
../Src/Msg/MsgGetFtpRtnDat.cpp \
../Src/Msg/MsgGetHisDatRtnDat.cpp \
../Src/Msg/MsgGetThresholdRtnDat.cpp \
../Src/Msg/MsgLoginAck.cpp \
../Src/Msg/MsgLogoutAck.cpp \
../Src/Msg/MsgSendAlarmAck.cpp \
../Src/Msg/MsgSetFsuLoginInfoRtnDat.cpp \
../Src/Msg/MsgSetFsuRebootRtnDat.cpp \
../Src/Msg/MsgSetFtpRtnDat.cpp \
../Src/Msg/MsgSetPointRtnDat.cpp \
../Src/Msg/MsgSetThresholdRtnDat.cpp \
../Src/Msg/MsgTimeCheckRtnDat.cpp 

OBJS += \
./Src/Msg/InComeMsg.o \
./Src/Msg/Message.o \
./Src/Msg/MsgGetDatRtnDat.o \
./Src/Msg/MsgGetFsuInfoRtnDat.o \
./Src/Msg/MsgGetFsuLoginInfoRtnDat.o \
./Src/Msg/MsgGetFtpRtnDat.o \
./Src/Msg/MsgGetHisDatRtnDat.o \
./Src/Msg/MsgGetThresholdRtnDat.o \
./Src/Msg/MsgLoginAck.o \
./Src/Msg/MsgLogoutAck.o \
./Src/Msg/MsgSendAlarmAck.o \
./Src/Msg/MsgSetFsuLoginInfoRtnDat.o \
./Src/Msg/MsgSetFsuRebootRtnDat.o \
./Src/Msg/MsgSetFtpRtnDat.o \
./Src/Msg/MsgSetPointRtnDat.o \
./Src/Msg/MsgSetThresholdRtnDat.o \
./Src/Msg/MsgTimeCheckRtnDat.o 

CPP_DEPS += \
./Src/Msg/InComeMsg.d \
./Src/Msg/Message.d \
./Src/Msg/MsgGetDatRtnDat.d \
./Src/Msg/MsgGetFsuInfoRtnDat.d \
./Src/Msg/MsgGetFsuLoginInfoRtnDat.d \
./Src/Msg/MsgGetFtpRtnDat.d \
./Src/Msg/MsgGetHisDatRtnDat.d \
./Src/Msg/MsgGetThresholdRtnDat.d \
./Src/Msg/MsgLoginAck.d \
./Src/Msg/MsgLogoutAck.d \
./Src/Msg/MsgSendAlarmAck.d \
./Src/Msg/MsgSetFsuLoginInfoRtnDat.d \
./Src/Msg/MsgSetFsuRebootRtnDat.d \
./Src/Msg/MsgSetFtpRtnDat.d \
./Src/Msg/MsgSetPointRtnDat.d \
./Src/Msg/MsgSetThresholdRtnDat.d \
./Src/Msg/MsgTimeCheckRtnDat.d 


# Each subdirectory must supply rules for building sources it contributes
Src/Msg/%.o: ../Src/Msg/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-none-linux-gnueabi-g++ -I"/home/vmuser/workspace/FSU_ARM/Inc" -I"/home/vmuser/workspace/FSU_ARM/Inc/libxml" -I"/home/vmuser/workspace/FSU_ARM/Inc/sqlite3" -I"/home/vmuser/workspace/FSU_ARM/Inc/gSoapSTD" -I"/home/vmuser/workspace/FSU_ARM/Inc/Msg" -O0 -g3 -rdynamic -Wall -c -fmessage-length=0 -std=c++11 --static -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


