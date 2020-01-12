#include <interrupt.h>
#include <stdio.h>
#include <utils.h>
#include <keyboard.h>
#include <clock.h>

//idt table with 256 entries
static idt_entry_t idt[IDT_NUM];
static idt_entry_t gdt[GDT_NUM];
static idt_info_t idt_i;
static idt_info_t gdt_i;

static inline void lidt(uint32_t)__attribute__((always_inline));
static inline void lgdt(uint32_t)__attribute__((always_inline));
static inline void ltr(uint16_t) __attribute__((always_inline));

static inline void lidt(uint32_t addr){
    asm volatile(
        "lidt (%%eax)"
        :
        :"a"(addr)
    );
}

static inline void lgdt(uint32_t addr){
    asm volatile(
        "lgdt (%%eax)"
        :
        :"a"(addr)
    );
}

static inline void ltr(uint16_t index){
    asm volatile(
        "ltr %0"
        :
        :"r"(index)
    );
}

void make_idt_entry(idt_entry_t* entry, uint32_t handler, uint8_t DPL, uint8_t P, uint8_t type){
    entry->lo = (SEG_SEL << 16) + (handler & 0xFFFF);
    entry->hi = (handler & 0xFFFF0000) + ((P & 0x1) << 15) + ((DPL & (0x3)) << 13) + ((type & 0xF) << 8);
}

void make_gdt_entry(idt_entry_t* entry, uint32_t base, uint32_t limit,uint8_t access_byte, uint8_t flag){
    entry->lo = ((base & 0xFFFF) << 16) + (limit & 0xFFFF);
    entry->hi = (base & 0xFF000000) + ((base & 0xFF0000) >> 16) + (limit & 0xF0000) + (access_byte << 8) + ((0xF & flag) << 20);
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
    
    //gdt
    memset(gdt,sizeof(idt_entry_t)*GDT_NUM);
    make_gdt_entry(&gdt[1],0,0xFFFFF,0x9a,0xc);//kernel code
    make_gdt_entry(&gdt[2],0,0xFFFFF,0x92,0xc);//kernel data
    make_gdt_entry(&gdt[3],0,0xFFFFF,0xfa,0xc);//user code
    make_gdt_entry(&gdt[4],0,0xFFFFF,0xf2,0xc);//user data
    make_gdt_entry(&gdt[5],(uint32_t)(&tss),sizeof(tss_t),0x89,0x4); //tss

    gdt_i.len = GDT_NUM*sizeof(idt_entry_t);
    gdt_i.off = (uint32_t)(&gdt[0]);
    lgdt((uint32_t)(&gdt_i));

    //load tss
    ltr(SEG_TSS);
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
            kprintln("unknown interrupt %d code %d\n",c,is->err_code);
            while(1){}
            break;
        }
    }
}