#include "htc.h"
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/hrtimer.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>

#define RECORD_COUNT 1000

struct gpio unlock_gpios[] = {
  {21, GPIOF_OUT_INIT_LOW, "UNLOCK_OUT"},
  {22, GPIOF_IN, "UNLOCK_IN" },
};

struct gpio_log {
  struct timespec time;
  int triggered_by_timer;
};

struct gpio_log log[RECORD_COUNT];
u32 cur_count = 0;

/* 
 * Status struct from regular ath9k driver
 * Holds onto a large part of the state of the driver
 * and exposes the hooks required to read and write
 * registers
 */
extern struct ath_hw *ath9k_ah;

// backoff param Delta, define how long to sleep (in us) before device can start to compete for CSMA again
static u32 Delta = 100;
module_param(Delta, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

// period param T, define how often (in us) to send unlocking signal
static u32 T = 20000;	// initial value 20ms
module_param(T, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

/* FD that we use to keep track of incoming unlock signals */
short int unlock_irq = 0;

/* 
 * Holds onto any pending interrupts while we handle the
 * the incoming unlock interrupt
 */
unsigned long flags;

// last time of unlocking and 2nd last time of unlocking

struct timespec last_unlock, last_unlock_2nd;

// Timer used to periodically send out unlocking signal
struct hrtimer unlock_timer;

/* 
 * Spin Lock used to guarantee that a single unlock
 * interrupt is handled at a time 
 */
DEFINE_SPINLOCK(driver_lock);

static void print_log(void) {
  int i;
  for (i = 0; i < RECORD_COUNT; i++)
    if (log[i].triggered_by_timer)
      printk("Timer: %lu.%09lu\n", log[i].time.tv_sec,  log[i].time.tv_nsec);
    else
      printk("IRQ  : %lu.%09lu\n", log[i].time.tv_sec,  log[i].time.tv_nsec);
}

static enum hrtimer_restart unlock_timer_handler(struct hrtimer *timer) {
  if (cur_count < RECORD_COUNT) {
    getnstimeofday(&(log[cur_count].time));
    log[cur_count].triggered_by_timer = 1;
    cur_count++;
    gpio_set_value(unlock_gpios[0].gpio, 1);
    udelay(1);
    gpio_set_value(unlock_gpios[0].gpio, 0);
    if (cur_count == RECORD_COUNT)
      print_log();
  }

  hrtimer_forward_now(timer, ktime_set(0, T * 1000));

  return HRTIMER_NORESTART;
}

/* Interrupt handler called on falling edfe of UNLOCK_IN GPIO */
static irqreturn_t unlock_r_irq_handler(int irq, void *dev_id) {
  struct timespec now, diff;
  unsigned int next_timer, backoff, rng;
  spin_lock_irqsave(&driver_lock, flags);
  getnstimeofday(&now);
  diff = timespec_sub(now, last_unlock);
  if (diff.tv_sec || diff.tv_nsec < T * 500) {
    spin_unlock_irqrestore(&driver_lock, flags);
    return IRQ_HANDLED;
  }
  // printk(KERN_INFO "irq called\n");

  hrtimer_cancel(&unlock_timer);	//stop timer from being fire up

  if (cur_count < RECORD_COUNT) {
    getnstimeofday(&(log[cur_count].time));
    log[cur_count].triggered_by_timer = 0;
    cur_count++;
    gpio_set_value(unlock_gpios[0].gpio, 1);	//relay the unlocking signal
    udelay(1);
    gpio_set_value(unlock_gpios[0].gpio, 0);
    if (cur_count == RECORD_COUNT)
      print_log();
  }

  get_random_bytes(&rng, sizeof(rng));
  rng %= (Delta + Delta);
  diff = timespec_sub(last_unlock, last_unlock_2nd);
  if (diff.tv_sec || diff.tv_nsec <= T * 1000)
    next_timer = T + rng;
  else
    next_timer = T - rng;

  last_unlock_2nd.tv_sec = last_unlock.tv_sec;
  last_unlock_2nd.tv_nsec = last_unlock.tv_nsec;
  last_unlock.tv_sec = now.tv_sec;
  last_unlock.tv_nsec = now.tv_nsec;

  get_random_bytes(&backoff, sizeof(backoff));
  backoff %= (Delta + Delta);

  REG_SET_BIT(ath9k_ah, AR_PCU_MISC, AR_PCU_FORCE_QUIET_COLL);
  ndelay(backoff * 1000);
  REG_CLR_BIT(ath9k_ah, AR_PCU_MISC, AR_PCU_FORCE_QUIET_COLL);

  next_timer -= backoff;
  if (next_timer < 0)
    next_timer = 0;

  hrtimer_start(&unlock_timer, ktime_set(0, next_timer * 1000), HRTIMER_MODE_REL);

  spin_unlock_irqrestore(&driver_lock, flags);

  return IRQ_HANDLED;
}

/* Entry point of driver */
static int __init unlock_init(void)
{
  int ret;

  printk(KERN_INFO "U-CSMA - unlock inserted");

  /* Initialize CTS poll timer */
  hrtimer_init(&unlock_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
  unlock_timer.function = unlock_timer_handler;

  getnstimeofday(&last_unlock);

  /* Get access to GPIOS for recieving and propogating unlock signal */
  if ((ret = gpio_request_array(unlock_gpios, ARRAY_SIZE(unlock_gpios)))) {
    printk(KERN_ERR "U-CSMA - unable to get input gpio\n");
    return ret;
  }
  /* Get interrupt number for UNLOCK_IN GPIO */
  else if ((ret = unlock_irq = gpio_to_irq(unlock_gpios[1].gpio)) < 0) {
    printk(KERN_ERR "U-CSMA - IRQ mapping failure\n");
    goto fail;
  }
  /* Initialize interrupt on UNLOCK_IN GPIO to call unlock_r_irq_handler */
  else if ((ret = request_irq(unlock_irq,
                (irq_handler_t) unlock_r_irq_handler,
                IRQF_TRIGGER_FALLING,
                "U-CSMA",
                "felipe device"))) {
    printk(KERN_ERR "U-CSMA - unable to get IRQ\n");
    goto fail;
  }

  hrtimer_start(&unlock_timer, ktime_set(0, T * 1000), HRTIMER_MODE_REL);

  printk(KERN_INFO "U-CSMA INIT complete\n");

  return 0;

fail:
  gpio_free_array(unlock_gpios, ARRAY_SIZE(unlock_gpios));

  return ret;
}

/* Exit Point of driver */
static void __exit unlock_exit(void)
{
  hrtimer_cancel(&unlock_timer);
  free_irq(unlock_irq, "felipe device");

  gpio_free_array(unlock_gpios, ARRAY_SIZE(unlock_gpios));

  printk(KERN_INFO "U-CSMA - unlock module unloaded\n");
  return;
}

module_init(unlock_init);
module_exit(unlock_exit);
MODULE_LICENSE("GPL");
