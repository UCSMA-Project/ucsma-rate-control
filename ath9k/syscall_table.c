// #include <linux/module.h>
// #include <linux/kernel.h>
// #include <linux/syscalls.h>
// #include <linux/kernel.h>
// #include <linux/init.h>
#include <linux/module.h>
// #include <asm/current.h>
// #include <asm/ptrace.h>
// #include <linux/sched.h>
// #include <linux/cred.h>
// #include <asm/unistd.h>
// #include <linux/syscalls.h>

// extern void* sys_call_table[];

static int __init init_function(void) {
    // pr_info("SYSCALL_TBALE: %d\n", FAKE_SYSCALL_NUM);
    return 0;
}

static int __exit exit_function(void) {
    return 0;
}

module_init(init_function);
module_exit(exit_function);