#include "htc.h"
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>

/*
 * import the status struct from regular ath9k driver
 */

extern struct ath_hw *ath9k_ah;

/*
 * define the mask to set the backoff persistence factor low
 */
#define CW_CONSTANT 0xffffefff 

/*
 * define the parameter deciding whether to turn the bit on
 * or low
 */
static int equalizeCW = 1; //default to set the bit high
module_param(equalizeCW, int, 0664); // user can change it without sudo too
MODULE_PARM_DESC(toggleBit, "Parameter for equaling contention window max and min values (value 1) or back to their default pair of values (value 0)");

// declare the timers
struct timer_list cw_timer;

// create the work struct for toggling the bit up and down; also printing registers 
static struct work_struct edit_cw;

// declare the workqueue
static struct workqueue_struct *cw_queue;


/*
 * create the function for polling the register to verify the changes
 * to the contention window max/min value and backoff persistence factor
 */
static void edit_contentionWindow(struct work_struct *work)
{
  // int ret;
  u32 val, set, qnum;

   // use loop to unify operations on all 10 DCU units
  for (qnum = 0; qnum < 8; qnum = qnum+1) {
        // set CW_max to original min value, so it's easier to observe 
        // upperbound
        set = 0x002fffff;     
        //REG_WRITE(ath9k_ah, AR_DLCL_IFS(qnum), set);
	    val = REG_READ(ath9k_ah, AR_DLCL_IFS(qnum));
        printk(KERN_INFO "===William CW: reg value D_LCL_IFS for DCU%d after writing: %x===\n", qnum, val);
   
        /*
		 * enforce New CW equals to old CW through 
		 * field of backoff persistence factor setting
		 */
        val = REG_READ(ath9k_ah, AR_DMISC(qnum));
		set = val & CW_CONSTANT;
		//REG_WRITE(ath9k_ah, AR_DMISC(qnum), set);
		// read again to ensure the correct value set
        val = REG_READ(ath9k_ah, AR_DMISC(qnum));
        printk(KERN_INFO "===William CW: reg value D_MISC for DCU%d after writing: %x===\n", qnum, val);		
   }

  // update when timer expires
  if(mod_timer(&cw_timer, jiffies+usecs_to_jiffies(5000000))) {
    // timer set error
    printk(KERN_ERR "William ERROR: stop timer continuation error at stop_transmission work\n");
    return;
  }
  return;
}

/*
 * deal with action when stop timer expires
 */
static void cw_timer_handler(unsigned long data)
{
  //put a stop transmission work into the work queue
  queue_work(cw_queue, &edit_cw);
  return;
}
/*
 * initialize function for this test module
 */
static int __init test_init(void)
{
  u32 val, qnum, cwmin, cwmax, aifs;
  /* int ret;
  // initialize a work queue for toggling bit for stop transmission
  cw_queue = create_workqueue("William_CW");
  // initialize the work function within this queue
  INIT_WORK(&edit_cw, edit_contentionWindow);
  // initialize the timer for periodically turnning on and off
  setup_timer(&cw_timer, cw_timer_handler, (unsigned long) ath9k_ah);
  // set up when timer expires
  if (mod_timer(&cw_timer, jiffies+usecs_to_jiffies(5000000))) {
    // timer set error
    printk(KERN_ERR "William Error: stop timer error at initialization \n");
    return ret;
  } */

  // finished initialization
  printk(KERN_INFO "===========William CW: Module Starting! =========\n");
  REG_RMW_FIELD(ath9k_ah, AR_DLCL_IFS(1), AR_D_LCL_IFS_CWMIN, 0);
  REG_RMW_FIELD(ath9k_ah, AR_DLCL_IFS(1), AR_D_LCL_IFS_CWMAX, 0);
  REG_RMW_FIELD(ath9k_ah, AR_DLCL_IFS(1), AR_D_LCL_IFS_AIFS, 0);
  for (qnum = 0; qnum < 10; qnum++) {
    val = REG_READ(ath9k_ah, AR_DLCL_IFS(qnum));
	cwmin = REG_READ_FIELD(ath9k_ah, AR_DLCL_IFS(qnum), AR_D_LCL_IFS_CWMIN);
	cwmax = REG_READ_FIELD(ath9k_ah, AR_DLCL_IFS(qnum), AR_D_LCL_IFS_CWMAX);
	aifs = REG_READ_FIELD(ath9k_ah, AR_DLCL_IFS(qnum), AR_D_LCL_IFS_AIFS);
    printk(KERN_INFO "===William CW: reg value D_LCL_IFS for DCU%d: %x===\n", qnum, val);
	printk(KERN_INFO "cwmin: %d, cwmax: %d, aifs: %d\n", cwmin, cwmax, aifs);
  }
  
  return 0;
}
/*
 * exit function to clean up resources
 */
static void __exit test_exit(void)
{
  /* printk(KERN_INFO "===========William CW: About to exit, start cleaning up...=============\n");
  // delete the timers
  del_timer(&cw_timer);

  // clean up the workqueue
  flush_workqueue(cw_queue);
  destroy_workqueue(cw_queue); */

  printk(KERN_INFO "===========William CW: Module Exiting! =========\n");
  // finished cleanning up
  return;
}

/*
 * common MACROS for the module
 */
module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");
