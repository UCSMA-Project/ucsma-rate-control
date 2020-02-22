#include "htc.h"
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>

#define RECORD_COUNT 1000

struct gpio unlock_gpios[] = {
  {21, GPIOF_OUT_INIT_LOW, "UNLOCK_OUT"},
  {22, GPIOF_IN, "UNLOCK_IN" },
};

struct timespec records[RECORD_COUNT];
u32 cur_count = 0;

/* FD that we use to keep track of incoming unlock signals */
short int unlock_irq = 0;

/* 
 * Holds onto any pending interrupts while we handle the
 * the incoming unlock interrupt
 */
unsigned long flags;

/* 
 * Spin Lock used to guarantee that a single unlock
 * interrupt is handled at a time 
 */
DEFINE_SPINLOCK(driver_lock);

/* Interrupt handler called on falling edfe of UNLOCK_IN GPIO */
static irqreturn_t unlock_r_irq_handler(int irq, void *dev_id) {
  int i;

  spin_lock_irqsave(&driver_lock, flags);

  if (cur_count < RECORD_COUNT) {
    getnstimeofday(&records[cur_count++]);
    if (cur_count == RECORD_COUNT)
      for (i = 0; i < RECORD_COUNT; i++)
        printk("%lu.%09lu\n", records[i].tv_sec,  records[i].tv_nsec);
  }
  
  spin_unlock_irqrestore(&driver_lock, flags);

  return IRQ_HANDLED;
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
  /* Get interrupt number for UNLOCK_IN GPIO */
  else if ((ret = unlock_irq = gpio_to_irq(unlock_gpios[1].gpio)) < 0) {
    printk(KERN_ERR "[Interrupt test] IRQ mapping failure\n");
    goto fail;
  }
  /* Initialize interrupt on UNLOCK_IN GPIO to call unlock_r_irq_handler */
  else if ((ret = request_any_context_irq(unlock_irq,
                (irq_handler_t) unlock_r_irq_handler,
                IRQF_TRIGGER_FALLING,
                "GPIO Interrupt",
                "Interrupt Delay Test"))) {
    printk(KERN_ERR "[Interrupt test] unable to get IRQ\n");
    goto fail;
  }
  
  return 0;

fail:
  gpio_free_array(unlock_gpios, ARRAY_SIZE(unlock_gpios));

  return ret;
}

/* Exit Point of driver */
static void __exit unlock_exit(void)
{
  free_irq(unlock_irq, "Interrupt Delay Test");

  gpio_free_array(unlock_gpios, ARRAY_SIZE(unlock_gpios));

  printk(KERN_INFO "[Interrupt test] Uninit\n");
  return;
}

module_init(unlock_init);
module_exit(unlock_exit);
MODULE_LICENSE("GPL");
