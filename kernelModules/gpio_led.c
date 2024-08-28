/*
 * gpio_led.c
 * author: Venkata Naga Ravikiran Bulusu
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

/* Variables for device and device class */
static dev_t         sDevNo;
static struct class *sDevClass;
static struct cdev   sDevice;

#define DRIVER_NAME "elrpi4_led_gpio_driver"
#define DRIVER_CLASS "LED_BUTTON"
#define RED_GPIO_LED 15 + GPIO_DYNAMIC_BASE // 15 + 512           /* GPIO15 with a base of gpiochip0 on 512 */

static ssize_t driver_read(struct file *File, char *user_buffer, size_t count, loff_t *offs) {
    printk("empty read!\n");
	return 0;
}

/**
 * @brief Write data to buffer
 */
static ssize_t driver_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs) {
	int to_copy, not_copied, delta;
	char value;

	/* Get amount of data to copy */
	to_copy = min(count, sizeof(value));

	/* Copy data to user */
	not_copied = copy_from_user(&value, user_buffer, to_copy);

	/* Setting the LED */
	switch(value) {
		case '0':
			gpio_set_value(RED_GPIO_LED, 0);
			break;
		case '1':
			gpio_set_value(RED_GPIO_LED, 1);
			break;
		default:
			printk("Invalid Input!\n");
			break;
	}

	/* Calculate data */
	delta = to_copy - not_copied;

	return delta;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int driver_open(struct inode *device_file, struct file *instance) {
	printk("dev_nr - open was called!\n");
	return 0;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int driver_close(struct inode *device_file, struct file *instance) {
	printk("dev_nr - close was called!\n");
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_close,
	.read = driver_read,
	.write = driver_write
};

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init ModuleInit(void) {
	/* Allocate a device nr */
	if( alloc_chrdev_region(&sDevNo, 0, 1, DRIVER_NAME) < 0) {
		printk("Device Nr. could not be allocated!\n");
		return -1;
	}
	printk("read_write - Device Nr. Major: %d, Minor: %d was registered!\n", sDevNo >> 20, sDevNo & 0xfffff);

	/* Create device class */
	if((sDevClass = class_create(DRIVER_CLASS)) == NULL) {
		printk("Device class can not be created!\n");
		goto ClassError;
	}

	/* create device file */
	if(device_create(sDevClass, NULL, sDevNo, NULL, DRIVER_NAME) == NULL) {
		printk("Can not create device file!\n");
		goto FileError;
	}

	/* Initialize device file */
	cdev_init(&sDevice, &fops);

	/* Regisering device to kernel */
	if(cdev_add(&sDevice, sDevNo, 1) == -1) {
		printk("Registering of device to kernel failed!\n");
		goto AddError;
	}

	/* GPIO 15 init */
	if(gpio_request(RED_GPIO_LED, "rpi-gpio-15")) {
		printk("Can not allocate GPIO 15\n");
		goto AddError;
	}

	/* Set GPIO 15 direction */
	if(gpio_direction_output(RED_GPIO_LED, 0)) {
		printk("Can not set GPIO 15 to output!\n");
		goto Gpio15Error;
	}
    
	return 0;
Gpio15Error:
	gpio_free(RED_GPIO_LED);
AddError:
	device_destroy(sDevClass, sDevNo);
FileError:
	class_destroy(sDevClass);
ClassError:
	unregister_chrdev_region(sDevNo, 1);
	return -1;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit ModuleExit(void) {
	gpio_set_value(RED_GPIO_LED, 0);
	gpio_free(RED_GPIO_LED);
	cdev_del(&sDevice);
	device_destroy(sDevClass, sDevNo);
	class_destroy(sDevClass);
	unregister_chrdev_region(sDevNo, 1);
	printk("Goodbye, Kernel\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ravi");
MODULE_DESCRIPTION("LED example using gpio");