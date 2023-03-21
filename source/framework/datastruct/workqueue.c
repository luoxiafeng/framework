#include <linux/sched.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/completion.h>
#include <linux/framework/task/task_ops.h>
#include <linux/kthread.h>
#include <linux/delay.h>
DECLARE_COMPLETION_ONSTACK(done);

static int my_workqueue(void* data)
{
    int32_t i = 0;
    while(1) {
        if (i > 5)
            break;
        msleep(1000);
        printk("waiting event at %d secs\n",i);
        i ++;
    }
    printk("wait event success,and wake up the waiting thread\n");
    complete(&done);
    return 0;
}

static __init int my_workqueue_init(void)
{
    struct task_struct* task;
    task = create_kthread_and_run(my_workqueue,NULL,"my_workqueue");
    if(IS_ERR(task)) 
        printk(KERN_EMERG "create kthread failed\n");
    printk("wait for completion\n");
    wait_for_completion(&done);//blocked and wait until complete(&done) is called
    printk("completion\n");
    
    return 0;
}

static __exit void my_workqueue_exit(void)
{
    printk(KERN_EMERG "hello, exit\n");
    
}


module_init(my_workqueue_init);
module_exit(my_workqueue_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("xiafeng.luo");