/***************************************************************************/ /**
                                                                               *  \file       driver.c
                                                                               *
                                                                               *  \details    Simple hello world driver
                                                                               *
                                                                               *  \author     Vignesh R
                                                                               *
                                                                               * *******************************************************************************/
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

/*
** Module Init function
*/
static int __init hello_world_init(void) {
  printk(KERN_INFO "This is Hello World Module\n");
  printk(KERN_INFO "Kernel Module Inserted Successfully...\n");
  return 0;
}

/*
** Module Exit function
*/
static void __exit hello_world_exit(void) {
  printk(KERN_INFO "Kernel Module Removed Successfully...\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vignesh R");
MODULE_DESCRIPTION("A simple hello world driver");
MODULE_VERSION("2:1.0");
