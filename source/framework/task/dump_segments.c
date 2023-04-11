#include <linux/sched.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched/task.h>
#include <linux/sched/debug.h>
#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <asm/page.h>
#include <linux/pfn.h>

/*
*(1)according to my test,this function output all empty
*(2)task_struct->mm->exe_file d_path() return NULL
*(3)task_struct->active_mm->exe_file d_path() return NULL
*/
char* dump_task_exec_file_path(struct task_struct* tsk)
{
    struct mm_struct* mm = NULL;
    struct file* exe_file = NULL;
    char* exe_file_path = NULL;

    if (tsk == NULL) {
        printk("tsk is NULL\n");
        return NULL;
    }
    mm = tsk->mm;
    if (mm)
        exe_file = mm->exe_file;
    else if(tsk->active_mm)
        exe_file = tsk->active_mm->exe_file;
    
    if (exe_file)
    {
        exe_file_path = d_path(&exe_file->f_path, NULL, 0);
            printk("exe_file is %s\n",exe_file_path);
        return exe_file_path;
    }
    
    return NULL;
}

/*
*(1)第一个参数表示需要dump的进程
*(2)第二个参数表示dump到哪个目录
*(3)得到的文件，跟源elf文件计算md5值，是一样的
*/
asmlinkage long dump_elf(struct task_struct* task,const char *path) {
    struct mm_struct *mm = task->mm;
    struct vm_area_struct *vma;
    unsigned long start, end;
    struct file* file;
    struct file* vm_file;
    char* buf = NULL;
    char* vm_file_name = NULL;
    loff_t pos = 0;
    ssize_t wret = 0, rret = 0;
    int32_t vma_count = 0;
    char file_name[256] = {0};
    
    //iterate all vma 
    for (vma = mm->mmap; vma != NULL; vma = vma->vm_next) {
        printk("vma->vm_start = %lx,vma->vm_end = %lx\n", vma->vm_start, vma->vm_end);
        start = vma->vm_start;
        end = vma->vm_end;
        vm_file = vma->vm_file;
        if (start >= end) {
            printk(KERN_ERR "Invalid start/end address\n");
            return -1;
        }
        if (start == 0) {
            printk(KERN_ERR "Invalid start address\n");
            return -1;
        }
        if (IS_ERR(vm_file)) {
            printk(KERN_ERR "Invalid vm_file\n");
            return -1;
        }
        //check f_path.dentry not null
        if ( !vm_file || !vm_file->f_path.dentry ) 
            continue;
    
        vm_file_name = vm_file->f_path.dentry->d_iname;
        printk("vm_file_name = %s\n", vm_file_name);

        if (strcmp(vm_file_name,task->comm) == 0) {
            vma_count ++;
            memset(file_name,0,sizeof(file_name));
            sprintf(file_name,"%s/%s_%d",path,vm_file_name,vma_count);
            file = filp_open(file_name, O_RDWR | O_CREAT | O_TRUNC, 0655);
            if (IS_ERR(file)) {
                printk(KERN_ERR "Failed to open file %s\n", file_name);
                return -1;
            }
            //read from vma->vm_start to vma->vm_end
            buf = kmalloc(vm_file->f_inode->i_size,GFP_KERNEL);
            if (buf == NULL) {
                printk(KERN_ERR "Failed to kmalloc\n");
                return -1;
            }
            printk("vma->vm_start=%lx,vma->vm_end=%lx,vm_file->f_inode->i_size=%lld\n",
                        vma->vm_start,vma->vm_end,vm_file->f_inode->i_size);
            memset(&pos,0,sizeof(pos));
            rret = kernel_read(vm_file, buf, vm_file->f_inode->i_size, &pos);
            if (rret <= 0) {
                printk(KERN_ERR "Failed to read vma->vm_file %s\n", vm_file->f_path.dentry->d_iname);
                continue;
            }
            //write to file in filesystem
            memset(&pos,0,sizeof(pos));
            vfs_llseek(file, 0, SEEK_SET);
            wret = kernel_write(file, buf, rret, &pos);
            if (wret <= 0) {
                printk(KERN_ERR "Failed to write file %s\n", file_name);
                continue;
            }
            printk("read len:%ld,write len:%ld\n",rret,wret);
            filp_close(file, NULL);
            kfree(buf);
        }
    }

    return 0;
}
EXPORT_SYMBOL_GPL(dump_elf);

