#include <linux/sched.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched/task.h>
#include <linux/sched/debug.h>

static __init int test_init(void)
{
   
}

static __exit void test_exit(void)
{
    
}


module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xiafeng.luo");