#include "gpiowire.h"

/************/
/* Protocol */
/************/

void print_perf_data(struct device_data* data)
{
  size_t  index;
  size_t  pulse_count    = 0;
  
  ktime_t edge_time      = ktime_set(0, 0);
  ktime_t last_edge_time = edge_time;

  for (index = 1 /* > Baseline */; index < data->prot_ctx.perf_count; index++)
  {
    edge_time = ktime_sub(
      data->prot_ctx.perf_data[index].time,
      data->prot_ctx.perf_data[index - 1].time
    );

    LOG_DEV(
      debug,
      "[PERF] Edge  [%05zu] %s %10llu ns.\n",
      index, 

      (
          (EDGE_LOW == data->prot_ctx.perf_data[index].next_edge)
        ? "/"
        : "\\"
      ),

      ktime_to_ns(edge_time)
    );

    if (
         (index > 1) 
      && (EDGE_LOW == data->prot_ctx.perf_data[index].next_edge)
    )
    {
      LOG_DEV(
        debug,
        "[PERF] Pulse (%05zu) = %10llu ns.\n",
        ++pulse_count,

        ktime_to_ns(ktime_add(edge_time, last_edge_time))
      );
    }

    last_edge_time = edge_time;
  }

  LOG_DEV(
    debug, 
    "[PERF] collected %zu edges, %zu pulses.\n", 
    data->prot_ctx.perf_count,
    pulse_count
  );
}

inline int gpio_set_pin(struct device_data* data, int value)
{
  if (data->attr_swap_output)
  {
    value = (value ? 0 : 1);
  }

  if (data->attr_can_sleep)
  {
    gpio_set_value_cansleep(data->attr_pin_number, value);
  }
  else
  {
    gpio_set_value(data->attr_pin_number, value); 
  }

  return value;
}

enum hrtimer_restart prot_write_callback(struct hrtimer *timer)
{
  struct device_data *data = container_of(
    timer, 
    struct device_data, 
    timer
  );

  ktime_t low_edge;

  // Performance data collection
  
  if (data->attr_perf_debug)
  {
    data->prot_ctx.perf_data[data->prot_ctx.perf_count].next_edge =
      data->prot_ctx.next_edge;

    data->prot_ctx.perf_data[data->prot_ctx.perf_count].time = ktime_get();
    data->prot_ctx.perf_count++;
  }

  // High edge

  if (EDGE_HIGH == data->prot_ctx.next_edge)
  {
    data->prot_ctx.next_edge = EDGE_LOW;

    gpio_set_pin(data, 1); 
    hrtimer_forward_now(timer, data->edge_high_state);

    return HRTIMER_RESTART;
  }

  // Sync bits
  
  if (data->prot_ctx.sync_count > 0) 
  {
    data->prot_ctx.sync_count--;

    // Low edge
    
    data->prot_ctx.next_edge = EDGE_HIGH;

    gpio_set_pin(data, 0);
    hrtimer_forward_now(timer, data->edge_sync_bit);  

    return HRTIMER_RESTART;
  }

  // Check for sequence completion
  
  if (
       (0 == data->prot_ctx.msg_count)
    && (0 == data->prot_ctx.bit_count)
  )
  {
    // Last edge

    gpio_set_pin(data, 0);

    // Sequence completed

    complete(&data->prot_ctx.sem);
    return HRTIMER_NORESTART;
  }

  // Encode bit
  
  low_edge =
      (128 == (*data->prot_ctx.message & 128))
    ? data->edge_one_bit
    : data->edge_zero_bit
  ;

  *data->prot_ctx.message <<= 1;

  // Get next data bits
  
  data->prot_ctx.bit_count--;

  if (0 == data->prot_ctx.bit_count)
  {
    // Byte completed
    
    data->prot_ctx.msg_count--;

    if (data->prot_ctx.msg_count > 0)
    {
      // Other bytes available...
      
      data->prot_ctx.sync_count = data->attr_sync_bit_count;
      data->prot_ctx.bit_count  = 8;
      data->prot_ctx.message++;
    }
  }

  // Low edge
    
  data->prot_ctx.next_edge = EDGE_HIGH;

  gpio_set_pin(data, 0); 
  hrtimer_forward_now(timer, low_edge);  

  return HRTIMER_RESTART;
}

