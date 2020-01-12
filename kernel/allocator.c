#include <allocator.h>
#include <list.h>
#include <utils.h>
#include <stdio.h>
#include <lock.h>

//cache0 is the cache of struct ram_cache_s
static ram_cache_t cache0;
static int cache_num;

void default_init_cache(ram_cache_t* rc){
    list_init(&rc->cache_list);
    list_init(&rc->slab_list.partial_list);
    list_init(&rc->slab_list.full_list);
    rc->slab_size = DEFAULT_SLAB_PAGE;
    rc->slab_list.free_object_num = 0;
    rc->slab_list.full_slab_num = 0;
}

/*a much reduced sl*b allocator*/
void init_allocator(){
    cache_num = 1;
    cache0.object_size = sizeof(ram_cache_t) <= SLAB_UNIT ? SLAB_UNIT: (1<<clog(sizeof(ram_cache_t)));
    default_init_cache(&cache0);
}

ram_cache_t* create_new_cache(uint32_t new_size){
    ram_cache_t* rc = (ram_cache_t*)kalloc(sizeof(ram_cache_t));
    rc->object_size = new_size;
    default_init_cache(rc);
    list_add(cache0.cache_list.prev,&rc->cache_list);
    cache_num ++ ;
    return rc;
}

//only alloc memory for kernel objects
//we have already have a sizeof(ram_cache_t) cache at the beginning
void* kalloc(uint32_t size){

    if(size > SLAB_MAX || size == 0){
        return NULL;
    }
    
    uint8_t ilock = lock_interrupt();

    uint32_t fit_size = size > SLAB_UNIT ? (1<<clog(size)) : SLAB_UNIT; 

    //find a fit cache
    list_t* lc = &cache0.cache_list;
    int i = cache_num;
    while(i > 0){
        ram_cache_t* rc = to_struct(lc,ram_cache_t,cache_list);
        if(rc->object_size == fit_size){
            goto find_fit_slab;
        }
        lc = lc -> next;
        i -- ;
    }

    //create a new cache, which has already been in cache list
    ram_cache_t* newrc = create_new_cache(fit_size);
    lc = &newrc->cache_list;
    if(newrc == NULL){
        unlock_interrupt(ilock);
        return NULL;
    }
    
    find_fit_slab:
    lc->next = lc -> next; //just nop for label
    ram_cache_t* fitcache = to_struct(lc,ram_cache_t,cache_list);

    //allocate continuous pages for a new slab if no free memory for new objects
    if(fitcache->slab_list.free_object_num == 0){
        page_t* slabpage = alloc_pages(fitcache->slab_size,&global_zone);
        if(slabpage == NULL){
            unlock_interrupt(ilock);
            return NULL;
        }

        set_page_flags(slabpage,PG_SLAB_USED);
        list_add(&fitcache->slab_list.partial_list,&slabpage->link);

        //create the slab descriptor structure
        slab_t* slabaddr = (slab_t*)page_addr(slabpage);
        uint32_t slice_num = (fitcache->slab_size << PAGESHIFT)/fit_size;
        uint32_t header_len = slice_num * sizeof(uint32_t) + sizeof(slab_t);
        slabaddr->fst_slice = (ALIGN_UP(header_len, fit_size)) / fit_size;
        slabaddr->slice_off = slabaddr->fst_slice;
        slabaddr->slice_num = slice_num - slabaddr->fst_slice;
        slabaddr->cache = fitcache;

        int i;
        for(i = 0;i < slice_num; i++){
            if(i>= slabaddr->fst_slice && i < slice_num - 1){
                slabaddr->bitmap[i] = i+1;
            }else{
                slabaddr->bitmap[i] = 0x0;
            }
        }

        fitcache->slab_list.free_object_num += slice_num - slabaddr->fst_slice;
    }

    list_t* ls = fitcache->slab_list.partial_list.next;
    page_t* ps = to_struct(ls,page_t,link);
    slab_t* slb  = (slab_t*)page_addr(ps);
    void* obj = (void*)((uint32_t)slb + slb->fst_slice*fitcache->object_size);
    uint32_t tmpid = slb->fst_slice;
    slb->fst_slice = slb->bitmap[slb->fst_slice];
    slb->bitmap[tmpid] = ALLOC_MAGIC;
    ps->ref ++;
    fitcache->slab_list.free_object_num -- ;

    if(slb->fst_slice == 0){ //this slab is full
        list_del(ls);
        list_add(&fitcache->slab_list.full_list,ls);
        fitcache->slab_list.full_slab_num ++ ;
    }

    unlock_interrupt(ilock);
    return obj;
}

int kfree(void* object){

    if(object == NULL){
        return -1;
    }

    uint32_t objpaddr = ALIGN_DOWN((uint32_t)object,PAGESIZE);
    uint8_t ilock = lock_interrupt();

    //default page size is 1, just check if this page is for slab
    page_t* objpage = page_map + (objpaddr>>PAGESHIFT);
    if(objpage->flags & PG_SLAB_USED == 0){
        unlock_interrupt(ilock);
        return -1;
    }

    slab_t* slb = (slab_t*)objpaddr;
    ram_cache_t* cache = slb->cache;
    uint32_t off = (uint32_t)object - (uint32_t)slb;
    uint32_t offid = off / cache->object_size;
    if(off % cache->object_size != 0 || offid < slb->slice_off || slb->bitmap[offid] != ALLOC_MAGIC){
        // not aligned or aligned to slab header or the obj is free. 
        unlock_interrupt(ilock);
        return -1;
    }

    cache->slab_list.free_object_num ++ ;
    objpage->ref --;
    if(slb->fst_slice == 0){ // full slab
        slb->fst_slice = offid;
        slb->bitmap[offid] = 0x0;
        cache->slab_list.full_slab_num--;
        list_del(&objpage->link);
        list_add(&cache->slab_list.partial_list,&objpage->link);
    }else{
        slb->bitmap[offid] = slb->fst_slice;
        slb->fst_slice = offid;
    }

    unlock_interrupt(ilock);
    return 0;
}

uint32_t kalloc_pages(uint32_t num){

    uint8_t ilock = lock_interrupt();
    page_t* p = alloc_pages(num,&global_zone);
    if(p == NULL){
        unlock_interrupt(ilock);
        return 0;
    }
    p->ref ++ ;
    unlock_interrupt(ilock);
    return (page_addr(p));
}

int kfree_pages(uint32_t addr){

    if((addr & 0xFFF) != 0 || addr == 0){
        return -1;
    }
    
    page_t* p = page_map + (addr >> PAGESHIFT);
    uint8_t ilock = lock_interrupt();
    p->ref --;
    int ret = free_pages(p,&global_zone);
    if(ret == -1){
        p->ref ++;
    }
    unlock_interrupt(ilock);
    return ret;
}
