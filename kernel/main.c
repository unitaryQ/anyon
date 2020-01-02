#include <types.h>
#include <stdio.h>
#include <debug.h>
#include <utils.h>
#include <vga.h>
#include <interrupt.h>
#include <keyboard.h>
#include <pic.h>
#include <clock.h>
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
    
    virtual_zone_t* vz = create_virtual_zone();
    virtual_area_t* va1 = create_virtual_area(0x0,0x40000000,VA_EXEC);
    //kprintln("%p &",vz);
    virtual_area_t* vac = create_virtual_area(0x40000000,0x1000,VA_EXEC);
    virtual_area_t* vad = create_virtual_area(0x40001000,0x1000,VA_DATA);
    virtual_area_t* vas = create_virtual_area(0xFFE00000,0x1000,VA_STACK);
    //kprintln("%x\n",vas);
    kprintln("%d\n",insert_virtual_area(vz,va1));
    kprintln("%d\n",insert_virtual_area(vz,vac));
    kprintln("%d\n",insert_virtual_area(vz,vad));
    kprintln("%d\n",insert_virtual_area(vz,vas));
    virtual_area_t* test = create_virtual_area(0x40000000,0x40010000,VA_STACK);
    kprintln("%d\n",insert_virtual_area(vz,test));
    kprintln("start = %x\n",find_virtual_area(vz,0x40001FF0)->start);
    kprintln("va_num = %d",vz->va_num);

    //void* p = kalloc(30);
    //kprintln("%p\n",p);
    khalt();
}