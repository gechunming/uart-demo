#include <stdio.h>
#include <getopt.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>

#include <sys/termios.h>
#include <sys/ioctl.h>
#include <limits.h>
#include <string.h>
#include <signal.h>

int uart_fd;
struct termios termios;
void init_uart()
{
	tcflush(uart_fd, TCIOFLUSH);
	tcgetattr(uart_fd, &termios);

#ifndef __CYGWIN__
	cfmakeraw(&termios);
#else
	termios.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP
                | INLCR | IGNCR | ICRNL | IXON);
	termios.c_oflag &= ~OPOST;
	termios.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	termios.c_cflag &= ~(CSIZE | PARENB);
	termios.c_cflag |= CS8;
#endif

//	termios.c_cflag |= CRTSCTS;
	tcsetattr(uart_fd, TCSANOW, &termios);
	tcflush(uart_fd, TCIOFLUSH);
	tcsetattr(uart_fd, TCSANOW, &termios);
	tcflush(uart_fd, TCIOFLUSH);
	tcflush(uart_fd, TCIOFLUSH);
	cfsetospeed(&termios, B115200);
	cfsetispeed(&termios, B115200);
	tcsetattr(uart_fd, TCSANOW, &termios);
}

unsigned char readbuf[1024];
unsigned char getbuf[1024];
unsigned char tmpe[1024]; 

#define MODE_WRITE 0
#define MODE_READ 1
int mode;
int main(int argc, char *argv[])
{
	int i;
	if (argc < 3) {
		printf("give devname and mode [send, read] ..\n");
		return -1;
	}

	if (strncmp(argv[2], "send", 4) == 0) {
		mode = MODE_WRITE; 
	} else {
		mode = MODE_READ;
	}

	//initialize buf
	unsigned char value=0;
	for (i =  0; i < 1000; i++) {
		readbuf[i] = value++;
	}

	uart_fd = open(argv[1], O_RDWR | O_NOCTTY);
	if (uart_fd == -1) {
		printf("open %s error \n", argv[1]);
		return 0;
	}

	//初始化
	init_uart();

	int j;
	int global_count = 0;
	int send_times=0;
	if (mode == MODE_WRITE) {	
		while(1) {
			printf("input something to send:");
			gets(tmpe);
			if (!strncmp(tmpe, "exit", 4)) {
				return 0;	
			}
			printf("now send 0x00 ... 0x255 bytes\n");
			j = write(uart_fd, readbuf, 255);   
			send_times+=j;
			printf("send %d bytes ok, all send_bytes=%d\n", j, send_times);
		}
	} else {
		while(1) {
			printf("now read something--->\n");
			i = read(uart_fd, getbuf, 1000); 
			if (i == -1) {
				printf("read uart %s error\n", argv[1]);
				return -1;
			}
			global_count += i;
			printf("all=%d,read %d bytes\n", global_count, i);
			for (j = 0; j < i; j++) {
				printf("0x%02x,", getbuf[j]); 
			}
			printf("\n");
			if (global_count >= 512) {
				printf("-----------------------open uart again-----------------------------\n");
				close(uart_fd);
				uart_fd = open(argv[1], O_RDWR | O_NOCTTY);
				if (uart_fd == -1) {
					printf("open %s error\n", argv[1]);
					return -1;
				}
				init_uart();
				global_count = 0;
			}
		}
	}
	close(uart_fd);
	return 0;
}
