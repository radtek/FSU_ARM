#ifndef USER_LED_H_
#define USER_LED_H_

#ifdef __cplusplus
 extern "C" {
#endif

extern int initLed(void);
extern int ctrlPort(int portNum, int ctrl);
extern void HW_74HC595_Send(__u8 data);
extern void HW_74HC595_Out(__u16 data);
extern void HW_STM8_Out(__u32 data);

#ifdef __cplusplus
}
#endif

#endif /* USER_LED_H_ */
