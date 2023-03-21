#include <linux/init.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched/task.h>
#include <linux/sched/debug.h>
#include <linux/framework/task/task_ops.h>


static int test_call_back(void* data)
{
    int32_t i = 0;
    struct task_struct* task = NULL;
    struct pid* pid = NULL;
    pid = find_get_pid(10);
    if (pid != NULL)
        task = pid_task(pid,PIDTYPE_PID);
    
    
    for (i = 0; i < 30; i++)
    {
        //printk("pid=[%d],name=[%s],test_call_back i = %d\n",current->pid,current->comm,i);
        //get_all_task_on_each_sched_class();
        //get_tasks_on_running_queue();
        //sched_show_task(task);
        //get_all_task_on_each_sched_id();
        //get_task_sched_statistics(task);
        get_tasks_ps();
        msleep(20000);
    }
    return 0;
}


struct task_struct * create_kthread_and_run(int (*func)(void* data),void* data,const char* task_name)
{
    struct task_struct *task = NULL;
    task = kthread_create(func, data, task_name);
    if (task == NULL)
    {
        printk("kthread_create failed\n");
        return NULL;
    }
    printk("task->pid = %d,task->name=%s\n",task->pid,task->comm);
    wake_up_process(task);
    return task;
}
EXPORT_SYMBOL_GPL(create_kthread_and_run);

const char* get_task_state_as_char(struct task_struct* task)
{
    BUILD_BUG_ON(1 + ilog2(TASK_REPORT_MAX) != ARRAY_SIZE(task_state_array));
	return task_state_array[task_state_index(task)];
}
EXPORT_SYMBOL_GPL(get_task_state_as_char);

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
    get_task_sched_statistics(t1);
    wake_up_process(t1);
    get_task_sched_statistics(t1);
    printk("kthread test_kthread_on_node_0 state:%s\n",get_task_state_as_char(t1));
    t2 = kthread_create_on_node(func,data,1,"test_kthread_on_node_1");
    if (t2 == NULL)
    {
        printk("kthread_create_on_node1 failed\n");
        return -2;
    }
    printk("kthread test_kthread_on_node_1 state:%c\n",task_state_to_char(t2));
    get_task_sched_statistics(t2);
    wake_up_process(t2);
    get_task_sched_statistics(t2);
    printk("kthread test_kthread_on_node_1 state:%c\n",task_state_to_char(t2));
    return 0;
}
EXPORT_SYMBOL_GPL(create_kthread_on_node);


int32_t get_all_sched_class(void)
{
    struct sched_class* class;
    int32_t i = 0;
    for_each_class(class){
        printk("one sched class was called,addr=0x%px\n",(void*)class);
        i++;
    }
    printk("current progress sched class addr=0x%px\n",(void*)current->sched_class);
    return i;
}
EXPORT_SYMBOL_GPL(get_all_sched_class);

//统计每个调度器上有多少个进程
int32_t get_all_task_on_each_sched_class(void)
{
    int64_t array[10][2] = {0};
    int32_t i = 0;
    struct task_struct* task;
    struct sched_class* class;
    for_each_class(class) {
        array[i][0] = (int64_t)class;
        for_each_process(task) {
            if (task->sched_class == class)
                array[i][1]++;
        }
        i++;
    }
    printk("total %d sched classes\n",i);
    for (--i; i >= 0; --i) {
        printk("sched class addr: %llx, %lld processes\n",array[i][0],array[i][1]);
    }
    return 0;
}

int32_t get_all_task_on_each_sched_id(void)
{
    int64_t array[SCHED_DEADLINE+1][2] = {0};
    struct task_struct* task;
    for_each_process(task) {
        array[task->policy][0] = task->pid;
        array[task->policy][1]++;
    }
    printk("SCHED_NORMAL: %d, %lld processes\n",0,array[0][1]);
    printk("SCHED_FIFO: %d, %lld processes\n",1,array[1][1]);
    printk("SCHED_RR: %d, %lld processes\n",2,array[2][1]);
    printk("SCHED_BATCH: %d, %lld processes\n",3,array[3][1]);
    printk("SCHED_IDLE: %d, %lld processes\n",5,array[5][1]);
    printk("SCHED_DEADLINE: %d, %lld processes\n",6,array[6][1]);

    return 0;
}

bool task_is_on_running_queue(struct task_struct* tsk)
{
    return tsk->on_rq ? true : false;  
}
EXPORT_SYMBOL_GPL(task_is_on_running_queue);

//获取处在running queue上的进程数
int32_t get_tasks_on_running_queue(void)
{
    struct task_struct* task;
    int32_t i = 0;
    int32_t total_tasks = 0;
    for_each_process(task) {
        if (task_is_on_running_queue(task))
            i++;
        total_tasks ++;
    }
    printk("totoal %d tasks, %d tasks on running queue\n",total_tasks,i);
    return i;
}
EXPORT_SYMBOL_GPL(get_tasks_on_running_queue);
//输出进程的调度统计信息，实际测试结果输出均为0,把menucnofig的kernel hacking中的schedule debug下两个都选中
//重新编译内核，再次运行，结果才会有
int32_t get_task_sched_statistics(struct task_struct* tsk)
{
    struct sched_statistics *stats = &tsk->stats;
    printk(KERN_INFO "    task name: %s\n", tsk->comm);
    printk(KERN_INFO "    wait_start: %llu\n", stats->wait_start);
    printk(KERN_INFO "    wait_max: %llu\n", stats->wait_max);
    printk(KERN_INFO "    wait_count: %llu\n", stats->wait_count);
    printk(KERN_INFO "    wait_sum: %llu\n", stats->wait_sum);
    printk(KERN_INFO "    iowait_count: %llu\n", stats->iowait_count);
    printk(KERN_INFO "    iowait_sum: %llu\n", stats->iowait_sum);
    printk(KERN_INFO "    sleep_start: %llu\n", stats->sleep_start);
    printk(KERN_INFO "    sleep_max: %llu\n", stats->sleep_max);
    printk(KERN_INFO "    sum_sleep_runtime: %llu\n",stats->sum_sleep_runtime);
    printk(KERN_INFO "    block_start: %llu\n", stats->block_start);
    printk(KERN_INFO "    block_max: %llu\n", stats->block_max);
    printk(KERN_INFO "    exec_max: %llu\n", stats->exec_max);
    printk(KERN_INFO "    slice_max: %llu\n", stats->slice_max);
    return 0;
}
EXPORT_SYMBOL_GPL(get_task_sched_statistics);

char* is_kernel_progress(struct task_struct* tsk)
{
    return tsk->mm ? "user-progress" : "kernel-progress";
}
EXPORT_SYMBOL_GPL(is_kernel_progress);

int32_t get_tasks_ps(void)
{
    struct task_struct* task;
    int32_t i = 0;
    int32_t total_tasks = 0;
    printk("\t\tPID\t\tNAME\t\tSTATE\t\tSCHED_CLASS\t\tSTART_TIME\t\tPROG_TYPE\n");
    for_each_process(task) {
        printk("%10d\t%10s\t\t%5c\t\t%10px\t%5lld\t%s\n",task->pid,task->comm,task_state_to_char(task),
                        task->sched_class,task->start_time,is_kernel_progress(task));
        total_tasks ++; 
    }
    printk("totoal %d tasks\n",total_tasks);
    return i;
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
    //create_kthread_and_run(test_call_back,NULL,"test_kthread");
    //create_kthread_on_node(test_call_back,NULL,NULL,0);
    //get_all_sched_class();
    create_kthread_and_run(test_call_back,NULL,"TEST_KTHREAD");
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

