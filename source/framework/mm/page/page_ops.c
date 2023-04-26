#include <linux/sched.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched/task.h>
#include <linux/sched/debug.h>
#include <asm/pgtable.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/kernel.h>   

static unsigned long vir_addr = 100;
static pid_t pid = 0;
module_param(vir_addr, ulong, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
module_param(pid, int, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
/*
*(1)获取进程的页表项基地址寄存器的值（用户空间）
*/
long get_ttba0_el1(void)
{
	long ttb;
	asm volatile("mrs %0, ttbr0_el1\n" : "=r"(ttb));
	return ttb;
}
EXPORT_SYMBOL_GPL(get_ttba0_el1);
/*
(1)获取进程的页表项基地址寄存器的值（内核空间）
*/
long get_ttba1_el1(void)
{
	long ttb;
	asm volatile("mrs %0, ttbr1_el1\n" : "=r"(ttb));
	return ttb;
}
EXPORT_SYMBOL_GPL(get_ttba1_el1);

/*
*(1)通过遍历页表的方式，来看虚拟地址对应的物理地址是否存在.若果不存在，就表示缺页。
*(2)是否可以得出结论：内核中的（虚拟页--物理页）并不在也表中？
*/
int is_page_fault(struct task_struct* task,unsigned long vir_addr)
{
	struct mm_struct *mm = NULL;
	pgd_t *pgd = NULL;
	p4d_t *p4d = NULL;
	pud_t *pud = NULL;
	pmd_t *pmd = NULL;
	pte_t *pte = NULL;

	mm = task->mm;
	if (mm == NULL) {
		printk("kernel process,address map is not in page-table\n");
		return 0;
	}
	pgd = pgd_offset(mm, vir_addr);
	if (pgd_none(*pgd) || pgd_bad(*pgd)) {
		printk("pgd_none(*pgd) || pgd_bad(*pgd) error\n");
		return 0;
	}
	p4d = p4d_offset(pgd, vir_addr);
	if (p4d_none(*p4d) || p4d_bad(*p4d)) {
		printk("p4d_none(*p4d) || p4d_bad(*p4d) error\n");
		return 0;
	}
	pud = pud_offset(p4d,vir_addr);
	if (pud_none(*pud) || pud_bad(*pud)) {
		printk("pud_none(*pud) || pud_bad(*pud) error\n");
		return 0;
	}
	pmd = pmd_offset(pud,vir_addr);
	if (pmd_none(*pmd) || pmd_bad(*pmd)) {
		printk("pmd_none(*pmd) || pmd_bad(*pmd) error\n");
		return 0;
	}
	pte = pte_offset_kernel(pmd,vir_addr);
	if (pte_present(*pte)) {
		printk("pte_present(*pte) is true\n");
		return 1;
	} else {
		printk("pte_present(*pte) is false\n");
		return 0;
	}
}
EXPORT_SYMBOL_GPL(is_page_fault);

/*
*(1)通过内存反向映射的方式，来看物理地址对应的虚拟地址有多少个？
*(1.1)通过遍历所有的进程task_struct->mm->maps（vma）,查看每个进程所有的虚拟地址中是否有
*     目标物理地址？
*(1.2)
*/
int count_ptes_by_phyaddr(void)
{
	return 0;
}

/*
*(1)下面这种方法成功的获取到了虚拟地址对应的物理地址。并且与另一个app处理得到的结果一致。
*(2)我们通过把mm/samples/virt_to_phys.c这个文件编译，得到一个可执行文件。运行这个可执行文件，
*   输入进程号，输入虚拟地址，就可以得到对应的物理地址。我们以当前系统中的任意一个进程为例，虚拟地址取
*	任意进程的libc库的虚拟地址。
*(3)如何获取进程号对应的虚拟地址映射？cat /proc/pid/maps，找到libc对应的虚拟地址
*(4)insmod page_ops.ko pid=进程号 vir_addr=虚拟地址,如insmod page_ops.ko pid=1 vir_addr=0x7f7f7f7f7f7f7f7f
*   最终你会发现，当前这个函数得到的物理地址，跟virt_to_phys_calc.c得到的物理地址是一致的。
*(5)这个函数的实现，是通过遍历页表，得到虚拟地址对应的物理地址。
*(6)注意，无论是virt_to_phys还是当前这个接口，只能处理用户空间的虚拟地址。0xffff000000000000往上的地址是直接映射的，
*   无法通过页表来获取物理地址。所以，对于内核空间的虚拟地址，我们需要重新写一个接口来计算。
*/
unsigned long  virt_to_phy_calc(struct task_struct* tsk,long virt_addr)
{
	struct page *page;
	struct mm_struct *mm = NULL;
	unsigned long phy_addr = 0;
	pgd_t *pgd = NULL;
	p4d_t *p4d = NULL;
	pud_t *pud = NULL;
	pmd_t *pmd = NULL;
	pte_t *pte = NULL;
	if (tsk != NULL) 
		mm = tsk->mm;
	else 
		return 0;
	
	if (virt_addr > 0xffff000000000000) {
		phy_addr = __pa(virt_addr);
		goto out;
	}
    pgd = pgd_offset(mm, virt_addr);
	if (pgd_none(*pgd) || pgd_bad(*pgd)) {
		printk("pgd_none(*pgd) || pgd_bad(*pgd) error\n");
		return 0;
	}
	p4d = p4d_offset(pgd, virt_addr);
	if (p4d_none(*p4d) || p4d_bad(*p4d)) {
		printk("p4d_none(*p4d) || p4d_bad(*p4d) error\n");
		return 0;
	}
	pud = pud_offset(p4d, virt_addr);
	if (pud_none(*pud) || pud_bad(*pud)) {
		printk("pud_none(*pud) || pud_bad(*pud) error\n");
		return 0;
	}
	pmd = pmd_offset(pud, virt_addr);
	if (pmd_none(*pmd) || pmd_bad(*pmd)) {
		printk("pmd_none(*pmd) || pmd_bad(*pmd) error\n");
		return 0;
	}
	/*
	*(1)对于虚拟地址范围在用户空间的，即小于0xffff000000000000的虚拟地址，
	*	我们使用pte_offset_kernel()获取到的物理地址跟virt_to_phys_calc.c中的结果一致。
	*(2)同样的场景，我们考虑使用pte_offset_map()来试一下。结果是一样的。难道说这两个
	*	函数的实现是一样的？？？
	*(3)对于用户空间的虚拟地址，无论是pte_offset_kernel()还是pte_offset_map()，得到的
	*   结果都是一样的。但是对于内核空间的虚拟地址，需要使用__pa来转换。
	*/
	pte = pte_offset_map(pmd, virt_addr);
	if (pte_none(*pte)) {
		printk("pte_none(*pte)  error\n");
		return 0;
	}
	if (!pte_present(*pte)) {
		printk("pte_present(*pte) error\n");
		return 0;
	}
	page = pte_page(*pte);
	phy_addr = page_to_phys(page);

out:
	printk("pid = %d, vir_addr = %lx,phy_addr = 0x%lx\n",tsk->pid,virt_addr,phy_addr);
	return phy_addr;
}
EXPORT_SYMBOL_GPL(virt_to_phy_calc);

void show_each_progress_pgd(void)
{
	struct task_struct *task;
	int tsk_pgd[4096][2] = { 0 };
	long ttba0, ttba1;
	int i;
	for_each_process(task) {
		ttba0 = get_ttba0_el1();
		ttba1 = get_ttba1_el1();
		//if userspace task
		if (task->mm != NULL) {
			printk("[user] task->pid = %d,task->name=%s,pgd=%px,ttba0=%lx,ttba1=%lx\n",
			       task->pid, task->comm, task->mm->pgd, ttba0,
			       ttba1);
			tsk_pgd[i][0] = task->pid;
			tsk_pgd[i][1] = (int)task->mm->pgd;
			i++;
		}
	}
}

static __init int test_init(void)
{
	unsigned long phy_addr;
	struct pid *pid_t = NULL;
	struct task_struct *tsk = NULL;
	show_each_progress_pgd();
	pid_t = find_get_pid(pid);
	tsk = pid_task(pid_t, PIDTYPE_PID);
	phy_addr = virt_to_phy_calc(tsk,vir_addr);
	return 0;
	
}

static __exit void test_exit(void)
{
}

module_init(test_init);
module_exit(test_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xiafeng.luo");