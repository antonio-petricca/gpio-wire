#ifndef _GPIOWIRE_H_
#define _GPIOWIRE_H_

/**
 * @file    gpiowire.c
 * @author  Antonio Petricca (antonio.petricca@gmail.com)
 * @date    26 Maj 2017
 * @version 0.0.1
 * @brief   An implementation of a RF pseudo serial protocol for cheap devices.
 * @see     http://www.romanblack.com/RF/cheapRFmodules.htm
*/

#include <linux/device.h>  // Header to support the kernel Driver Model
#include <linux/fs.h>      // Header for the Linux file system support
#include <linux/gpio/consumer.h> // Required for the GPIO functions
#include <linux/hrtimer.h> // High Resolution Timers
#include <linux/init.h>    // Macros used to mark up functions __init __exit
#include <linux/kernel.h>  // Contains types, macros, functions for the kernel
#include <linux/kobject.h> // Using kobjects for the sysfs bindings
#include <linux/ktime.h>   // ktime_get, ...
#include <linux/module.h>  // Core header for loading LKMs into the kernel
#include <linux/mutex.h>   // Required for the mutex functionality
#include <linux/slab.h>    // kmalloc / kfree
#include <linux/uaccess.h> // Required for the copy to user function

// Manifest

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Antonio Petricca (antonio.petricca@gmail.com)");
MODULE_DESCRIPTION("GPIO pseudo serial protocol.");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");

// Types

enum prot_edge
{
	EDGE_LOW  = 0,
	EDGE_HIGH = 1
};

struct perf_data
{
  enum prot_edge next_edge;
  ktime_t        time;
};

struct prot_ctx
{
  struct completion sem;
  ktime_t           time;

  size_t            perf_count;
  struct perf_data  *perf_data;

  int               sync_count;

  char              *message;
  size_t            msg_count;
  int               bit_count;
  enum prot_edge    next_edge;
};

struct device_data
{
  // Device

  int            dev_number;
  struct kobject *kobj;
  struct mutex   mutex;

  // Attributes

  bool          attr_perf_debug;
  unsigned int  attr_pin_number;
  bool          attr_can_sleep;
  bool          attr_swap_output;

  int           attr_sync_bit_count;

  unsigned long attr_high_state;
  unsigned long attr_zero_bit;
  unsigned long attr_one_bit;
  unsigned long attr_sync_bit;

  // Pre-calculated edges duration (for faster performances)

  ktime_t edge_high_state;
  ktime_t edge_zero_bit;
  ktime_t edge_one_bit;
  ktime_t edge_sync_bit;

  // Protocol

  struct hrtimer  timer;
  struct prot_ctx prot_ctx;
};

// Prototypes

ssize_t perfDebug_show(
  struct kobject        *kobj,
  struct kobj_attribute *attr,
  char                  *buf
);

ssize_t perfDebug_store(
  struct kobject        *kobj,
  struct kobj_attribute *attr,
  const char            *buf,
  size_t                count
);

ssize_t pinNumber_show(
  struct kobject        *kobj,
  struct kobj_attribute *attr,
  char                  *buf
);

ssize_t canSleep_show(
  struct kobject        *kobj,
  struct kobj_attribute *attr,
  char                  *buf
);

ssize_t canSleep_store(
  struct kobject        *kobj,
  struct kobj_attribute *attr,
  const char            *buf,
  size_t                count
);

ssize_t pinNumber_store(
  struct kobject        *kobj,
  struct kobj_attribute *attr,
  const char            *buf,
  size_t                count
);

ssize_t swapOutput_show(
  struct kobject        *kobj,
  struct kobj_attribute *attr,
  char                  *buf
);

ssize_t swapOutput_store(
  struct kobject        *kobj,
  struct kobj_attribute *attr,
  const char            *buf,
  size_t                count
);

ssize_t highStateEdge_show(
  struct kobject        *kobj,
  struct kobj_attribute *attr,
  char                  *buf
);

ssize_t highStateEdge_store(
  struct kobject        *kobj,
  struct kobj_attribute *attr,
  const char            *buf,
  size_t                count
);

