#ifndef __RAM_H__
#define __RAM_H__

#include <types.h>

typedef struct ram_layout_entry_s{
    uint32_t base_lo;
    uint32_t base_hi;
    uint32_t length_lo;
    uint32_t length_hi;
    uint32_t attribute;
    uint32_t apic;
}ram_layout_entry_t;

typedef struct ram_layout_info_s{
    uint32_t num;
    ram_layout_entry_t rl[];
}ram_layout_info_t;

#define PAGESIZE    0x1000
#define PAGESHIFT   0x0c
#define MAXORDER    0x5
#define MAXBUDDY (1<<MAXORDER)

#define PG_RESERVED     0x1
#define PG_BUDDYH       0x100   //is this page a buddy block head?
#define PG_BUDDY_USED   0x200   //is this buddy allocated?
#define PG_SLAB_USED    0x400   //is this a slab page?

// page descriptor
// in buddy system, link is hang to buddy descriptor as first free page
// in slab, link is hang to slab cache descriptor
// in a user process, link is hang to swap list for swapping
typedef struct page_s{
    list_t   link;   
    uint32_t flags; //reserved or dynamically allocated or something
    uint32_t index; // index of page data in ram or disk or swap
    uint32_t ref;   //reference counter
    uint32_t order; //buddy order
}page_t;

typedef struct buddy_s{
    list_t   free_list;
    uint32_t order;
    uint32_t free_num;
}buddy_t;

//anyon only supports one ram zone, where a buddy system runs
typedef struct zone_s{
    uint32_t fst_page;
    uint32_t page_num;
    buddy_t  buddy_array[MAXORDER+1];
}zone_t;

zone_t global_zone;
page_t* page_map;

page_t* alloc_pages(uint32_t, zone_t*);
int free_pages(page_t*,zone_t*);
int init_ram(ram_layout_info_t*,uint32_t);
void ram_print();
void set_page_flags(page_t*, uint32_t);
void clear_page_flags(page_t*, uint32_t);

#define page_addr(page) ((((uint32_t)(page) - (uint32_t)page_map) / sizeof(page_t)) << PAGESHIFT)
#endif