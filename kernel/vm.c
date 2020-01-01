#include <vm.h>
#include <utils.h>
#include <stdio.h>
#include <ram.h>

void enable_paging(uint32_t pd){
    asm volatile(
        "movl %0, %%cr3"
        :
        :"a"(pd)
    );

    asm volatile("movl %cr0, %eax");
    asm volatile("orl $0x80000001, %eax");
    asm volatile("movl %eax, %cr0");
}

/*anyon map kernel to low address, just do identity mapping*/
void init_paging(){

    int i;
    for(i = 0; i < page_table_num; i++){
        uint32_t ptaddr = page_dir + PAGESIZE*(i+1);
        *((uint32_t*)page_dir + i) = ptaddr + 0x03;
    }

    uint32_t* fst_pt = (uint32_t*)(page_dir + PAGESIZE);
    for(i = 0; i< total_page; i++){
        *(fst_pt + i) = (i<<PAGESHIFT) + 0x03;
    }

    enable_paging((uint32_t)page_dir);
}