/*
*(1)para1:
*(2)para2:path to store dump file
*(3)para3:section type,like .text,.data,.bss.when we dump all elf data from
*   vma->vm_file,section name is usless
*(4)dump from vma->vm_file to file in filesystem success
*/
asmlinkage long dump_segments_from_vmafile(struct task_struct* task,const char *path,char* section_type) {
    struct mm_struct *mm = task->mm;
    struct vm_area_struct *vma;
    unsigned long start, end;
    struct file* file;
    struct file* vm_file;
    char* buf = NULL;
    char* vm_file_name = NULL;
    loff_t pos = 0;
    ssize_t wret = 0, rret = 0;
    char file_name[256] = {0};
    
    //iterate all vma 
    for (vma = mm->mmap; vma != NULL; vma = vma->vm_next) {
        start = vma->vm_start;
        end = vma->vm_end;
        vm_file = vma->vm_file;
        if (start >= end) {
            printk(KERN_ERR "Invalid start/end address\n");
            return -1;
        }
        if (start == 0) {
            printk(KERN_ERR "Invalid start address\n");
            return -1;
        }
        if (IS_ERR(vm_file)) {
            printk(KERN_ERR "Invalid vm_file\n");
            return -1;
        }
        //check f_path.dentry not null
        if ( !vm_file || !vm_file->f_path.dentry ) 
            continue;
    
        vm_file_name = vm_file->f_path.dentry->d_iname;
        
        if (strcmp(section_type,".text") == 0) {
            if ( !(vma->vm_flags & VM_EXEC) ) {
                continue;
            }
        } else if (strcmp(section_type,".data") == 0) {
            if ( !(vma->vm_flags & VM_WRITE) ) {
                continue;
            }
        } else if (strcmp(section_type,".rodata") == 0) {
            if ( !(vma->vm_flags & VM_READ) ) {
                continue;
            }
        } 

        memset(file_name,0,sizeof(file_name));
        //sprintf(file_name,"%s/%s%s",path,vm_file_name,section_type);
        sprintf(file_name,"%s/%s.bin",path,vm_file_name);
        file = filp_open(file_name, O_RDWR | O_CREAT | O_TRUNC, 0655);
        if (IS_ERR(file)) {
            printk(KERN_ERR "Failed to open file %s\n", file_name);
            return -1;
        }
        //read from vma->vm_start to vma->vm_end
        buf = kmalloc(vm_file->f_inode->i_size,GFP_KERNEL);
        if (buf == NULL) {
            printk(KERN_ERR "Failed to kmalloc\n");
            return -1;
        }
        printk("vm_file_name:%s,vma->vm_start=0x%lx,vma->vm_end=0x%lx,vma_size=%ld,vm_file->f_inode->i_size=%lld\n",
                    vm_file_name,vma->vm_start,vma->vm_end,
                        (vma->vm_end - vma->vm_start),vm_file->f_inode->i_size);
        memset(&pos,0,sizeof(pos));
        rret = kernel_read(vm_file, buf, vm_file->f_inode->i_size, &pos);
        if (rret <= 0) {
            printk(KERN_ERR "Failed to read vma->vm_file %s\n", vm_file->f_path.dentry->d_iname);
            continue;
        }
        //write to file in filesystem
        memset(&pos,0,sizeof(pos));
        vfs_llseek(file, 0, SEEK_SET);
        wret = kernel_write(file, buf, rret, &pos);
        if (wret <= 0) {
            printk(KERN_ERR "Failed to write file %s\n", file_name);
            continue;
        }
        printk("read len:%ld,write len:%ld\n",rret,wret);
        filp_close(file, NULL);
        kfree(buf);
        break;
    }

    return 0;
}
EXPORT_SYMBOL_GPL(dump_segments_from_vmafile);

