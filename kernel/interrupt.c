#include <interrupt.h>
#include <stdio.h>
#include <utils.h>
#include <keyboard.h>
#include <clock.h>

void make_idt_entry(idt_entry_t* entry, uint32_t handler, uint8_t DPL, uint8_t P, uint8_t type){
    entry->lo = (SEG_SEL << 16) + (handler & 0xFFFF);
    entry->hi = (handler & 0xFFFF0000) + ((P & 0x1) << 15) + ((DPL & (0x3)) << 13) + ((type & 0xF) << 8);
}

static inline void lidt(uint32_t addr){
    asm volatile(
        "lidt (%%eax)"
        :
        :"a"(addr)
    );
}

void init_idt(){
    
    memset(idt, sizeof(idt_entry_t)*IDT_NUM);

    //exceptions and irqs
    int id;
    for (id = 0; id < 48; id ++){
        make_idt_entry(&idt[id],(intr_lst[id]),M_USR,1,T_INTR);
    }

    //syscall
    make_idt_entry(&idt[128],(intr_lst[48]),M_USR,1,T_TRAP);

    idt_i.len = IDT_NUM*sizeof(idt_entry_t);
    idt_i.off = (uint32_t)(&idt[0]);
    lidt((uint32_t)(&idt_i));
}
void common_handler(interrupt_stack_t* is){
    
    uint32_t c = is->int_code;

    switch(c){
        case 0x20:{
            clk_handler();
            break;
        }
        case 0x21:{
            kbd_handler();
            break;
        }
        default:{
            //kprintln("unknown interrupt %d\n",c);
            break;
        }
    }
}