#include <ram.h>
#include <utils.h>
#include <stdio.h>
#include <list.h>

void get_usable_ram(ram_layout_info_t* rli, ram_layout_entry_t* zone){
    
    uint32_t num = rli->num;
    ram_layout_entry_t* rlt = rli->rl;
    uint8_t is_find = 0; 

    uint32_t i;
    for (i = 0; i < num; i++){
        uint32_t base = rlt->base_lo +(rlt->base_hi<<16);
        uint32_t length  = rlt->length_lo + (rlt->length_hi<<16);
        uint32_t end = base + length;
        uint32_t attr = rlt->attribute;
        length = length >> 10;

        //find the FIRST usable zone above 1M, better to check length as well
        if(base >= 0x100000 && attr ==1 && is_find == 0){
            is_find == 1;
            zone -> base_lo = base; 
            zone -> length_lo = length >> 2; // page number
        }

        if(zone->base_hi < end){
            zone->base_hi = end;
        }

        if(length >= 0x400){
            length = length >> 10;
            kprintln("base=%x end=%x length=%dM attr=%x\n",base,end,length,attr);
            rlt ++;
            continue;
        }
        kprintln("base=%x end=%x length=%dK attr=%x\n",base,end,length,attr);
        rlt ++ ;
    }
}

void set_page_flags(page_t* p, uint32_t f){
    p->flags |= f;
}
void clear_page_flags(page_t* p, uint32_t f){
    p->flags &= ~f;
}

int init_ram(ram_layout_info_t* rli, uint32_t ktail){

    // align kernel tail to the following page
    ktail = ALIGN_UP(ktail,PAGESIZE);
    page_map = (page_t*)(ktail); 

    ram_layout_entry_t zone = {0};
    get_usable_ram(rli, &zone);

    uint32_t ram_size = zone.base_hi;
    uint32_t zone_size = zone.length_lo; //free page number
    uint32_t zone_base = (ALIGN_UP(zone.base_lo,PAGESIZE)) / PAGESIZE; 
    uint32_t page_num = ram_size / PAGESIZE; 

    //check usable zone length, maybe more strict 
    if(zone_size == 0){
        goto failed;
    }

    ktail = (ALIGN_UP(ktail + page_num*sizeof(page_t),PAGESIZE));//update kernel tail
    ktail = ktail >> PAGESHIFT;
    memset(page_map, sizeof(page_t)*page_num);

    goto okay;

    failed:
    kprintln("ram init failed\n");
    return -1;

    okay:
    global_zone.fst_page = zone_base;
    global_zone.page_num = ALIGN_DOWN(zone_base + zone_size, MAXBUDDY);//align to max buddy

    uint8_t ord;
    for(ord = 0; ord <= MAXORDER; ord++){
        global_zone.buddy_array[ord].order = ord;
        global_zone.buddy_array[ord].free_num = 0;
        list_init(&global_zone.buddy_array[ord].free_list);
    }

    uint32_t max_buddy_num = global_zone.page_num / MAXBUDDY;
    uint32_t kernel_res_buddy = ktail > global_zone.fst_page ? (ALIGN_UP(ktail - global_zone.fst_page,MAXBUDDY))/MAXBUDDY : 0;
    global_zone.buddy_array[MAXORDER].free_num = max_buddy_num - kernel_res_buddy;

    //initialize every page in ram, if outside zone or in overlapped area of kernel and zone, set reserved
    uint32_t i;
    for (i = 0; i < page_num; i ++){
        if(i < zone_base + (kernel_res_buddy << MAXORDER) || i >= (zone_base + zone_size)){
            set_page_flags(&page_map[i],PG_RESERVED);
        }
    }
    
    list_t* pl = &global_zone.buddy_array[MAXORDER].free_list;
    for(i = kernel_res_buddy; i < max_buddy_num; i++){
        page_t* p = &page_map[global_zone.fst_page + (i << MAXORDER)];
        set_page_flags(p,PG_BUDDYH); // marked as a buddy block head
        p->order = MAXORDER;
        list_add(pl,&p->link); // add to free buddy list
        pl = pl -> next;
    }

    kprintln("buddy system managed zone: first_page=%x total_page=%x order=%d\n",
                global_zone.fst_page,global_zone.page_num,MAXORDER);

    return 0;
}

