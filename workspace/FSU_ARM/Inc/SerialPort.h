/*
 * SerialPort.h
 *
 *  Created on: 2016-4-16
 *      Author: lcz
 */

#ifndef SERIALPORT_H_
#define SERIALPORT_H_

#define TIMEOUT_SEC(buflen, baud)       (buflen * 20 / baud + 2)
#define TIMEOUT_USEC    0

void SetBaudrate (int);
int  GetBaudrate ();
void SetDataBit (int databit);
int  BAUDRATE (int baudrate);
int _BAUDRATE (int baudrate);
int  SetPortAttr (int fd, int baudrate, int databit, const char *stopbit, char parity);
void SetStopBit (const char *stopbit);
void SetParityCheck (char parity);

//OpenComPort返回值是打开的串口文件描述符fd
int  OpenComPort (char *ComPort, int baudrate, int databit, const char *stopbit, char parity);
void CloseComPort (int fd);
int  ReadComPort (int fd, void *data, int datalength);
int  WriteComPort (int fd, char * data, int datalength);

#endif /* SERIALPORT_H_ */
