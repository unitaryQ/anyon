#ifndef __VM_H__
#define __VM_H__

#include <types.h>
#include <list.h>

#define VA_EXEC     0x01
#define VA_DATA     0x02
#define VA_STACK    0x08
#define VA_DYNAMIC  0x10
#define VA_SHARE    0x20
#define VADDR_MAX   0xFFF00000

#define PE_U 0x04
#define PE_W 0x02
#define PE_P 0x01

#define PDESHIFT 22
#define PTESHIFT 12

typedef struct virtual_area_s{
    list_t link;
    uint32_t start;
    uint32_t end;
    uint32_t length;
    uint32_t flags; //code, data, stack, heap or some advance flags(shared...)
}virtual_area_t;

typedef struct virtual_zone_s{
    list_t va_list; // init node of virtual area list
    uint32_t va_num;
    uint32_t count;
}virtual_zone_t;

static inline void lcr3(uint32_t)__attribute__((always_inline));
static inline void lcr3(uint32_t addr){
    asm volatile(
        "movl %0, %%cr3"
        :
        :"a"(addr)
    );
}

void init_paging();
virtual_zone_t* create_virtual_zone();
virtual_area_t* create_virtual_area(uint32_t, uint32_t, uint32_t);
int insert_virtual_area(virtual_zone_t*, virtual_area_t*);
virtual_area_t* find_virtual_area(virtual_zone_t*, uint32_t);
#endif