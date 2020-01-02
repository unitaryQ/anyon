#include <vm.h>
#include <allocator.h>
#include <utils.h>
#include <stdio.h>
#include <lock.h>

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

virtual_zone_t* create_virtual_zone(){
    virtual_zone_t* vz = (virtual_zone_t*)kalloc(sizeof(virtual_zone_t));
    list_init(&vz->va_list);
    vz->va_num = 0;
    return vz;//maybe NULL
}

virtual_area_t* create_virtual_area(uint32_t beg, uint32_t len, uint32_t flags){

    if(beg >= VADDR_MAX || (VADDR_MAX - beg < len) || len == 0){
        return NULL;
    }

    virtual_area_t* va = (virtual_area_t*)kalloc(sizeof(virtual_area_t));
    va->start = beg;
    va->length = len;
    va->end = beg + len;
    va->flags = flags;
    list_init(&va->link);
    return va; //maybe NULL
}

/*mainly for inserting a va during process creation*/
int insert_virtual_area(virtual_zone_t* vzone, virtual_area_t* varea){
    if(vzone->va_num == 0){
        list_add(&vzone->va_list,&varea->link);
        vzone->va_num ++ ;
        return 0;
    }

    uint32_t i;
    list_t* lp = vzone->va_list.next;
    for(i = 0; i < vzone->va_num; i++){
        virtual_area_t* va = to_struct(lp,virtual_area_t,link);
        if(va->end <= varea->start){
            //check overlapping
            if(i < vzone->va_num - 1){
                if((to_struct((lp->next),virtual_area_t,link))->start >= varea->end){
                    goto inserted;
                }
            }else{
                goto inserted;
            }
        }
        lp = lp->next;
    }
    return -1;

    inserted:
    list_add(lp,&varea->link);
    vzone->va_num ++;
    return 0;
}

/*mainly for handling with page fault*/
virtual_area_t* find_virtual_area(virtual_zone_t* vzone, uint32_t vaddr){
    
    list_t* lva = vzone->va_list.next; 
    uint32_t i;
    for(i = 0; i < vzone->va_num; i++){
        virtual_area_t* tmpva = to_struct(lva,virtual_area_t,link);
        if(vaddr >= tmpva->start && tmpva->end > vaddr){
            return tmpva;
        }
        lva = lva->next;
    }

    return NULL;
}