page_t* alloc_pages(uint32_t num, zone_t* zone){
    
    if(num > MAXBUDDY || num == 0){
        return NULL;
    }

    uint8_t order = 0;
    while(1){
        if(num <= (1<<order)){
            break;
        }
        order ++ ;
    }

    uint32_t order0 = order;
    while(order <= MAXORDER){
        uint32_t free_num = zone->buddy_array[order].free_num;
        if(free_num > 0){
            zone->buddy_array[order].free_num --;
            list_t* lblk = zone->buddy_array[order].free_list.next;
            list_del(lblk);
            page_t* p = to_struct(lblk,page_t,link);
            set_page_flags(p,PG_BUDDYH | PG_BUDDY_USED);
            if(order0 == order){
                return p;
            }else{
                int d;
                for (d = (int)(order -1); d >= (int)(order0); d --){
                    zone->buddy_array[d].free_num ++ ;
                    page_t* p1 = p + (1<<d);
                    p1->order = d;
                    set_page_flags(p1,PG_BUDDYH);
                    list_add(&zone->buddy_array[d].free_list,&p1->link);
                }
                p -> order = order0;
                return p;
            }
        }
        order ++ ;
    }

    // cannot find a continuous physical ram 
    if(order > MAXORDER){
        return NULL;
    }
}


int free_pages(page_t* fp,zone_t* zone){
    
    //check if this is a buddy's first page
    if((fp->flags & PG_BUDDYH) == 0){
        return -1;
    }

    //check if each page is not referred or reserved
    uint32_t pn = 1 << fp->order;
    uint32_t i;
    for(i = 0; i < pn; i++){
        page_t* p = fp + i;
        if(p->ref > 0 || (p->flags & PG_RESERVED) != 0){
            return -1;
        }
    }

    clear_page_flags(fp,PG_BUDDY_USED);

    page_t* buddy_base = page_map + ALIGN_DOWN(((uint32_t)fp - (uint32_t)page_map)/sizeof(page_t),MAXBUDDY);
    uint32_t order = fp->order;
    while(order < MAXORDER){
        page_t* pb = buddy_base + ((((uint32_t)fp - (uint32_t)buddy_base)/sizeof(page_t)) ^ (1<<order));
        //check if the buddy pb is in used
        if((pb->flags & PG_BUDDY_USED) == 0){
            list_del(&pb->link);
            zone->buddy_array[order].free_num -- ;
            if(fp > pb){
                page_t* tmp = pb;
                pb = fp;
                fp = tmp;
            }
            clear_page_flags(pb,PG_BUDDYH);
            pb -> order = 0;
            fp -> order ++ ;
            order ++ ;
        }else{
            break;
        }
    }

    list_add(&zone->buddy_array[order].free_list,&fp->link);
    zone->buddy_array[order].free_num ++ ;
    return 0;
}

void ram_print(){
    kprintln("alloc info\n");
    int i;
    for(i = 0;i<=MAXORDER;i++){
        kprintln("\n[order %d]\n",i);
        int j;
        list_t* p = &global_zone.buddy_array[i].free_list;
        int max = global_zone.buddy_array[i].free_num;
        if(max > 10){
            max = 10;
        }
        for(j = 0 ;j < max;j++){
            p = p->next;
            page_t* q = to_struct(p,page_t,link);
            uint32_t a = (uint32_t)(page_map);
            uint32_t b = (uint32_t)(q);
            kprintln("%x ",(b-a)/sizeof(page_t));
        }
    }
}