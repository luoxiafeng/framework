#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <elf.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/ptrace.h>

#define MAX_PATH_LEN 1024

int dump_elf_from_task_struct(pid_t pid, const char* outfile_path) {
    char mem_path[MAX_PATH_LEN];
    char* buf = NULL;
    struct stat st;
    Elf64_Ehdr ehdr;
    off_t code_start, code_end, code_size;
    int mem_fd, out_fd, ret = -1;
    bool done = false;
    
    // 构造内存映像文件路径
    snprintf(mem_path, MAX_PATH_LEN, "/proc/%d/mem", pid);
    
    // 打开内存映像文件
    mem_fd = open(mem_path, O_RDONLY);
    if (mem_fd < 0) {
        perror("open mem file failed");
        goto out;
    }
    
    // 获取 task_struct 结构
    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) < 0) {
        perror("get regs failed");
        goto out;
    }
    
    // 计算代码段的大小
    code_start = regs.ehdr.e_entry;
    code_end = regs.ehdr.e_phoff + regs.ehdr.e_phnum * sizeof(Elf64_Phdr);
    code_size = code_end - code_start;
    
    // 读取 ELF 文件头
    if (pread(mem_fd, &ehdr, sizeof(Elf64_Ehdr), code_start) != sizeof(Elf64_Ehdr)) {
        perror("read elf header failed");
        goto out;
    }
    
    // 校验 ELF 文件头
    if (memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0) {
        fprintf(stderr, "invalid elf header\n");
        goto out;
    }
    
    // 分配缓冲区
    buf = malloc(code_size);
    if (!buf) {
        perror("malloc failed");
        goto out;
    }
    
    // 读取代码段内容
    if (pread(mem_fd, buf, code_size, code_start) != code_size) {
        perror("read code segment failed");
        goto out;
    }
    
    // 打开输出文件
    out_fd = open(outfile_path, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (out_fd < 0) {
        perror("open output file failed");
        goto out;
    }
    
    // 写入 ELF 文件头
    if (write(out_fd, &ehdr, sizeof(Elf64_Ehdr)) != sizeof(Elf64_Ehdr)) {
        perror("write elf header failed");
        goto out;
    }
    
    // 写入代码段内容
    if (write(out_fd, buf, code_size) != code_size) {
        perror("write code segment failed");
        goto out;
    }
    
    done = true;
    ret = 0;
    
out:
    if (buf) {
        free(buf);
    }
    if (mem_fd >= 0) {
        close(mem_fd);
    }
    if (out_fd >= 0) {
        close(out_fd);
    }
    if (!done && outfile_path) {
        unlink(outfile_path);
    }
    return ret;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <pid> <outfile>\n", argv[0]);
        return -1;
    }
    pid_t pid = atoi(argv[1]);
    const char* outfile_path = argv[2];
    if (dump_elf_from_task_struct(pid, outfile_path) != 0) {
        fprintf(stderr, "dump elf failed\n");
        return -1;
    }
    printf("dump elf success\n");
    return 0;
}