/*******************************************************************************
 *																									  *
 * Linux GPIO IRQ latency test																 *
 *																									  *
 * taken from https://github.com/gkaindl/linux-gpio-irq-latency-test			  *
 *																									  *
 ******************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/time.h>
#include <linux/delay.h>

unsigned int delay = 5;
module_param(delay, uint, 0);
MODULE_PARM_DESC(delay, "us to sleep between measurement");

int __init kernel_timing_test_init_module(void) {
	int i;
	struct timeval st, en, diff;
	struct timespec s, e, dif;

	printk(KERN_INFO "init timing function test.\n");

	do_gettimeofday(&st);
	udelay(delay);
	do_gettimeofday(&en);

	diff.tv_sec = en.tv_sec - st.tv_sec;
	if (en.tv_usec < st.tv_usec) {
		diff.tv_usec = 1000000 + en.tv_usec - st.tv_usec;
		diff.tv_sec--;
	}
	else
		diff.tv_usec = en.tv_usec - st.tv_usec;

	getnstimeofday(&s);
	udelay(delay);
	getnstimeofday(&e);

	dif = timespec_sub(e, s);

	printk("do_gettimeofday: slept %u us, measured diff: %lu.%06lu s\n", delay, diff.tv_sec, diff.tv_usec);
	printk("getnstimeofday: slept %u us, measured diff: %lu.%09lu s\n", delay, dif.tv_sec, dif.tv_nsec);

	do_gettimeofday(&st);
	for (i = 0; i < 10000000; i++)
		do_gettimeofday(&en);

	diff.tv_sec = en.tv_sec - st.tv_sec;
	if (en.tv_usec < st.tv_usec) {
		diff.tv_usec = 1000000 + en.tv_usec - st.tv_usec;
		diff.tv_sec--;
	}
	else
		diff.tv_usec = en.tv_usec - st.tv_usec;

	getnstimeofday(&s);
	for (i = 0; i < 10000000; i++)
		getnstimeofday(&e);

	dif = timespec_sub(e, s);

	printk("do_gettimeofday: %d times in %lu.%06lu s\n", i, diff.tv_sec, diff.tv_usec);
	printk("getnstimeofday: %d times in %lu.%09lu s\n", i, dif.tv_sec, dif.tv_nsec);

	return 0;
}

void __exit test_irq_latency_exit_module(void) {
	printk(KERN_INFO "unloaded timing function test.\n");
}

module_init(kernel_timing_test_init_module);
module_exit(test_irq_latency_exit_module);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("kernel module to test various timing functions");
