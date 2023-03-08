#include <linux/init.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/types.h>
#include <linux/time_types.h>
#include "linux/framework/time/time_ops.h"

long get_current_time_byns(void)
{
    time64_t ns;
    ns = ktime_get_real_seconds();
    return ns;
}
EXPORT_SYMBOL(get_current_time_byns);

long get_current_time_byus(void)
{
    time64_t ns,us;
    ns = ktime_get_real_seconds();
    us = ns / 1000;
    return us;
}
EXPORT_SYMBOL(get_current_time_byus);

long get_current_time_bymsec(void)
{
    time64_t ns,msec;
    ns = ktime_get_real_seconds();
    msec = ns / (1000 * 1000);
    return msec;
}
EXPORT_SYMBOL(get_current_time_bymsec);

long get_current_time_bysec(void)
{
    time64_t ns,sec;
    ns = ktime_get_real_seconds();
    sec = ns / (1000 * 1000 * 1000);
    return sec;
}
EXPORT_SYMBOL(get_current_time_bysec);

void displayCurrentTimeByYMDS(void)
{
    struct tm tm;
    time64_t now = ktime_get_real_seconds();
    time64_to_tm(now, 0, &tm);
    printk("Current time is %04ld-%02d-%02d %02d:%02d:%02d\\\\n",
	   tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min,
	   tm.tm_sec);
}
EXPORT_SYMBOL(displayCurrentTimeByYMDS);

void get_current_time_byYMD(struct tm* tm)
{
    time64_t now = ktime_get_real_seconds();
    time64_to_tm(now, 0, tm);
}
EXPORT_SYMBOL(get_current_time_byYMD);

static int __init my_time_init(void)
{
        unsigned long start,end;
        start = get_current_time_byns();
        end = get_current_time_byns();
        printk("start = %lu,end = %lu,diff = %lu\n",start,end,end - start);
        displayCurrentTimeByYMDS();
        return 0;
}
static void __exit my_time_exit(void)
{
        printk(KERN_EMERG "hello, exit\n");
}
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xiafeng.luo");
MODULE_DESCRIPTION("This is just a hello module!\n");
module_init(my_time_init);
module_exit(my_time_exit);

