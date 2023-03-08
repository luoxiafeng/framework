#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/preempt.h>

static struct task_struct *task;

static int my_thread_func(void *data) {
    int i;
    preempt_disable();
    for (i = 0; i < 1000000; i++) {
        printk(KERN_INFO "Counting: %d\\n", i);
        udelay(10);
    }
    preempt_enable();
    return 0;
}

static int __init my_init(void) {
    task = kthread_run(my_thread_func, NULL, "my_thread");
    return 0;
}

static void __exit my_exit(void) {
    kthread_stop(task);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple kernel module");


/* makefile for cross-compile 
obj-m := test.o

KDIR := /home/rlk/xiafeng.luo/buildroot-2022.11-rc2/linux/linux-6.0.9/
CROSS_COMPILE := /home/rlk/xiafeng.luo/CrossCompileTools/gcc-linaro-12.2.1-2022.11-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
all:
	make -C $(KDIR) M=$(PWD) ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) modules
clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions modules.* Module.* *.mod
 */