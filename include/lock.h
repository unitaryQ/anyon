#ifndef __LOCK_H__
#define __LOCK_H__ 

#include <interrupt.h>
#define IF_BIT 0x0200

//return 1 means need reset IF, 0 means need not
static inline uint8_t lock_interrupt(){
    uint32_t eflags;
    asm volatile(
        "pushfl; popl %0"
        :"=a"(eflags)
        :
    );

    if((eflags & IF_BIT) == 0){
        // interrupts off
        return 0;
    }else{
        //interrupts on
        cli();
        return 1;
    }
}

static inline void unlock_interrupt(uint8_t IF){
    if(IF == 1){
        sti();
    }
}

#endif 