/*
  * Create simple kernel module with chardev and implement following functions:
  * * › Basic functions (open, read, write, release);
  * * › Configurations function (ioctl).
  * * › Create STACK data structure on implemented chardev. Use dynamic memory allocation and prepare
  * * data structure for simultaneous access (from multiple threads/processes). Stack must handle
  * * int32_t numbers only.
  * * › read op must act as `pop` from the stack. Write op must as act as `push` to the stack.
  * * › ioctl op must act as configuration operation – ability of changing stack size must be provided.
  * * › Add error codes according to man and cover common corner cases (stack is empty, stack is full and
  * * etc.)
  *
*/

#include <linux/cdev.h> 
#include <linux/delay.h> 
#include <linux/device.h> 
#include <linux/fs.h> 
#include <linux/init.h> 
#include <linux/irq.h> 
#include <linux/kernel.h> /* We are doing kernel work */ 
#include <linux/module.h> /* Specifically, a module */ 
#include <linux/poll.h> 
#include "chardev.h" 
#define SUCCESS 0 
#define DEVICE_NAME "char_dev" 
#define BUF_LEN 80 
 
enum { 
    CDEV_NOT_USED = 0, 
    CDEV_EXCLUSIVE_OPEN = 1, 
}; 
 
static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED); 
 
static char message[BUF_LEN + 1]; 
 
static struct class *cls; 

struct node
{
    int32_t data;
    struct node *link;
}*top = NULL;
 
int MAX = 5;
 

int st_count(void)
{
    int count = 0;
    struct node *temp;
    temp = top;
    while (temp != NULL)
    {
        temp = temp->link;
        count++;
    }
    return count;
}

// to insert elements in stack
int push(int32_t val)
{
    int result = 0;
    int count;
    struct node *temp;
    count = st_count();
    if (count <  MAX)
    {
        temp = (struct node*)kmalloc(sizeof(struct node), GFP_KERNEL);
        temp->data = val;
        temp->link = top;
        top = temp;
    }
    else {
      result = -1;
    }
    return result;
}
 
// to delete elements from stack
int pop(void)
{
    int result = 0;
    struct node *temp;
    if (top == NULL){
      result = -1;
    }
    else
    {
        temp = top;
        top = top->link;
        kfree(temp);
    }
    return result;
}
 
// to count the number of elements
void print_stack(void)
{
    int count = 0;
    struct node *temp;
 
    temp = top;
    while (temp != NULL)
    {
        printk(" %d\n",temp->data);
        temp = temp->link;
        count++;
    }
    printk("size of stack is %d \n",count);
}

/* Called when a process tries to open the device file, like 
 * "sudo cat /dev/char_dev" 
 */ 
static int device_open(struct inode *inode, struct file *file) 
{ 
    pr_info("device_open(%p)\n", file); 
    try_module_get(THIS_MODULE); 
    return SUCCESS; 
} 
 
static int device_release(struct inode *inode, struct file *file) 
{ 
    pr_info("device_release(%p,%p)\n", inode, file); 
    module_put(THIS_MODULE); 
    return SUCCESS; 
} 


static ssize_t device_read(struct file *file, 
                           char __user *buffer,
                           size_t length,
                           loff_t *offset) 
{ 
    int32_t bytes_read = 0; 
    const char *message_ptr = message; 
    message_ptr += *offset; 
    while (length && *message_ptr) { 
        put_user(*(message_ptr++), buffer++); 
        length--; 
        bytes_read++; 
    } 
    pr_info("Read %d bytes, %ld left\n", bytes_read, length); 
    *offset += bytes_read; 
    int result = pop();
    if (result != 0) {
        printk(KERN_ALERT "Error: Something went wrong\n");
    }
    return bytes_read; 
} 
 
static ssize_t device_write(struct file *file, const char __user *buffer, 
                            size_t length, loff_t *offset) 
{ 
    int32_t i; 
    pr_info("device_write(%p,%p,%ld)", file, buffer, length); 
    for (i = 0; i < length && i < BUF_LEN; i++) 
        get_user(message[i], buffer + i); 
    int result =  push((int32_t) message);;
    if (result != 0) {
        printk(KERN_ALERT "Error: Something went wrong\n");
    }
    return i; 
} 
 
static long 
device_ioctl(struct file *file, 
             unsigned int ioctl_num,
             unsigned long ioctl_param) 
{ 
    int i; 
    long ret = SUCCESS; 
    if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN)) 
        return -EBUSY; 
 
    switch (ioctl_num) { 
    case IOCTL_SET_MSG: { 
        int __user *tmp = (int __user *)ioctl_param; 
        int ch; 

        MAX = (int)(int __user *)ioctl_param;
        break; 
    } 
    } 
    atomic_set(&already_open, CDEV_NOT_USED); 
 
    return ret; 
} 
 
/* Module Declarations */ 
static struct file_operations fops = { 
    .read = device_read, 
    .write = device_write, 
    .unlocked_ioctl = device_ioctl, 
    .open = device_open, 
    .release = device_release,
}; 

static int __init chardev2_init(void) 
{ 
    int ret_val = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops); 

    if (ret_val < 0) { 
        pr_alert("%s failed with %d\n", 
                 "Sorry, registering the character device ", ret_val); 
        return ret_val; 
    } 
 
    cls = class_create(THIS_MODULE, DEVICE_FILE_NAME); 
    device_create(cls, NULL, MKDEV(MAJOR_NUM, 0), NULL, DEVICE_FILE_NAME); 
 
    pr_info("Device created on /dev/%s\n", DEVICE_FILE_NAME); 
 
    return 0; 
} 
 
/* Cleanup - unregister the appropriate file from /proc */ 
static void __exit chardev2_exit(void) 
{ 
    device_destroy(cls, MKDEV(MAJOR_NUM, 0)); 
    class_destroy(cls); 
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME); 
} 
 
module_init(chardev2_init); 
module_exit(chardev2_exit); 
 
MODULE_LICENSE("GPL");