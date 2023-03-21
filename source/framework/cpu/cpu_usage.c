/* (1)统计每个cpu的空闲时间，busy时间，以及总时间
(2)统计每个cpu的中断时间，以及总时间
(3)统计每个cpu的软中断时间，以及总时间
(4)统计每个cpu的系统调用时间，以及总时间
(5)统计每个cpu的用户态时间，以及总时间
(6)统计每个cpu的内核态时间，以及总时间
(7)统计每个cpu的iowait时间，以及总时间
(8)统计每个cpu的steal时间，以及总时间
(9)统计每个cpu的nice时间，以及总时间
(10)统计每个cpu的guest时间，以及总时间
(11)统计每个cpu的guest_nice时间，以及总时间
(12)统计每个cpu的idle时间，以及总时间
(13)统计每个cpu的irq时间，以及总时间
(14)统计每个cpu的softirq时间，以及总时间
(15)统计每个cpu的system时间，以及总时间
(16)统计每个cpu的user时间，以及总时间
(17)统计每个cpu的iowait时间，以及总时间
(18)统计每个cpu的steal时间，以及总时间
(19)统计每个cpu的nice时间，以及总时间
(20)统计每个cpu的guest时间，以及总时间
(21)统计每个cpu的guest_nice时间，以及总时间
(22)统计每个cpu的idle时间，以及总时间
(23)统计每个cpu的irq时间，以及总时间
(24)统计每个cpu的softirq时间，以及总时间
(25)统计每个cpu的system时间，以及总时间
(26)统计每个cpu的user时间，以及总时间
(27)统计每个cpu的iowait时间，以及总时间
(28)统计每个cpu的steal时间，以及总时间
(29)统计每个cpu的nice时间， */

#include <linux/sched.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched/task.h>
#include <linux/sched/debug.h>
#include <linux/cpumask.h>
#include <linux/sched/isolation.h>


struct cpumask* get_cpu_masks(void)
{
    struct cpumask *mask = NULL;
    mask = housekeeping_cpumask(HK_TYPE_KTHREAD);
    return mask;
}

/*
*(1)if return 1, the cpu is in the mask. 
*(2)if return 0, the cpu is not in the mask.
*/
bool is_cpu_in_mask(int cpu)
{
    return cpumask_test_cpu(cpu,get_cpu_masks());
} 

static __init int test_init(void)
{
    int32_t i = 0;
    for(i = 0; i < NR_CPUS; i++) {
        printk("bits[%d]:%lx\n",i,get_cpu_masks()->bits[i]);
        printk("is_cpu%d_in_mask:%s\n",i,is_cpu_in_mask(i)?"yes":"no");
    }
    return 0;
}

static __exit void test_exit(void)
{
    
}


module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xiafeng.luo");