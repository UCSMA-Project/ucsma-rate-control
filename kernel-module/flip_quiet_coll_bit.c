#include "htc.h"
#include <linux/timer.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>

/* 
 * Status struct from regular ath9k driver
 * Holds onto a large part of the state of the driver
 * and exposes the hooks required to read and write
 * registers
 */
extern struct ath_hw *ath9k_ah;

// sleep delay time parameter, defines how long (in us) to sleep after setting FORCE_QUIET_BIT
// release duration (in us) between clearing the FORCE_QUIET_COLL bit and setting it again
static u32 release_duration = 0;
module_param(release_duration, uint, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

/* 
 * Timer used to periodically release FORCE_QUIET_COLL bit for a short time
 */
struct timer_list release_timer;

// function that release FORCE_QUIET_COLL bit for a short time
void release_force_quiet_coll(unsigned long data) {
	do_gettimeofday(&ath9k_ah->last_interrupt_tx);
	REG_CLR_BIT(ath9k_ah, AR_PCU_MISC, AR_PCU_FORCE_QUIET_COLL);
	udelay(release_duration);
	REG_SET_BIT(ath9k_ah, AR_PCU_MISC, AR_PCU_FORCE_QUIET_COLL);
	mod_timer(&release_timer, jiffies + 1);
	return;
}

/* Entry point of driver */
static int __init release_init(void)
{
	int ret;

	printk(KERN_INFO "INIT releasing FORCE_QUIET_COLL bit periodically\n");

	setup_timer(&release_timer, release_force_quiet_coll, (unsigned long) ath9k_ah);

	/* Set first call of release timer */
	if ((ret = mod_timer(&release_timer, jiffies + 1))) {
		printk(KERN_ERR "releasing FORCE_QUIET_COLL bit - timer error\n");
		goto fail;
	}

	printk(KERN_INFO "releasing FORCE_QUIET_COLL bit INIT complete\n");

	return 0;

fail:
	return ret;
}

/* Exit Point of driver */
static void __exit release_exit(void)
{
  del_timer(&release_timer);

  printk(KERN_INFO "releasing FORCE_QUIET_COLL bit unloaded\n");
  return;
}

module_init(release_init);
module_exit(release_exit);
MODULE_LICENSE("GPL");
