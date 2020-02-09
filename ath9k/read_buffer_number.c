#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>


int main(){
    int ret, fd;
    printf("Starting device test code example...\n");
    fd = open("/dev/ebbchar", O_RDWR);
    if (fd < 0){
        perror("Failed to open the device...");
        return errno;
    }
    for (int i = 0; i < 100; i++) {
        printf("Reading from the device...\n");
        int tmp;
        ret = read(fd, &tmp, sizeof(int));
        printf("This read is %d\n", tmp);
        if (ret < 0){
            perror("Failed to read the message from the device.");
            return errno;
        }
    }
    return 0;
}