inline ssize_t prot_write_message(
  struct device_data *data,
  const char         *buffer,
  size_t             len
)
{
  // Initialization
  
  size_t  edge_count    = 0;
  ktime_t tot_perf_time = ktime_set(0, 0);
  
  if (!len)
  {
    LOG_DEV(warn, "null buffer provided.\n");
    return -EINVAL;
  }
  
  if (data->attr_perf_debug)
  {
    // Allocate memory for performance data collection
    
    edge_count = (
        1 /* Baseline */
      + 2 /* Sequence setup pulse */
      + ((data->attr_sync_bit_count + 8) * len * 2)
    );
    
    data->prot_ctx.perf_data = kzalloc(
      (sizeof(struct perf_data) * edge_count), 
      GFP_KERNEL
    );

    if (!data->prot_ctx.perf_data)
    {
      LOG_DEV(crit, "[PERF] failed to allocate memory.\n");
      return -ENOMEM;    
    } 
  }

  data->prot_ctx.sync_count = data->attr_sync_bit_count;
  data->prot_ctx.message    = (char *)buffer;
  data->prot_ctx.msg_count  = len;
  data->prot_ctx.bit_count  = 8;

  reinit_completion(&data->prot_ctx.sem);

  // Setup line up for 1st bit transition
  
  data->prot_ctx.next_edge = EDGE_HIGH;
  gpio_set_pin(data, 0);

  if (data->attr_perf_debug)
  {
    // Init performance data collection
    
    LOG_DEV(debug, "[PERF] collecting performance data...\n");
    
    tot_perf_time = ktime_get();

    // Store baseline
    
    data->prot_ctx.perf_data[0].time = ktime_get();
    data->prot_ctx.perf_count        = 1;
  }

  // Wait for sequence completion...

  hrtimer_start(&data->timer, data->edge_high_state, HRTIMER_MODE_REL);
  wait_for_completion_killable(&data->prot_ctx.sem);

  if (data->attr_perf_debug)
  {
    // Finish performance data collection
    
    LOG_DEV(
      debug,
      "[PERF] performance data collection completed in %llu ns.\n",
      ktime_to_ns(ktime_sub(ktime_get(), tot_perf_time))
    );

    print_perf_data(data);
    kfree(data->prot_ctx.perf_data);
  }

  return len;
}

/*****************/
/* Device Driver */
/*****************/

