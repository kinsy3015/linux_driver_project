#include <stdio.h>
#include <unistd.h> 
#include <fcntl.h> 
#include <sys/mman.h> 
#include <sys/stat.h>
#include <stdlib.h>

#define DEV_NAME	"/dev/mmap"

volatile int i;


void create_data(void){
	int fd;
	int *data = (int *) malloc(sizeof(int)* 10);
	
	printf("%s\n", __func__);
	fd = open(DEV_NAME, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	for (i = 0; i< 10; i++){
		write(fd, (data+i), sizeof(int));
		printf("%4d",*(data+i));
	}
	free(data);
	close(fd);

}

void display_data(void){
	int fd = 0;
	int *data = (int *) malloc(sizeof(int)* 10);

	printf("%s\n", __func__);

	fd = open(DEV_NAME, O_RDONLY);
	for(i=0;i<10;i++){
		if(read(fd, data+i, sizeof(int))==4){
			printf("%4d", *(data+i));
		}
	}
	puts("");
	free(data);
	close(fd);
}

void change_data(void){
	int fd;
	int *data = (int *) malloc(sizeof(int)* 10);

	fd = open(DEV_NAME, O_RDWR); 
	read(fd, data, sizeof(int)); 
	*data += 100; 
	write(fd, data, sizeof(int));
	free(data); 
	close(fd);
}

void mmap_data(void){
	int *mapped = NULL; 
	int fd;   
	fd = open(DEV_NAME, O_RDWR);   
	mapped = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); 
	if (mapped == NULL) return;   
	mapped[7] += 200;   
	msync(mapped, 4096, MS_ASYNC); 
	munmap(mapped, 4096);   
	close(fd);


}

int main(void){
	create_data();
	display_data();
	change_data();
	display_data();
	mmap_data();
	display_data();	
	return 0;
}