/*
*(1)para1:
*(2)para2:path to store dump file
*(3)para3:section type,like .text,.data,.bss
*(4)dump from vma->vm_start to file in filesystem
*   vm_area_struct not represent to .text,.data,.bss
*/
asmlinkage long dump_segments_from_vma(struct task_struct* task,const char *path,char* section_type) {
    struct mm_struct *mm = task->mm;
    struct vm_area_struct *vma;
    unsigned long start, end;
    struct file* file;
    struct file* vm_file;
    char* buf = NULL;
    char* vm_file_name = NULL;
    loff_t pos = 0;
    ssize_t wret = 0, rret = 0;
    char file_name[256] = {0};
    
    //iterate all vma 
    for (vma = mm->mmap; vma != NULL; vma = vma->vm_next) {
        start = vma->vm_start;
        end = vma->vm_end;
        vm_file = vma->vm_file;
        if (start >= end) {
            printk(KERN_ERR "Invalid start/end address\n");
            return -1;
        }
        if (start == 0) {
            printk(KERN_ERR "Invalid start address\n");
            return -1;
        }
        if (IS_ERR(vm_file)) {
            printk(KERN_ERR "Invalid vm_file\n");
            return -1;
        }
        //check f_path.dentry not null
        if ( !vm_file || !vm_file->f_path.dentry ) 
            continue;
    
        vm_file_name = vm_file->f_path.dentry->d_iname;
        
        if (strcmp(section_type,".text") == 0) {
            if ( !(vma->vm_flags & VM_EXEC) ) {
                continue;
            }
        } else if (strcmp(section_type,".data") == 0) {
            if ( !(vma->vm_flags & VM_WRITE) ) {
                continue;
            }
        } else if (strcmp(section_type,".rodata") == 0) {
            if ( !(vma->vm_flags & VM_READ) ) {
                continue;
            }
        } 

        memset(file_name,0,sizeof(file_name));
        sprintf(file_name,"%s/%s%s",path,vm_file_name,section_type);
        file = filp_open(file_name, O_RDWR | O_CREAT | O_TRUNC, 0655);
        if (IS_ERR(file)) {
            printk(KERN_ERR "Failed to open file %s\n", file_name);
            return -1;
        }
        //read from vma->vm_start to vma->vm_end
        //buf = kmalloc(vm_file->f_inode->i_size,GFP_KERNEL);
        buf = kmalloc(vma->vm_end - vma->vm_start,GFP_KERNEL);
        if (buf == NULL) {
            printk(KERN_ERR "Failed to kmalloc\n");
            return -1;
        }
        printk("vm_file_name:%s,vma->vm_start=%lx,vma->vm_end=%lx,vma_size=%lx,vm_file->f_inode->i_size=%lld\n",
                    vm_file_name,vma->vm_start,vma->vm_end,
                        (vma->vm_end - vma->vm_start),vm_file->f_inode->i_size);
        memset(&pos,0,sizeof(pos));
        /* rret = kernel_read(vma->vm_start, buf, (vma->vm_end - vma->vm_start), &pos);
        if (rret <= 0) {
            printk(KERN_ERR "Failed to read vma->vm_file %s\n", vm_file->f_path.dentry->d_iname);
            continue;
        } */
        //memcpy(buf,(void*)vma->vm_start,(vma->vm_end - vma->vm_start));
        rret = access_process_vm(task,vma->vm_start,buf,(vma->vm_end - vma->vm_start),0);
        //write to file in filesystem
        memset(&pos,0,sizeof(pos));
        vfs_llseek(file, 0, SEEK_SET);
        wret = kernel_write(file, buf, rret, &pos);
        if (wret <= 0) {
            printk(KERN_ERR "Failed to write file %s\n", file_name);
            continue;
        }
        printk("read len:%ld,write len:%ld\n",rret,wret);
        filp_close(file, NULL);
        kfree(buf);
        
    }

    return 0;
}
EXPORT_SYMBOL_GPL(dump_segments_from_vma);

