/**
 * @file chardev.c
 * @author Neil D'Souza
 * @date 26 September 2017
 * @version 4.12.8
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>


int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define CLASS_NAME "devchar"
#define DEVICE_NAME "s3600251Device" 
#define BUF_LEN 100

MODULE_LICENSE("GPL");	

static int Major;
static int Device_Open = 0;  

static int msg_len = 0;

static struct class* chardevClass = NULL; ///< device class struct pointer

struct virtual_device {
	char data[100];
	struct semaphore sem;
}myDevice;

static struct device* chardevDevice = NULL;

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

/* Functions */

int init_module(void) {
	Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0) {
		printk("Registering the character device failed with %d\n", Major);
	}

	printk("<1>I was assigned major number %d\n", Major);
	
	chardevClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(chardevClass)) {
		unregister_chrdev(Major, DEVICE_NAME);
		printk(KERN_ALERT "Failed to register device class\n");
		return PTR_ERR(chardevDevice);
	}
	printk(KERN_INFO "CHARDEV: Device class registered correctly\n");

	// Register the device driver
	chardevDevice = device_create(chardevClass, NULL, 
		MKDEV(Major, 0), NULL, DEVICE_NAME);
	if (IS_ERR(chardevClass)) {
		class_destroy(chardevClass);
		unregister_chrdev(Major, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the devices\n");
		return PTR_ERR(chardevDevice);
	}
	printk(KERN_INFO "CHARDEV: Character device created correctly\n");
	return 0;
}

void cleanup_module(void) {
	device_destroy(chardevClass, MKDEV(Major, 0));
	class_unregister(chardevClass);
	unregister_chrdev(Major, DEVICE_NAME);
	printk(KERN_INFO "CHARDEV: Goodbye from the LKM!\n");
}

/* Methods */

/* Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */

static int device_open(struct inode *inode, struct file *file) {
	if (Device_Open) return -EBUSY;

	Device_Open++;
	try_module_get(THIS_MODULE);

	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file) {
	Device_Open--;

	module_put(THIS_MODULE);	

	return 0;
}

static ssize_t device_read(struct file *flip, 
char *buffer,
size_t length,
loff_t *offset) {
	int error_count = 0;

	error_count = copy_to_user(buffer, myDevice.data, msg_len);
	
	if (error_count) {
		printk(KERN_INFO "CHARDEV: %d errors reading\n", error_count);
		return -EFAULT;
	}

	printk(KERN_INFO "CHARDEV: Sent %d characters to user\n", msg_len);
	return msg_len;
}

static ssize_t device_write(struct file *fpli,
const char *buffer,
size_t len,
    loff_t *off) {
	sprintf(myDevice.data, "%s(%zu letters)", buffer, len);
	msg_len += strlen(myDevice.data);
	printk(KERN_INFO "CHARDEV: Received %zu characters from the user\n", len);
	return len;	
}
	
