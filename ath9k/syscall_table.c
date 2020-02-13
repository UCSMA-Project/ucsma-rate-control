#include <linux/module.h>
#include <linux/kernel.h>
#include <sys/syscall.h>

extern void* sys_call_table[];

static void init_function(void) {
    pr_info("SYSCALL_TBALE: %d\n", NR_syscalls);
}

static void exit_function(void) {
    return;
}

module_init(init_function);
module_exit(exit_function);