#include <process.h>
#include <allocator.h>
#include <utils.h>
#include <lock.h>
#include <scheduler.h>
#include <stdio.h>
#include <interrupt.h>
#include <vm.h>

static uint32_t process_num;
static uint32_t max_pid;
extern void lift_kernel_thread();

process_t* alloc_pcb(){
    process_t* pcb = (process_t*)kalloc(sizeof(process_t)); 
    if(pcb == NULL){
        return NULL;
    }
    memset(pcb,sizeof(process_t));
    list_init(&pcb->plink);
    list_init(&pcb->qlink);
    list_init(&pcb->brothers);
    return pcb;
}

//this function is running in user mode as the first user task
void user_idle(){
    while(1){}
}

//lift the kernel thread to user process
void kernel_to_user(process_t* self){
    virtual_zone_t* vzone = create_virtual_zone();
    virtual_area_t* vastack = create_virtual_area(VADDR_MAX - PAGESIZE, PAGESIZE, VA_STACK);
    if(vastack == NULL || vzone == NULL){
        goto free_va;
    }

    insert_virtual_area(vzone,vastack);
    vzone->count ++ ;
    self->vzone = vzone;

    uint32_t user_stack_addr = kalloc_pages(3);
    uint32_t user_page_dir = user_stack_addr + PAGESIZE;
    uint32_t page_table_stack = user_page_dir + PAGESIZE;
    if(user_stack_addr == 0){
        goto free_pg;
    }

    //copy page_dir and map user stack page
    memcopy((void*)user_page_dir, (void*)page_dir, 0x400);// just map the lower 1GB as kernel
    *((uint32_t*)user_page_dir + (vastack->start >> PDESHIFT)) = page_table_stack | PE_U | PE_W | PE_P;
    *((uint32_t*)page_table_stack + ((vastack->start >> PTESHIFT) & 0x3FF)) = user_stack_addr | PE_U | PE_W | PE_P;
    uint32_t code_addr = (uint32_t)user_idle;

    //set user_idle PDE & PTE in user mode, while since the first real user task this mode should be eliminated
    *((uint32_t*)user_page_dir + (code_addr>>PDESHIFT)) |= PE_U;
    *((uint32_t*)(page_dir + PAGESIZE) + (code_addr >> PAGESHIFT)) |= PE_U;
    *((uint32_t*)(page_dir + PAGESIZE) + (code_addr >> PAGESHIFT) + 1) |= PE_U;

    //set and load cr3
    self->page_dir = user_page_dir;
    lcr3(self->page_dir);
    /*kprintln("addr: %x\n",vastack->start);
    kprintln("stack_addr %x\n",user_stack_addr);
    kprintln("pde: %x\n",(vastack->start >> 22));
    kprintln("pte: %x\n",((vastack->start >> 12) & 0x3FF));
    kprintln("\ntest_page: %x\n",*(uint32_t*)0xffeffff0);*/
    //load tss
    tss.esp0 = self->kernel_stack;
    tss.ss0  = SEG_KER_D; //kernel data segment

    //set iret stack
    interrupt_stack_t* is = (interrupt_stack_t*)(self->kernel_stack - sizeof(interrupt_stack_t));
    is->gs = SEG_USR_D | M_USR; //user data segment
    is->ds = is->gs;
    is->fs = is->gs;
    is->es = is->gs;
    is->eip = (uint32_t)user_idle;
    is->cs = SEG_USR_C | M_USR; //user code segment
    is->eflag = self->context.eflags;
    is->tss_esp = vastack->end - sizeof(uint32_t);
    is->tss_ss = is->gs;
    lift_kernel_thread();

    free_pg:
    kfree_pages(user_stack_addr);
    free_va:
    kfree(vastack);
    kfree(vzone);
    while(1){} //just halt if failed
}

int create_kernel_thread(process_t* pcbp, uint32_t function){
    process_t* pcbc = alloc_pcb();
    if(pcbc == NULL){
        goto failed;
    }

    uint32_t kstack_addr = kalloc_pages(1);
    if(kstack_addr == 0){
        goto free_pcb;
    } 

    uint8_t ilock = lock_interrupt();
    pcbc->state = READY;
    list_add(&ready_queue.node,&pcbc->qlink);
    ready_queue.num ++;
    pcbc->pid = ++max_pid;
    pcbc->page_dir = page_dir;
    pcbc->parent = pcbp;
    pcbc->kernel_stack = kstack_addr + PAGESIZE;
    if(pcbp->children_num == 0){
        list_init(&pcbp->children);
        pcbp->children_num ++ ;
    }
    list_add(&pcbp->children,&pcbc->brothers);
    pcbc->context.eip = function;
    pcbc->context.esp = pcbc->kernel_stack - sizeof(interrupt_stack_t) - 3*sizeof(uint32_t);
    *(uint32_t*)(pcbc->context.esp + 2*sizeof(uint32_t)) = (uint32_t)pcbc;//reserve stack space for dummy ebp and eip
    pcbc->context.eflags = IF_BIT;//we need to set IF to enable interrupts when running
    unlock_interrupt(ilock);
    return pcbc->pid;

    free_kstack:
    kfree_pages(kstack_addr);
    free_pcb:
    kfree(pcbc);
    failed:
    return -1;
}

int init_kernel_thread(){
    process_t* pcb = alloc_pcb();
    if(pcb == NULL){
        return -1;
    }

    pcb->state = RUN;
    pcb->kernel_stack = 0x7c00;
    pcb->page_dir = page_dir;
    memmove((void*)pcb->name, "kernel_task0", 12);
    max_pid = 0;
    process_num = 1;

    ready_queue.stat = READY;
    ready_queue.num = 0;
    list_init(&ready_queue.node);
    current = pcb;//set the current running process the kernel thread
    
    return create_kernel_thread(pcb,(uint32_t)kernel_to_user);
}