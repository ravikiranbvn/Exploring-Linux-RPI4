/*
 * hello_world.c
 * author: Venkata Naga Ravikiran Bulusu
 *
 */

#include <linux/module.h>  // Needed for all kernel modules
#include <linux/kernel.h>  // Needed for KERN_INFO
#include <linux/init.h>    // Needed for the macros

// Function that runs when the module is loaded
static int __init hello_init(void) {
    printk(KERN_INFO "Hello, World!\n");
    return 0;  // Return 0 if successfully loaded
}

// Function that runs when the module is removed
static void __exit hello_exit(void) {
    printk(KERN_INFO "Goodbye, World!\n");
}

// Register the functions
module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");  // Specify the license of the module
MODULE_AUTHOR("ravi");  // Author of the module
MODULE_DESCRIPTION("A simple Hello World kernel module");  // Short description
