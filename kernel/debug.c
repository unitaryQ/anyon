#include <debug.h>
#include <stdio.h>

uint32_t get_ebp(){
    uint32_t val_ebp;
    asm (
        "movl %%ebp, %0"
        :"=r"(val_ebp)
        :
    );
    
    return val_ebp;
}

/*
STACK
|args ...
|return point
|old caller ebp -> old caller stack
-- new callee stack-- <- new callee ebp
|objs ...
*/
void kstack(){
    kprintln("-----KERNEL-STACK-----\n");
    uint32_t ebp = get_ebp();
    uint32_t caller;
    uint32_t caller_ebp;

    // the bootloader's ebp is 0x00000000
    while(ebp > 0){
        caller = *((uint32_t*)(ebp + 0x04));
        caller_ebp = *((uint32_t*)(ebp + 0x0));
        ebp = caller_ebp;
        kprintln("ebp=%x  caller=%x\n",ebp,caller);
    }
    kprintln("----------------------\n");
}