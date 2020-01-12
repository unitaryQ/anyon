#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <types.h>
#include <list.h>
#include <vm.h>

//since any scheduling occurs in kernel, segment regs are well-defined
typedef struct process_context_s{
    uint32_t eflags; // +0x00
    uint32_t esp; // + 0x04
    uint32_t eip; // + 0x08
    uint32_t ecx; // + 0x0c
    uint32_t edx; //+ 0x10
    uint32_t ebx; //+ 0x14
    uint32_t ebp;// + 0x18
    uint32_t esi;// + 0x1c
    uint32_t edi;// + 0x20
}process_context_t;

#define MAX_NAME_LEN 0x20
#define MAX_PID      0x400 //1024

typedef struct process_s{
    list_t   plink; //general process list
    list_t   qlink; //specific queue the process in
    uint32_t state;// running, ready, sleep, exit. maybe more
    uint32_t pid;
    char name[MAX_NAME_LEN];
    struct process_s* parent;
    list_t children;
    list_t brothers;
    uint32_t children_num;
    uint32_t page_dir; // physical address of page directory (CR3)
    virtual_zone_t* vzone;
    uint32_t kernel_stack;
    process_context_t context;
}process_t;

enum process_state {RUN = 1, READY = 2, STOP = 4, EXIT = 8};

process_t* current;

int init_kernel_thread();
#endif