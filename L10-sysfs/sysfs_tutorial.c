/***************************************************************************/ /**
                                                                               *  \file       driver.c
                                                                               *
                                                                               *  \details    Simple linux driver (First Dummy Linux Driver)
                                                                               *
                                                                               *  \author     Vignesh R
                                                                               *
                                                                               * *******************************************************************************/
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h> // copy_to/from_user()
#include <linux/ioctl.h>
#include <linux/sysfs.h>

volatile int eg_val = 0;

dev_t dev = 0;
static struct class *dev_class;
static struct cdev eg_cdev;
struct kobject *kobj_ref;

static ssize_t eg_read(struct file *filp, char __user *buf, size_t len,
                       loff_t *off) {
  pr_info("Data read\n");
  return 0;
}

static ssize_t eg_write(struct file *filp, const char __user *buf, size_t len,
                        loff_t *off) {
  pr_info("Data write\n");
  return len;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = eg_read,
    .write = eg_write,
};

static ssize_t sysfs_show(struct kobject *kobj, 
                struct kobj_attribute *attr, char *buf)
{
  pr_info("Sysfs - Read!!!\n");
  return sprintf(buf, "%d", eg_val);
}

static ssize_t sysfs_store(struct kobject *kobj, 
                struct kobj_attribute *attr,const char *buf, size_t count)
{
  pr_info("Sysfs - Write!!!\n");
  sscanf(buf,"%d",&eg_val);
  return count;
}

struct kobj_attribute eg_attr = __ATTR(eg_val, 0660, sysfs_show, sysfs_store);

/*
** Module Init function
*/
static int __init eg_init(void) {
  if (alloc_chrdev_region(&dev, 0, 1, "My_Dev") < 0) {
    pr_err("Cannot allocate major number for device\n");
    return -1;
  }
  printk(KERN_INFO "Major = %d, Minor = %d\n", MAJOR(dev), MINOR(dev));

  cdev_init(&eg_cdev, &fops);

  if ((cdev_add(&eg_cdev, dev, 1)) < 0) {
    pr_err("Cannot add the device to the system\n");
    goto r_class;
  }

  dev_class = class_create(THIS_MODULE, "my_class");
  if (IS_ERR(dev_class)) {
    pr_err("Cannot create the struct clzss for device\n");
    goto r_class;
  }

  if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "my_device"))) {
    pr_err("Cannot create the Device");
    goto r_device;
  }

 /*Creating a directory in /sys/kernel/ */
  kobj_ref = kobject_create_and_add("eg_sysfs",kernel_kobj);

  /*Creating sysfs file for eg_value*/
  if(sysfs_create_file(kobj_ref,&eg_attr.attr)){
    pr_err("Cannot create sysfs file......\n");
    goto r_sysfs;
  }
    
  printk(KERN_INFO "Device Driver Inserted Successfully...\n");
  return 0;

r_sysfs:
  sysfs_remove_file(kernel_kobj, &eg_attr.attr);
  kobject_put(kobj_ref);

r_device:
  class_destroy(dev_class);

r_class:
  unregister_chrdev_region(dev, 1);

  return -1;
}

/*
** Module Exit function
*/
static void __exit eg_exit(void) {
  sysfs_remove_file(kernel_kobj, &eg_attr.attr);
  kobject_put(kobj_ref);
  device_destroy(dev_class, dev);
  class_destroy(dev_class);
  cdev_del(&eg_cdev);
  unregister_chrdev_region(dev, 1);
  printk(KERN_INFO "Kernel Module Removed Successfully...\n");
}

module_init(eg_init);
module_exit(eg_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vignesh R");
MODULE_DESCRIPTION("A simple hello world driver");
MODULE_VERSION("2:1.0");
