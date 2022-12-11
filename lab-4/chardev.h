/* 
 * chardev.h - the header file with the ioctl definitions. 
 */ 
 
#ifndef CHARDEV_H 
#define CHARDEV_H 
 
#include <linux/ioctl.h> 

#define MAJOR_NUM 100 
 
#define IOCTL_SET_MSG _IOW(MAJOR_NUM, 0, int *) 

#define DEVICE_FILE_NAME "char_dev" 
#define DEVICE_PATH "/dev/char_dev" 
 
#endif