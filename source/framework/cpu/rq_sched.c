#include <linux/init.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/framework/cpu/rq_sched.h>
#include <linux/framework/task/task_ops.h>
#define MAX_CPUS NR_CPUS


static int test_call_back(void* data)
{
        while(1) {
            get_each_cpu_rq_info();
            msleep(1000);
        }
        return 0;
}

int32_t get_each_cpu_rq_info(void)
{
        struct rq* rq;
        struct cfs_rq* cfs_rq_ = NULL;
        int32_t i = 0;
        for_each_possible_cpu(i) {
            rq = cpu_rq(i);
            printk("cpu:%d, running_tasks:%d,ttwu_pending:%d,nr_switches=%lld\n",i,
                            rq->nr_running,rq->ttwu_pending,rq->nr_running + rq->nr_switches);
            printk("cfs: ");
        }
        return 0;
}
EXPORT_SYMBOL_GPL(get_each_cpu_rq_info);

static int __init rq_sched_init(void)
{
        create_kthread_and_run(test_call_back,NULL,"test_call_back");   
        return 0;
}
 
static void __exit rq_sched_exit(void)
{
        printk(KERN_EMERG "hello, exit\n");
}
 
module_init(rq_sched_init);
module_exit(rq_sched_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("xiafeng.luo");
MODULE_DESCRIPTION("This is just a hello module!\n");

