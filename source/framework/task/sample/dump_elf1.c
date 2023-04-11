#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <elf.h>

void dump_elf(int pid, char* filename){
    FILE* fp;
    Elf64_Ehdr ehdr;
    Elf64_Phdr phdr;
    struct user_regs_struct regs;
    unsigned long addr, offset, size;
    char* buf;

    ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    wait(NULL);

    fp = fopen(filename, "wb");
    if(fp == NULL){
        printf("Failed to open file %s\n", filename);
        return;
    }

    ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    addr = regs.ehdr;
    ptrace(PTRACE_PEEKDATA, pid, (void*)addr, &ehdr, sizeof(ehdr));

    if(ehdr.e_ident[EI_MAG0] != ELFMAG0 || ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr.e_ident[EI_MAG2] != ELFMAG2 || ehdr.e_ident[EI_MAG3] != ELFMAG3){
        printf("Not a valid ELF file\n");
        return;
    }

    fwrite(&ehdr, sizeof(ehdr), 1, fp);

    for(int i=0; i<ehdr.e_phnum; i++){
        addr = ehdr.e_phoff + i * ehdr.e_phentsize;
        ptrace(PTRACE_PEEKDATA, pid, (void*)addr, &phdr, sizeof(phdr));
        if(phdr.p_type != PT_LOAD){
            continue;
        }
        offset = phdr.p_offset;
        size = phdr.p_filesz;
        buf = (char*)malloc(size);
        ptrace(PTRACE_PEEKDATA, pid, (void*)phdr.p_vaddr, buf, size);
        fseek(fp, offset, SEEK_SET);
        fwrite(buf, size, 1, fp);
        free(buf);
    }

    fclose(fp);
    ptrace(PTRACE_DETACH, pid, NULL, NULL);
}

int main(int argc, char *argv[]){
    if(argc < 3){
        printf("Usage: %s <pid> <filename>\n", argv[0]);
        return 0;
    }
    int pid = atoi(argv[1]);
    char* filename = argv[2];
    dump_elf(pid, filename);
    return 0;
}