/*
*(1)if vma->vm_file->f_path.dentry->d_iname equal to task->comm
*(2)For task_struct of test.out,there may be one more vmas,so i 
*   dump all vmas to file in filesystem
*(3)ERROR,i cant get all elf data from vma->vm_start to vma->vm_end
*   as i dump all data from vma->vm_start to vma->vm_end to file in filesystem,
*   but the file size is not equal to the size of elf file,and the data is not
*   equal to the data of elf file.
*/
asmlinkage long dump_segments_from_vmas(struct task_struct* task,const char *path) {
    struct mm_struct *mm = task->mm;
    struct vm_area_struct *vma;
    unsigned long start, end;
    struct file* file;
    struct file* vm_file;
    char* buf = NULL;
    char* vm_file_name = NULL;
    loff_t pos = 0;
    ssize_t wret = 0, rret = 0;
    char file_name[256] = {0};
    uint32_t vma_count = 0;
    uint32_t vma_mem_readsize = 0;
    uint32_t vm_file_size = 0;
    uint32_t vma_area_sizes = 0;
    bool remapped = false;
    unsigned long pfn = 0;
    memset(file_name,0,sizeof(file_name));
    sprintf(file_name,"%s/%s",path,task->comm);
    printk("\n");
    //iterate all vma 
    for (vma = mm->mmap; vma != NULL; vma = vma->vm_next) {
        start = vma->vm_start;
        end = vma->vm_end;
        vm_file = vma->vm_file;
        if (start >= end) {
            printk(KERN_ERR "Invalid start/end address\n");
            return -1;
        }
        if (start == 0) {
            printk(KERN_ERR "Invalid start address\n");
            return -1;
        }
        if (IS_ERR(vm_file)) {
            printk(KERN_ERR "Invalid vm_file\n");
            return -1;
        }
        //check f_path.dentry not null
        if ( !vm_file || !vm_file->f_path.dentry ) 
            continue;
    
        vm_file_name = vm_file->f_path.dentry->d_iname;
        vm_file_size = vm_file->f_inode->i_size;
        printk("vm_file_name:%s  ",vm_file_name);
        
        if (strcmp(vm_file_name,task->comm) == 0) {
            vma_count++;
            //first time,file not exist
            if (vma_count == 1) {
                file = filp_open(file_name, O_RDWR | O_CREAT | O_TRUNC, 0655);
                if (IS_ERR(file)) {
                    printk(KERN_ERR "Failed to open file %s\n", file_name);
                    return -1;
                }
            } else if (vma_count > 1) {
                file = filp_open(file_name, O_RDWR | O_APPEND, 0655);
                if (IS_ERR(file)) {
                    printk(KERN_ERR "Failed to open file %s\n", file_name);
                    return -1;
                }
            }
            //vma->vm_start may not maaped to physical memory,remap it
            if (virt_to_phys((void*)vma->vm_start) == 0) {
                pfn = PFN_DOWN(virt_to_phys((void*)vma->vm_start));
                remap_pfn_range(vma, vma->vm_start, pfn, vma->vm_end - vma->vm_start, vma->vm_page_prot);
                remapped = true;
            }
            buf = kmalloc(vma->vm_end - vma->vm_start,GFP_KERNEL);
            if (buf == NULL) {
                printk(KERN_ERR "Failed to kmalloc\n");
                return -1;
            }
           /*  printk("vm_file_name:%s,vma->vm_start=%lx,vma->vm_end=%lx,vma_size=%lx,vm_file->f_inode->i_size=%d,vm_pgoff=%ld\n",
                        vm_file_name,vma->vm_start,vma->vm_end,
                            (vma->vm_end - vma->vm_start),vm_file_size,vma->vm_pgoff); */
            vma_area_sizes += (vma->vm_end - vma->vm_start);
            memset(&pos,0,sizeof(pos));
            rret = access_process_vm(task,vma->vm_start,buf,(vma->vm_end - vma->vm_start),0);
            vma_mem_readsize += rret;
            //write to file in filesystem
            memset(&pos,0,sizeof(pos));
            if (vma_count == 1) 
                vfs_llseek(file, 0, SEEK_SET);
            wret = kernel_write(file, buf, rret, &pos);
            if (wret <= 0) {
                printk(KERN_ERR "Failed to write file %s\n", file_name);
                continue;
            }
            printk("read len:%ld,write len:%ld\n",rret,wret);
            kfree(buf);
            filp_close(file, NULL);
            //unmap vma->vm_start
            if (remapped)
                vm_unmap_ram((void*)vma->vm_start, vma->vm_end - vma->vm_start);
        }
    }
    //close file
    printk("vma_count=%d,vma_mem_readsize=%d,vm_file_size=%d,vma_area_size=%d\n",
                vma_count,vma_mem_readsize,vm_file_size,vma_area_sizes);
    
    return 0;
}
EXPORT_SYMBOL_GPL(dump_segments_from_vmas);

