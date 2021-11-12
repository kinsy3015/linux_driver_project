#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
//#include <linux/pm.h>
//#include <linux/suspend.h>
//#include <linux/freezer.h>
#define DEV_NAME	"/dev/touch_sus0"
#define BUF_MAX		128


int main(void){
	int fd;
	int *data = (int *) malloc(sizeof(int)*BUF_MAX);
	int dat[] = { 11, 22, 33, 0 }; 
	fd = open(DEV_NAME, O_RDWR);
	write(fd, dat, sizeof(int));
	read(fd, data, sizeof(int));
	//set_freezable();
	//freeze_processes();	
	//suspend_device_irqs();
	//pm_suspend();
	free(data);
	close(fd);
	printf("hello_sus");
	return 0;
}
