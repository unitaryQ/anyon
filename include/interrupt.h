#include <types.h>

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#define SEG_SEL 0x08
#define T_TRAP 0x0F
#define T_INTR 0x0E 
#define IDT_NUM 0x100
#define GDT_NUM 0x05
#define M_USR 0x03
#define M_KER 0x00

typedef struct idt_entry_s{
    uint32_t lo;
    uint32_t hi;
}idt_entry_t;

typedef struct idt_info_s{
    uint16_t len;
    uint32_t off;
}__attribute__((packed))idt_info_t;

//stack layout of interrupt handler
typedef struct interrupt_stack_s{
    uint32_t gs;
    uint32_t ds;
    uint32_t fs;
    uint32_t es;
    uint32_t eflags[8];
    uint32_t int_code;
    uint32_t err_code;
    uint32_t cs;
    uint32_t eip;
    uint32_t tss_ss;
    uint32_t tss_esp;
}interrupt_stack_t;

//idt table with 256 entries
static idt_entry_t idt[IDT_NUM];
static idt_entry_t gdt[GDT_NUM];
static idt_info_t idt_i;
static idt_info_t gdt_i;
static inline void sti()__attribute__((always_inline));
static inline void cli()__attribute__((always_inline));

static inline void sti(){
    asm volatile("sti");
}

static inline void cli(){
    asm volatile("cli");
}

extern uint32_t intr_lst[]; // uint** intr not work!

void init_idt();
extern void common_handler(interrupt_stack_t*);
#endif 