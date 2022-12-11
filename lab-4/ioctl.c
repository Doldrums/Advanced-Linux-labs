#include "../chardev.h" 
 
#include <stdio.h> 
#include <fcntl.h>
#include <unistd.h> 
#include <stdlib.h> 
#include <sys/ioctl.h> 
 
/* Functions for the ioctl calls */ 
 
int ioctl_set_msg(int file_desc, int *message) 
{ 
    int ret_val; 
 
    ret_val = ioctl(file_desc, IOCTL_SET_MSG, message); 
 
    if (ret_val < 0) { 
        printf("ioctl_set_msg failed:%d\n", ret_val); 
    } 
 
    return ret_val; 
} 

int main(void) 
{ 
    int file_desc, ret_val; 
    char *msg = "Message passed by ioctl\n"; 
 
    file_desc = open(DEVICE_PATH, O_RDWR); 
    if (file_desc < 0) { 
        printf("Can't open device file: %s, error:%d\n", DEVICE_PATH, 
               file_desc); 
        exit(EXIT_FAILURE); 
    } 
 
    ret_val = ioctl_set_msg(file_desc, msg); 
    if (ret_val) 
        goto error; 
    close(file_desc); 
    return 0; 
error: 
    close(file_desc); 
    exit(EXIT_FAILURE); 
}