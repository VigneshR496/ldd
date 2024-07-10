/***************************************************************************/ /**
                                                                               *  \file       driver.c
                                                                               *
                                                                               *  \details    Simple linux driver (Dynamic allocation of major and minor number)
                                                                               *
                                                                               *  \author     Vignesh R
                                                                               *
                                                                               * *******************************************************************************/
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

dev_t dev = 0;

/*
** Module Init function
*/
static int __init hello_world_init(void) {
  alloc_chrdev_region(&dev, 0, 1, "My_Dev");
  printk(KERN_INFO "Major = %d, Minor = %d\n", MAJOR(dev), MINOR(dev));
  printk(KERN_INFO "Kernel Module Inserted Successfully...\n");
  return 0;
}

/*
** Module Exit function
*/
static void __exit hello_world_exit(void) {
  unregister_chrdev_region(dev, 1);
  printk(KERN_INFO "Kernel Module Removed Successfully...\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vignesh R");
MODULE_DESCRIPTION("A simple hello world driver");
MODULE_VERSION("2:1.0");
