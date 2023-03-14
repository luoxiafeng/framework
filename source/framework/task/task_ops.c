#include <linux/init.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched/task.h>
 

 static const char * const task_state_array[] = {

	/* states in TASK_REPORT: */
	"R (running)",		/* 0x00 */
	"S (sleeping)",		/* 0x01 */
	"D (disk sleep)",	/* 0x02 */
	"T (stopped)",		/* 0x04 */
	"t (tracing stop)",	/* 0x08 */
	"X (dead)",		/* 0x10 */
	"Z (zombie)",		/* 0x20 */
	"P (parked)",		/* 0x40 */

	/* states beyond TASK_REPORT: */
	"I (idle)",		/* 0x80 */
};

static int test_call_back(void* data)
{
    int32_t i = 0;
    for (i = 0; i < 10; i++)
    {
        printk("pid=[%d],name=[%s],test_call_back i = %d\n",current->pid,current->comm,i);
        msleep(1000);
    }
    return 0;
}


int32_t create_kthread_and_run(int (*func)(void* data),void* data)
{
    struct task_struct *task = NULL;
    task = kthread_create(func, data, "test_kthread" );
    if (task == NULL)
    {
        printk("kthread_create failed\n");
        return -1;
    }
    printk("task->pid = %d,task->name=%s\n",task->pid,task->comm);
    wake_up_process(task);
    return 0;

}

const char* get_task_state_as_char(struct task_struct* task)
{
    BUILD_BUG_ON(1 + ilog2(TASK_REPORT_MAX) != ARRAY_SIZE(task_state_array));
	return task_state_array[task_state_index(task)];
}

int32_t create_kthread_on_node(int (*func)(void* data),char* task_name,void* data,int node_id)
{
    struct task_struct* t1;
    struct task_struct* t2;
    t1 = kthread_create_on_node(func,data,0,"test_kthread_on_node_0");
    if (t1 == NULL)
    {
        printk("kthread_create_on_node0 failed\n");
        return -1;
    }
    printk("kthread test_kthread_on_node_0 state:%s\n",get_task_state_as_char(t1));
    wake_up_process(t1);
    printk("kthread test_kthread_on_node_0 state:%s\n",get_task_state_as_char(t1));
    t2 = kthread_create_on_node(func,data,1,"test_kthread_on_node_1");
    if (t2 == NULL)
    {
        printk("kthread_create_on_node1 failed\n");
        return -2;
    }
    printk("kthread test_kthread_on_node_1 state:%c\n",task_state_to_char(t2));
    wake_up_process(t2);
    printk("kthread test_kthread_on_node_1 state:%c\n",task_state_to_char(t2));
    return 0;
}
int32_t create_kthread_and_wait(void)
{
    return 0;
}

int32_t create_kthread_and_wait_for_completion(void)
{
    return 0;
}



static int __init my_kthread_ops_init(void)
{
    //create_kthread_and_run(test_call_back,NULL);
    create_kthread_on_node(test_call_back,NULL,NULL,0);
    return 0;
}
 
static void __exit my_kthread_ops_exit(void)
{
        printk(KERN_EMERG "hello, exit\n");
}
 
module_init(my_kthread_ops_init);
module_exit(my_kthread_ops_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("xiafeng.luo");
MODULE_DESCRIPTION("This is just a hello module!\n");

