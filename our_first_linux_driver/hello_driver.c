/*
 * Install kernel headers (Debian/Ubuntu):
 * sudo apt install linux-headers-$(uname -r)
 *
 * Compile:
 * make
 *
 * Load the driver:
 * sudo insmod hello_driver.ko
 *
 * Check the kernel log for your message:
 * dmesg | tail
 *
 * Unload the driver:
 * sudo rmmod hello_driver
 *
 * Check the log again:
 * dmesg | tail 
 *
 * Clean:
 * make clean
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zuhaitz :3");
MODULE_DESCRIPTION("A simple Hello World kernel module.");
MODULE_VERSION("0.1");

static int __init hello_init(void)
{
    printk(KERN_INFO "Hello, World! Driver loaded.\n");
    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_INFO "Goodbye, World! Driver unloaded.\n");
}

module_init(hello_init);
module_exit(hello_exit);