asmlinkage long dump_segments_from_mm(struct task_struct* task,const char *path,const char* segment_type) {
    struct mm_struct *mm = task->mm;
    long long start_code, end_code, start_data, end_data, start_brk, brk, start_stack;
    struct file* file;
    char file_name[256] = {0};
    char* buf = NULL;
    loff_t pos = 0;
    ssize_t wret = 0;
    
    start_code = mm->start_code;
    end_code = mm->end_code;
    start_data = mm->start_data;
    end_data = mm->end_data;
    
    memset(file_name,0,sizeof(file_name));
    sprintf(file_name,"%s/%s%s",path,task->comm,segment_type);
    file = filp_open(file_name, O_RDWR | O_CREAT | O_TRUNC, 0655);
    if (IS_ERR(file)) {
        printk(KERN_ERR "Failed to open file %s\n", file_name);
        return -1;
    }
    buf = kmalloc(end_code - start_code,GFP_KERNEL);
    if (buf == NULL) {
        printk(KERN_ERR "Failed to kmalloc\n");
        return -1;
    }
    memset(&pos,0,sizeof(pos));
    vfs_llseek(file, 0, SEEK_SET);
    printk("start_code=%llx,end_code=%llx,start_data=%llx,end_data=%llx,start_brk=%llx,brk=%llx\n",
                start_code,end_code,start_data,end_data,start_brk,brk);
    if (strcmp(segment_type,".text") == 0) {
        access_process_vm(task,start_code,buf,(end_code - start_code),0);
        wret = kernel_write(file, buf, (end_code - start_code), &pos);
    }else if (strcmp(segment_type,".data") == 0) {
        access_process_vm(task,start_data,buf,(end_data - start_data),0);
        wret = kernel_write(file, buf, (end_data - start_data), &pos);
    }
    if (wret <= 0) {
        printk(KERN_ERR "Failed to write file %s\n", file_name);
        return -1;
    }
    printk("write len:%ld\n",wret);
    filp_close(file, NULL);
    kfree(buf); 
    return 0;
}
EXPORT_SYMBOL_GPL(dump_segments_from_mm);

static __init int dump_task_struct_segments_init(void)
{
    //dump task_struct->mm_struct->start_code...
    struct task_struct* tsk = NULL;
    for_each_process(tsk) {
        //dump all user-space process
        if (tsk->mm == NULL)
            continue;
        printk("task->pid = %d,task->name=%s    ,",tsk->pid,tsk->comm);
        /* printk("\n----------------------------------------text\n");
        dump_segments_from_vma(tsk,"/home/summer/elf",".text");
        printk("----------------------------------------data\n");
        dump_segments_from_vma(tsk,"/home/summer/elf",".data"); */
        //dump_segments_from_vmas(tsk,"/home/summer/elf");
        /* dump_segments_from_mm(tsk,"/home/summer/elf",".text");
        dump_segments_from_mm(tsk,"/home/summer/elf",".data"); */
        dump_segments_from_vmafile(tsk,"/home/summer/elf",".text");
        printk("\n\n");
    }
    return 0;
}

static __exit void dump_task_struct_segments_exit(void)
{
    
}


module_init(dump_task_struct_segments_init);
module_exit(dump_task_struct_segments_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("xiafeng.luo");