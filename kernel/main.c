#include <stdio.h>
#include <debug.h>
#include <vga.h>
#include <keyboard.h>
#include <clock.h>
#include <pic.h>
#include <interrupt.h>
#include <allocator.h>
#include <vm.h>

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
    init_allocator();
    init_paging();
    sti();

    khalt();
}