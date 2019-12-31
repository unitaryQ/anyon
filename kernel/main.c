#include <types.h>
#include <stdio.h>
#include <debug.h>
#include <utils.h>
#include <vga.h>
#include <interrupt.h>
#include <keyboard.h>
#include <pic.h>
#include <clock.h>
#include <ram.h>
#include <allocator.h>

void khalt(){
    while(1){
    }
}

void kmain(uint32_t* arg){

    init_vga();
    kstack();
    kprintln("test kprintln:%d=%u %s",-1,-1,"OK\n"); 
    kprintln("kernel tail = %x\n",*arg);
    init_pic();
    init_clk();
    init_kbd();
    init_idt();
    init_ram((ram_layout_info_t*)(*(arg+1)),*arg); 
    sti();
    
    khalt();
}