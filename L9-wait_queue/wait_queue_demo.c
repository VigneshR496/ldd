/***************************************************************************/ /**
                                                                               *  \file       driver.c
                                                                               *
                                                                               *  \details    Simple linux driver (File operations)
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
#include <linux/wait.h>
#include <linux/signal.h>
#include <linux/kthread.h>

uint32_t read_count = 0;
static struct task_struct *wait_thread;

dev_t dev = 0;
static struct class *dev_class;
static struct cdev eg_cdev;
// DECLARE_WAIT_QUEUE_HEAD(wait_queue_etx);
wait_queue_head_t wait_queue_eg;
int wait_queue_flag = 0;

static int eg_open(struct inode *inode, struct file *file) {
  pr_info("Driver Open function called.\n");
  return 0;
}

static int eg_release(struct inode *inode, struct file *file) {
  pr_info("Driver release function called.\n");
  return 0;
}

static ssize_t eg_read(struct file *filp, char __user *buf, size_t len,
                       loff_t *off) {
  int ret;
  pr_info("Driver Read Function Called...!!!\n");
  wait_queue_flag = 1;
  // ret = send_sig(SIGTERM, wait_thread, 1);   TODO: Figure how to send a signal to wake up a task
  // if (ret) {
  //   pr_err("Failed to send signal to the thread: %d\n", ret);
  // } else {
  //   pr_info("Signal sent to the thread successfully\n");
  // }
  wake_up_interruptible(&wait_queue_eg);
  return 0;
}

static ssize_t eg_write(struct file *filp, const char __user *buf, size_t len,
                        loff_t *off) {
  pr_info("Driver Write Function Called...!!!\n");
  return len;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = eg_read,
    .write = eg_write,
    .open = eg_open,
    .release = eg_release,
};

// Thread function
static int wait_function(void *unused)
{
  while(1) {
    int ret;
    pr_info("Waiting For Event...\n");
    ret = wait_event_interruptible(wait_queue_eg, wait_queue_flag != 0 );
    printk("Debug %d %d\n", ret, -ERESTARTSYS);
    if (ret == -ERESTARTSYS) {
      printk(KERN_INFO "Thread interrupted by a signal\n");
      continue;
    }
    if(wait_queue_flag == 2) {
      pr_info("Event Came From Exit Function\n");
      return 0;
    }
    pr_info("Event Came From Read Function - %d\n", ++read_count);
    wait_queue_flag = 0;
  }
  return 0;
}

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

  //Initialize wait queue
  init_waitqueue_head(&wait_queue_eg);

  //Create the kernel thread
  wait_thread = kthread_create(wait_function, NULL, "WaitThread");
  if (wait_thread) {
    pr_info("Thread Created successfully\n");
    wake_up_process(wait_thread);  // Start the thread
  } else
    pr_info("Thread creation failed\n");

  printk(KERN_INFO "Device Driver Inserted Successfully...\n");
  return 0;

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
  wait_queue_flag = 2;
  wake_up_interruptible(&wait_queue_eg);
  kthread_stop(wait_thread);
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
