#ifndef MY_FILE_H_
#define MY_FILE_H_


#ifdef __cplusplus


 extern "C" {
#endif
#define DEBUG_COMM_LED_CTRL 0
extern int my_init(int select_tty, int baud);
extern int my_open(const char* pathname, int tty, int oflag);
extern int my_close(int fd, int para);
extern int my_config(int fd, int select_tty, int baud, char parity, int len, float stopbits);
extern int my_write(int fd, int para, unsigned char *buf, int count);
extern int my_read(int fd, int para, unsigned char *buf, int count);
extern int heartLoop(void) ;
extern int my_transform(int fd, int tty_select,
		unsigned char *rb,	// receive buffer
		unsigned int * pos,
		unsigned char *buf, unsigned int count);

#ifdef __cplusplus
}
#endif

#endif /* MY_FILE_H_ */

