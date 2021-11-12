#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h> 
#include <stdlib.h>
 
#define DEV_NAME    "/dev/mmap"
volatile int i;

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
int main(void){
    display_data();
    return 0;
}

