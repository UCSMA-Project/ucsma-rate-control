#include "htc.h"
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>


#define TEST_INTERVAL	2
#define MAX_TEST_COUNT 10000

struct gpio unlock_gpios[] = {
  {21, GPIOF_OUT_INIT_LOW, "UNLOCK_OUT"},
  {22, GPIOF_IN, "UNLOCK_IN" },
};

struct irq_latency_test {
  struct timespec start, end;
  struct timer_list timer;
  u32 test_count, missed_count, max_count;
  u16 results[MAX_TEST_COUNT];
  unsigned long avg_nsecs, min_nsecs, max_nsecs;
};

static u32 test_count = 100;
module_param(test_count, uint, 0);

static struct irq_latency_test test_data;

static void interrupt_test_node1_timer_handler(unsigned long ptr) {
  struct irq_latency_test* data = (struct irq_latency_test*)ptr;
  int i = 0, test_ok = 0;

  gpio_set_value(unlock_gpios[0].gpio, 1);
  getnstimeofday(&data->start);
  gpio_set_value(unlock_gpios[0].gpio, 0);

  for (i = 0; i < 1000000; i++) {
    if (gpio_get_value(unlock_gpios[1].gpio))
      break;
  }

  getnstimeofday(&data->end);

  if (i < 1000000) {
    struct timespec delta = timespec_sub(data->end, data->start);
    if (delta.tv_sec > 0) {
      printk(KERN_ERR "[Interrupt test] GPIO IRQ > 1 sec, this should never happen\n");
      data->missed_count++;
    }
    else {
      data->avg_nsecs = (data->avg_nsecs * data->test_count + delta.tv_nsec) / (data->test_count + 1);
      data->max_nsecs = (delta.tv_nsec > data->max_nsecs) ? delta.tv_nsec : data->max_nsecs;
      data->min_nsecs = (delta.tv_nsec < data->min_nsecs || !data->min_nsecs) ? delta.tv_nsec : data->min_nsecs;
      data->results[data->test_count] = delta.tv_nsec;
      test_ok = 1;
    }
  }
  else {
    data->missed_count++;
  }

  if (test_ok && ++data->test_count >= data->max_count) {
    printk(KERN_INFO "[Interrupt test] [%u", data->results[0]);
    for (i = 1; i < data->max_count; i++)
      printk(", %u", data->results[i]);
    printk("]\n");
    printk(KERN_INFO "[Interrupt test] finished %u passes. average GPIO IRQ latency is %lu nsecs, "
                      "minimum latency is %lu nsecs, maximum latency is %lu nsecs. Missed IRQ: %u\n",
          data->max_count, data->avg_nsecs, data->min_nsecs, data->max_nsecs, data->missed_count);
    return;
  }

  mod_timer(&data->timer, jiffies + TEST_INTERVAL);

  return;

}

/* Entry point of driver */
static int __init unlock_init(void)
{
  int ret;

  printk(KERN_INFO "[Interrupt test] Init\n");

  /* Get access to GPIOS for recieving and propogating unlock signal */
  if ((ret = gpio_request_array(unlock_gpios, ARRAY_SIZE(unlock_gpios)))) {
    printk(KERN_ERR "[Interrupt test] unable to get input gpio\n");
	goto fail;
  }

  init_timer(&test_data.timer);
  test_data.timer.expires = jiffies + TEST_INTERVAL;
  test_data.timer.data = (unsigned long)&test_data;
  test_data.timer.function = interrupt_test_node1_timer_handler;
  add_timer(&test_data.timer);

  if (test_count > MAX_TEST_COUNT)
    test_count = MAX_TEST_COUNT;
  test_data.max_count = test_count;
  printk(KERN_INFO "[Interrupt test] beginning GPIO IRQ latency test %u passes\n",
    test_data.max_count);

  return 0;

fail:
  gpio_free_array(unlock_gpios, ARRAY_SIZE(unlock_gpios));

  return ret;
}

/* Exit Point of driver */
static void __exit unlock_exit(void)
{
  del_timer_sync(&test_data.timer);
  gpio_free_array(unlock_gpios, ARRAY_SIZE(unlock_gpios));
  printk(KERN_INFO "[Interrupt test] Uninit\n");
  return;
}

module_init(unlock_init);
module_exit(unlock_exit);
MODULE_LICENSE("GPL");
