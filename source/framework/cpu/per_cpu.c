#include <linux/module.h>
#include <linux/percpu.h>
 
/* 需要掌握的内容：
 * 1.怎么为每个cpu分配对应的变量，静态分配和动态分配，
 * 需要指定变量类型和名称，也可以赋初始值。
 * 2.怎么改变变量值，通过或per_cpu或者per_cpu_ptr.
 * 3.怎么获取变量值，也可以通过per_cpu或者per_cpu_ptr.
 */
 
/* 方式一：静态的分配每cpu数组 
 * 实际上是根据系统的cpu个数来分配数组，这个数组包含有cpu个数个元素，
 * 每个cpu在这个数组中都拥有一个对应的元素。
 */
static DEFINE_PER_CPU(int, val) = 8;
/* 动态分配的指针 */
static int __percpu *aval;
 
static int __init my_init(void)
{
	int cpu,i=1;
 
	pr_info("module init\n");
	
	/* 获得本地cpu的per-cpu变量值 */
	pr_info("local cpu:%d static per-cpu:%d\n", cpu, get_cpu_var(val));
	put_cpu_var(val);
 
	/* 打印每个cpu的per-cpu变量值 */
	for_each_possible_cpu(cpu){
		pr_info("each cpu:%d static per-cpu:%d\n", cpu, per_cpu(val,cpu));
	}
 
	/* 遍历每个可能的cpu */
	for_each_possible_cpu(cpu){
		/* 选择相应的cpu的per-cpu变量并赋值 */
		per_cpu(val, cpu) = cpu;	
		/* 获取相应的cpu的per-cpu变量并显示 */
		pr_info("cpu:%d assignment per-cpu:%d\n", cpu, per_cpu(val,cpu));
		/* 使能内核抢占，因为上面的函数禁止内核抢占了 */
	}
 
	/* 方式二：动态分配per-cpu变量 */
	aval = alloc_percpu(int);
	/* 遍历每个可能的cpu */
	for_each_possible_cpu(cpu){
		/* 通过指针为动态分配的per-cpu变量赋值 */
		*per_cpu_ptr(aval, cpu) = cpu;
		/* 获取相应的cpu的per-cpu变量并显示 */
		pr_info("cpu:%d alloc per-cpu:%d\n", cpu, *per_cpu_ptr(aval, cpu));
	}
 
	return 0;
}
 
static void __exit my_exit(void)
{
	pr_info("module exit\n");
	
	free_percpu(aval);
}
 
module_init(my_init);
module_exit(my_exit);
 
MODULE_AUTHOR("xiafeng.luo");
MODULE_LICENSE("GPL v2");