int gpiowire_register_device(int number)
{
  struct device      *dev;
  struct device_data *data;
  int                result;

  // Register device
  
  char deviceName[256];
  sprintf(deviceName, _DEVICE_NAME, number);
  
  dev = device_create(
    dev_class, 
    NULL, 
    MKDEV(major_mumber, number), 
    NULL, 
    deviceName
  );
  
  if (IS_ERR(dev))
  {
    LOG(crit, "failed to create device %d.\n", number);
    return PTR_ERR(dev);
  }
  
  LOG(debug, "device %d successfully created.\n", number);

  // Allocate device data
  
  data = kzalloc(sizeof(struct device_data), GFP_KERNEL);

  if (!data)
  {
    LOG(crit, "failed to allocate private memory for device %d.\n", number);
    return -ENOMEM;    
  }

  // Initialize device data

  dev_list[dev_count] = dev;

  *data               = def_dev_data;
  data->dev_number    = dev_count;

  mutex_init(&data->mutex);
  init_completion(&data->prot_ctx.sem);
  hrtimer_init(&data->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

  data->timer.function = &prot_write_callback;
  dev_set_drvdata(dev, data);

  // Create sysfs group
  
  data->kobj = kobject_create_and_add("settings", &dev->kobj);

  if (!data->kobj)
  {
    LOG_DEV(crit, "failed to create sysfs main entry.\n");
    return -ENOMEM;
  }

  result = sysfs_create_group(data->kobj, &attr_group);
  
  if (result)
  {
    LOG_DEV(crit, "failed to create sysfs group.\n");
    return result;
  }
  
  LOG_DEV(debug, "sysfs group successfully created.\n");
  return 0;
}

void gpiowire_unregister_devices(void)
{
  struct device      *dev; 
  struct device_data *data;
  int                index;

  for (index = 0; index < dev_count; index++)
  {
    dev  = dev_list[index];
    data = dev_get_drvdata(dev);

    if (data)
    {
      if (data->kobj)
      {
        kobject_put(data->kobj);
      }

      hrtimer_cancel(&data->timer);
      mutex_destroy(&data->mutex);
      kfree(data);
    }

    device_destroy(dev_class, MKDEV(major_mumber, index));
  }

  dev_count = 0;
  kfree(dev_list);

  class_unregister(dev_class);
  class_destroy(dev_class);
  unregister_chrdev(major_mumber, _CLASS_NAME);

  LOG(info, "devices successfully unregistered.\n");
}

static int __init gpiowire_init(void)
{
  int index, result;

  LOG(info, "loading module...\n");

  // Check some parameters 
  
  if (devicesNumber < 1)
  {
    LOG(crit, "invalid devices number.\n");
    return -EINVAL;
  }

  // Allocate major device number
   
  major_mumber = register_chrdev(0, _CLASS_NAME, &dev_file_ops);
   
  if (major_mumber < 0)
  {
    LOG(crit, "failed to register a major number.\n");
    return major_mumber;
  }
   
  LOG(debug, "successfully registered with major number %d.\n", major_mumber);
  
  // Register device class
  
  dev_class = class_create(THIS_MODULE, _CLASS_NAME);
   
  if (IS_ERR(dev_class))
  {
    unregister_chrdev(major_mumber, _CLASS_NAME); 

    LOG(crit, "failed to register device class.\n");
    return PTR_ERR(dev_class);
  }
  
  LOG(debug, "device class successfully registered.\n");

  // Register devices
  
  dev_list = kzalloc(sizeof(void*), GFP_KERNEL);

  if (!dev_list)
  {
    LOG(crit, "unable to allocate memory for devices.\n");
    return -ENOMEM;
  }
   
  dev_count = 0;
  
  for (index = 0; index < devicesNumber; index++)
  {
    result = gpiowire_register_device(index);

    if (result < 0)
    {
      gpiowire_unregister_devices();
      return result;
    }

    dev_count++;
  }

  LOG(info, "module successfully loaded.\n");
  return 0;
}

static void __exit gpiowire_exit(void)
{
  gpiowire_unregister_devices();
  
  LOG(info, "module unloaded.\n");
}

bool file_trylock(struct device_data* data)
{
  if (!mutex_trylock(&data->mutex))
  {
    LOG_DEV(crit, "device is in use by another process.\n");
    return false;
  }

  return true;
}

int file_open(struct inode *inodep, struct file *filep)
{
  char                pinLabel[64];
  int                 result;

  struct device_data* data = 
    (struct device_data*)(dev_list[MINOR(inodep->i_rdev)]->driver_data);

  filep->private_data = data;

  if (!file_trylock(data))
  {
    return -EBUSY;
  }

  sprintf(pinLabel, "pin %ud", data->attr_pin_number);

  result = gpio_request(data->attr_pin_number, pinLabel); 

  if (result < 0)
  {
    mutex_unlock(&data->mutex);

    LOG_DEV(crit, "gpio pin %d not available.\n", data->attr_pin_number);
    return result;
  }
   
  LOG_DEV(debug, "gpio pin %d successfully reserved.\n", data->attr_pin_number);

  result = gpio_export(data->attr_pin_number, false); 

  if (result < 0)
  {
    mutex_unlock(&data->mutex);
    
    LOG_DEV(crit, "unable to export gpio pin %d.\n", data->attr_pin_number);
    return result;
  }
   
  LOG_DEV(debug, "gpio pin %d successfully exported.\n", data->attr_pin_number);

  result = gpio_direction_output(data->attr_pin_number, data->attr_swap_output);

  if (result < 0)
  {
    mutex_unlock(&data->mutex);
    
    LOG_DEV(
      crit, 
      "unable to set gpio pin %d direction.\n", 
      data->attr_pin_number
    );

    return result;
  }

  LOG_DEV(
    debug, 
    "gpio pin %d direction successfully set.\n", 
    data->attr_pin_number
  );

  data->edge_high_state = ktime_set(0, (data->attr_high_state  * 1000));

  data->edge_zero_bit = 
    ktime_set(0, ((data->attr_zero_bit - data->attr_high_state) * 1000));

  data->edge_one_bit = 
    ktime_set(0, ((data->attr_one_bit  - data->attr_high_state) * 1000));

  data->edge_sync_bit = 
    ktime_set(0, ((data->attr_sync_bit - data->attr_high_state) * 1000));

  LOG_DEV(debug, "successfully opened.\n");
  return 0;
}

int file_release(struct inode *inodep, struct file *filep)
{
  struct device_data* data = (struct device_data*)filep->private_data;

  if (data->attr_pin_number > 0)
  {
    gpio_unexport(data->attr_pin_number);
    gpio_free(data->attr_pin_number);
  }

  mutex_unlock(&data->mutex);

  LOG_DEV(debug, "successfully released.\n");
  return 0;
}

ssize_t file_write(
  struct file       *filep, 
  const char __user *buffer, 
  size_t            len, 
  loff_t            *offset
)
{
  struct device_data* data    = (struct device_data*)filep->private_data;
  char*               message = kmalloc(len, GFP_KERNEL);
    
  if (!message)
  {
    LOG_DEV(crit, "cannot allocate buffer (%zu bytes).\n", len);
    return  -EFAULT;
  }

  if (copy_from_user(message, buffer, len))
  {
    kfree(message);

    LOG_DEV(crit, "cannot copy buffer (%zu bytes).\n", len);
    return -EFAULT;    
  }

  LOG_DEV(
    debug, 
    "writing %zu byte(s) to pin %d...\n", 
    len,
    data->attr_pin_number
  );

  prot_write_message(data, message, len);    
  kfree(message);

  LOG_DEV(debug, "buffer successfully written.\n");
  
  *offset += len;
  return len;
}

// Module entry/exit point

module_init(gpiowire_init);
module_exit(gpiowire_exit); 

/**************/
/* Attributes */
/**************/

struct device_data* kobj_to_dev_data(struct kobject *kobj)
{
  struct device *dev = kobj_to_dev(kobj->parent);

  return (struct device_data*)dev_get_drvdata(dev);
}

ssize_t perfDebug_show(
  struct kobject        *kobj, 
  struct kobj_attribute *attr, 
  char                  *buf
)
{
  struct device_data* data = kobj_to_dev_data(kobj);
  
  return sprintf(
    buf, 
    "%d\n", 
    (data->attr_perf_debug ? 1 : 0)
  );
}

ssize_t perfDebug_store(
  struct kobject        *kobj, 
  struct kobj_attribute *attr, 
  const char            *buf, 
  size_t                count)
{
  int                 value;
  struct device_data* data = kobj_to_dev_data(kobj);

  sscanf(buf, "%du", &value);

  data->attr_perf_debug = (1 == value);

  LOG_DEV(
    debug, 
    "performance debugging set to %s.\n", 
    (data->attr_perf_debug ? "true" : "false")
  );

  return count;
}

ssize_t pinNumber_show(
  struct kobject        *kobj, 
  struct kobj_attribute *attr, 
  char                  *buf
)
{
  struct device_data* data = kobj_to_dev_data(kobj);

  return sprintf(buf, "%ud\n", data->attr_pin_number);
}

ssize_t pinNumber_store(
  struct kobject        *kobj, 
  struct kobj_attribute *attr, 
  const char            *buf, 
  size_t                count
)
{
  int                 result;
  int                 value;
  struct device_data* data = kobj_to_dev_data(kobj);

  if (!file_trylock(data))
  {
    return -EBUSY;
  }

  sscanf(buf, "%du", &value);

  result = gpio_is_valid(value);

  if (result < 0)
  {
    mutex_unlock(&data->mutex);

    LOG_DEV(crit, "gpio pin %d not valid.\n", value);
    return result;
  }

  data->attr_pin_number = value;

  mutex_unlock(&data->mutex);

  LOG_DEV(debug, "gpio pin set to %d.\n", data->attr_pin_number);
  return count;
}

ssize_t highStateEdge_show(
  struct kobject        *kobj, 
  struct kobj_attribute *attr, 
  char                  *buf
)
{
  struct device_data* data = kobj_to_dev_data(kobj);

  return sprintf(buf, "%lu\n", data->attr_high_state);
}

ssize_t highStateEdge_store(
  struct kobject        *kobj, 
  struct kobj_attribute *attr, 
  const char            *buf, 
  size_t                count
)
{
  unsigned long         value;
  struct device_data* data = kobj_to_dev_data(kobj);

  sscanf(buf, "%luu", &value);

  if (0 == value)
  {
      LOG_DEV(err, "high state duration must be greater than zero.\n");    
      return -EINVAL;
  }

  data->attr_high_state = value;

  LOG_DEV(
    debug, 
    "high state edge set to %lu uS.\n",
    data->attr_high_state
  );

  return count;
}

ssize_t bitZeroDuration_show(
  struct kobject        *kobj, 
  struct kobj_attribute *attr, 
  char                  *buf
)
{
  struct device_data* data = kobj_to_dev_data(kobj);

  return sprintf(buf, "%lu\n", data->attr_zero_bit);
}

ssize_t bitZeroDuration_store(
  struct kobject        *kobj, 
  struct kobj_attribute *attr, 
  const char            *buf, 
  size_t                count
)
{
  unsigned long       value;
  struct device_data* data = kobj_to_dev_data(kobj);

  sscanf(buf, "%luu", &value);

  if (0 == value)
  {
    LOG_DEV(err, "bit zero duration must be greater than zero.\n");    
    return -EINVAL;
  }

  if (value <= data->attr_high_state)
  {
    LOG_DEV(warn, "bit zero duration should be greater than high state.\n");
  }

  data->attr_zero_bit = value;

  LOG_DEV(debug, "bit zero duration set to %lu uS.\n", data->attr_zero_bit);
  return count;
}

ssize_t bitOneDuration_show(
  struct kobject        *kobj, 
  struct kobj_attribute *attr, 
  char                  *buf
)
{
  struct device_data* data = kobj_to_dev_data(kobj);

  return sprintf(buf, "%lu\n", data->attr_one_bit);
}

ssize_t bitOneDuration_store(
  struct kobject        *kobj, 
  struct kobj_attribute *attr, 
  const char            *buf, 
  size_t                count
)
{
  unsigned long       value;
  struct device_data* data = kobj_to_dev_data(kobj);

  sscanf(buf, "%luu", &value);

  if (0 == value)
  {
    LOG_DEV(err, "bit one duration must be greater than zero.\n");    
    return -EINVAL;
  }

  if (value <= data->attr_zero_bit)
  {
    LOG_DEV(warn, "bit one duration should be greater than bit zero.\n");
  }

  data->attr_one_bit = value;

  LOG_DEV(debug, "bit one duration set to %lu uS.\n", data->attr_one_bit);
  return count;
}

ssize_t bitSyncCount_show(
  struct kobject        *kobj, 
  struct kobj_attribute *attr, 
  char                  *buf
)
{
  struct device_data* data = kobj_to_dev_data(kobj);

  return sprintf(buf, "%d\n", data->attr_sync_bit_count);
}

ssize_t bitSyncCount_store(
  struct kobject        *kobj, 
  struct kobj_attribute *attr, 
  const char            *buf, 
  size_t                count
)
{
  unsigned int        value;
  struct device_data* data = kobj_to_dev_data(kobj);

  sscanf(buf, "%udu", &value);

  if (0 == value)
  {
    LOG_DEV(err, "sync bit count must be greater than zero.\n");
    return -EINVAL;
  }

  data->attr_sync_bit_count = value;

  LOG_DEV(debug, "sync bit count set to %d times.\n", data->attr_sync_bit_count);
  return count;
}

ssize_t bitSyncDuration_show(
  struct kobject        *kobj, 
  struct kobj_attribute *attr, 
  char                  *buf
)
{
  struct device_data* data = kobj_to_dev_data(kobj);

  return sprintf(buf, "%lu\n", data->attr_sync_bit);
}

ssize_t bitSyncDuration_store(
  struct kobject        *kobj, 
  struct kobj_attribute *attr, 
  const char            *buf, 
  size_t                count
)
{
  unsigned long       value;
  struct device_data* data = kobj_to_dev_data(kobj);

  sscanf(buf, "%luu", &value);

  if (0 == value)
  {
    LOG_DEV(err, "sync bit duration must be greater than zero.\n");    
    return -EINVAL;
  }

  if (value <= data->attr_one_bit)
  {
    LOG_DEV(warn, "sync bit duration should be greater than bit one.\n");
  }

  data->attr_sync_bit = value;

  LOG_DEV(debug, "sync bit duration set to %lu uS.\n", data->attr_sync_bit);
  return count;
}

ssize_t swapOutput_show(
  struct kobject        *kobj, 
  struct kobj_attribute *attr, 
  char                  *buf
)
{
  struct device_data* data = kobj_to_dev_data(kobj);
  
  return sprintf(
    buf, 
    "%d\n", 
    (data->attr_swap_output ? 1 : 0)
  );
}

ssize_t swapOutput_store(
  struct kobject        *kobj, 
  struct kobj_attribute *attr, 
  const char            *buf, 
  size_t                count)
{
  int                 value;
  struct device_data* data = kobj_to_dev_data(kobj);

  sscanf(buf, "%du", &value);

  data->attr_swap_output = (1 == value);

  LOG_DEV(
    debug, 
    "swap output set to %s.\n", 
    (data->attr_swap_output ? "true" : "false")
  );

  return count;
}

ssize_t canSleep_show(
  struct kobject        *kobj, 
  struct kobj_attribute *attr, 
  char                  *buf
)
{
  struct device_data* data = kobj_to_dev_data(kobj);
  
  return sprintf(
    buf, 
    "%d\n", 
    (data->attr_can_sleep ? 1 : 0)
  );
}

ssize_t canSleep_store(
  struct kobject        *kobj, 
  struct kobj_attribute *attr, 
  const char            *buf, 
  size_t                count)
{
  int                 value;
  struct device_data* data = kobj_to_dev_data(kobj);

  sscanf(buf, "%du", &value);

  data->attr_can_sleep = (1 == value);

  LOG_DEV(
    debug, 
    "can sleep set to %s.\n", 
    (data->attr_can_sleep ? "true" : "false")
  );

  return count;
}
