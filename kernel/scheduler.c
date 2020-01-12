#include <scheduler.h>
#include <process.h>
#include <interrupt.h>
#include <lock.h>
#include <list.h>
#include <stdio.h>

extern void switch_proc(process_context_t*,process_context_t*);

int schedule(){
    uint8_t ilock = lock_interrupt();
    if(ready_queue.num == 0){
        goto no_scheduling;
    }

    list_t* lp = ready_queue.node.next;
    list_del(lp);
    ready_queue.num -- ;
    process_t* pcb = to_struct(lp,process_t,qlink);
    if(pcb->pid == current->pid){
        goto no_scheduling;
    }

    //if current waiting, it must set state to STOP or else,
    //therefore, we do not need to put it in the ready queue
    //since they are already in other queues
    if(current->state <= READY){
        ready_queue.num ++ ;
        list_add(ready_queue.node.prev,&current->qlink);
    }
    process_t* from = current;
    current = pcb;
    lcr3(current->page_dir);
    tss.esp0 = current->kernel_stack;
    kprintln("switch from %x -> %x\n",from->pid, current->pid);
    switch_proc(&from->context, &current->context);

    no_scheduling:
    unlock_interrupt(ilock);
    return 0;
}