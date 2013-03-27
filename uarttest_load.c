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

	termios.c_cflag |= CRTSCTS;
	tcsetattr(uart_fd, TCSANOW, &termios);
	tcflush(uart_fd, TCIOFLUSH);
	tcsetattr(uart_fd, TCSANOW, &termios);
	tcflush(uart_fd, TCIOFLUSH);
	tcflush(uart_fd, TCIOFLUSH);
	cfsetospeed(&termios, B115200);
	cfsetispeed(&termios, B115200);
	tcsetattr(uart_fd, TCSANOW, &termios);
}

char readbuf[1024];
char getbuf[1024];
char tmpe[1024]; 

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
	for (i =  0; i < 1000; i+=2) {
		readbuf[i] = 0xaa;
		readbuf[i+1] = 0x55;
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
			printf("now send 0x55, 0xaa, 800 bytes\n");
			global_count = 0;
//			for (j = 0; j < 40; j++) {
			while(1) {
				write(uart_fd, readbuf, 800);   
				send_times++;
				printf("send 800 bytes ok, send_times=%d\n", send_times);
			}
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
			sleep(1);	
			/*
			for (j = 0; j < i; j++) {
				printf("0x%02x,", getbuf[j]); 
			}
			printf("\n");
			*/
		}
	}
	close(uart_fd);
	return 0;
}
