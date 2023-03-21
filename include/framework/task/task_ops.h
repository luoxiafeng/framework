#ifndef __TASK_OPS_H__
#define __TASK_OPS_H__
#include <asm-generic/vmlinux.lds.h>
#include <linux/kernel.h>
#include "/home/rlk/xiafeng.luo/buildroot-2022.11-rc2/linux/linux-6.0.9/kernel/sched/sched.h"

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

extern struct sched_class __sched_class_highest[];
extern struct sched_class __sched_class_lowest[];

#define for_class_range(class, _from, _to) \
	for (class = (_from); class < (_to); class++)

#define for_each_class(class) \
	for_class_range(class, __sched_class_highest, __sched_class_lowest)

int32_t get_all_task_on_each_sched_class(void);
int32_t get_tasks_on_running_queue(void);
int32_t get_task_sched_statistics(struct task_struct* tsk);
int32_t get_tasks_ps(void);
int32_t get_all_task_on_each_sched_id(void);
struct task_struct *  create_kthread_and_run(int (*func)(void* data),void* data,const char* task_name);
int32_t get_all_tasks_on_kthread_create_list(void);

#endif