ssize_t bitZeroDuration_show(
  struct kobject        *kobj,
  struct kobj_attribute *attr,
  char                  *buf
);

ssize_t bitZeroDuration_store(
  struct kobject        *kobj,
  struct kobj_attribute *attr,
  const char            *buf,
  size_t                count
);

ssize_t bitOneDuration_show(
  struct kobject        *kobj,
  struct kobj_attribute *attr,
  char                  *buf
);

ssize_t bitOneDuration_store(
  struct kobject        *kobj,
  struct kobj_attribute *attr,
  const char            *buf,
  size_t                count
);

ssize_t bitSyncCount_show(
  struct kobject        *kobj,
  struct kobj_attribute *attr,
  char                  *buf
);

ssize_t bitSyncCount_store(
  struct kobject        *kobj,
  struct kobj_attribute *attr,
  const char            *buf,
  size_t                count
);

ssize_t bitSyncDuration_show(
  struct kobject        *kobj,
  struct kobj_attribute *attr,
  char                  *buf
);

ssize_t bitSyncDuration_store(
  struct kobject        *kobj,
  struct kobj_attribute *attr,
  const char            *buf,
  size_t                count
);

bool       file_trylock(struct device_data* data);

int        file_open(struct inode *inodep, struct file *filep);
int        file_release(struct inode *inodep, struct file *filep);

ssize_t    file_write(
  struct file       *filep,
  const char __user *buffer,
  size_t            len,
  loff_t            *offset
);

inline ssize_t prot_write_message(
  struct device_data *data,
  const char         *buffer,
  size_t             len
);


// Globals

#define _CLASS_NAME           "gpiowires"
#define _DEVICE_NAME          "gpiowire%d"

static int                    major_mumber;

static struct class           *dev_class   = NULL;
static struct device*         *dev_list    = NULL;
static int                    dev_count    = 0;

static struct file_operations dev_file_ops =
{
   .owner   = THIS_MODULE,

   .open    = file_open,
   .release = file_release,
   .write   = file_write
};

static struct device_data def_dev_data =
{
  .attr_perf_debug     = false,
  .attr_pin_number     = -1,
  .attr_can_sleep      = false,
  .attr_swap_output    = false,

  .attr_sync_bit_count = 5,

  .attr_high_state     = 500,
  .attr_zero_bit       = 1000,
  .attr_one_bit        = 2000,
  .attr_sync_bit       = 5000
};

// Debug macros

#define LOG(sev, fmt, ...) \
  pr_##sev(_CLASS_NAME ": " fmt, ##__VA_ARGS__)

#define LOG_DEV(sev, fmt, ...) \
  pr_##sev(_CLASS_NAME "[%d]: " fmt, data->dev_number, ##__VA_ARGS__)

// Parameters

static uint devicesNumber = 1;
module_param(devicesNumber, uint, S_IRUGO);
MODULE_PARM_DESC(devicesNumber, " number of handled device (default: 1).");

// Attributes

#define DEFINE_ATTRIBUTE(attrName) \
  struct kobj_attribute attrName##_attr = __ATTR_RW(attrName)

DEFINE_ATTRIBUTE(perfDebug);
DEFINE_ATTRIBUTE(pinNumber);
DEFINE_ATTRIBUTE(canSleep);
DEFINE_ATTRIBUTE(swapOutput);
DEFINE_ATTRIBUTE(highStateEdge);
DEFINE_ATTRIBUTE(bitZeroDuration);
DEFINE_ATTRIBUTE(bitOneDuration);
DEFINE_ATTRIBUTE(bitSyncDuration);
DEFINE_ATTRIBUTE(bitSyncCount);

struct attribute *dev_attrs[] = {
  &perfDebug_attr.attr,
  &pinNumber_attr.attr,
  &canSleep_attr.attr,
  &swapOutput_attr.attr,
  &highStateEdge_attr.attr,
  &bitZeroDuration_attr.attr,
  &bitOneDuration_attr.attr,
  &bitSyncDuration_attr.attr,
  &bitSyncCount_attr.attr,
  NULL
};

struct attribute_group attr_group = {
  .attrs = dev_attrs
};

#endif // _GPIOWIRE_H_
