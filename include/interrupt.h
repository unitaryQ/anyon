#include <types.h>

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#define SEG_SEL 0x08
#define T_TRAP  0x0F
#define T_INTR  0x0E 
#define IDT_NUM 0x100
#define GDT_NUM 0x06
#define M_USR   0x03
#define M_KER   0x00

#define SEG_KER_C 0x08
#define SEG_KER_D 0x10
#define SEG_USR_C 0x18
#define SEG_USR_D 0x20
#define SEG_TSS   0x28

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
    uint32_t eip;
    uint32_t cs;
    uint32_t eflag;
    uint32_t tss_esp;
    uint32_t tss_ss;
}interrupt_stack_t;

typedef struct tss_s{
    uint32_t link;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t reserved[0x17];
}tss_t;

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

tss_t tss;
#endif 