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
#include <linux/mutex.h>
#include "chardev.h" 

#define SUCCESS 0 

enum { 
    CDEV_NOT_USED = 0, 
    CDEV_EXCLUSIVE_OPEN = 1, 
}; 
 
static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED); 
static DEFINE_MUTEX(stack_lock);

static struct class *cls; 

struct node
{
    int32_t data;
    struct node *link;
}*top = NULL;
int current_stack_size = 0;

int MAX_STACK_SIZE = 5;

// to insert elements in stack
int push(int32_t val)
{
    mutex_lock(&stack_lock);
    int result = 0;
    struct node *temp;
    if (current_stack_size < MAX_STACK_SIZE)
    {
        temp = (struct node*)kmalloc(sizeof(struct node), GFP_KERNEL);
        temp->data = val;
        temp->link = top;
        top = temp;
        current_stack_size += 1;
    }
    else {
      result = -1;
    }
    mutex_unlock(&stack_lock);
    return result;
}
 
// to delete elements from stack
int32_t pop(void)
{
    mutex_lock(&stack_lock);
    int32_t result;
    struct node *temp;
    if (top == NULL){
      result = -1;
    }
    else
    {
        temp = top;
        result = temp->data;
        printk("Value popped out is %d \n",result);
        top = top->link;
        kfree(temp);
        current_stack_size -= 1;
    }
    mutex_unlock(&stack_lock);
    return result;
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
    int32_t value = pop();

    while (value != -1) {
        size_t bytes_to_read = sizeof(int32_t);
        if (bytes_read + bytes_to_read > length) {
            bytes_to_read = length - bytes_read;
        }

        copy_to_user(buffer + bytes_read, &value, bytes_to_read);
        bytes_read += bytes_to_read;

        if (bytes_read == length) {
            break;
        }

        value = pop();
    }

    return bytes_read;
} 
 
static ssize_t device_write(struct file *file, const char __user *buffer, 
                            size_t length, loff_t *offset) 
{ 
    ssize_t bytes_written = 0;
    int32_t value;

    for (size_t i = 0; i < length; i += sizeof(int32_t)) {
        size_t bytes_to_copy = sizeof(int32_t);
        if (i + bytes_to_copy > length) {
            bytes_to_copy = length - i;
        }
        
        copy_from_user(&value, buffer + i, bytes_to_copy);

        if (push(value) == -1) {
            break;
        }

        bytes_written += bytes_to_copy;
        value = 0;
    }

    if (bytes_written == 0) {
        return -EOVERFLOW;
    }

    return length;
} 
 
static long 
device_ioctl(struct file *file, 
             unsigned int ioctl_num,
             unsigned long ioctl_param) 
{ 
    int ret = SUCCESS; 
    if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN)) 
        return -EBUSY; 
 
    switch (ioctl_num) { 
        case IOCTL_SET_SIZE: { 
            int __user *new_size_user = (int __user *)ioctl_param;
            int new_size;

            if (copy_from_user(&new_size, new_size_user, sizeof(int))) {
                return -EFAULT;
            }

            while (current_stack_size > new_size) {
                pop();
            }

            MAX_STACK_SIZE = new_size;
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
    int ret_val = register_chrdev(MAJOR_NUM, DEVICE_FILE_NAME, &fops); 

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
    unregister_chrdev(MAJOR_NUM, DEVICE_FILE_NAME); 
} 
 
module_init(chardev2_init); 
module_exit(chardev2_exit); 
 
MODULE_LICENSE("GPL");
