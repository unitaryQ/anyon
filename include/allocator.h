#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

#include <types.h>
#include <ram.h>

typedef struct slab_list_s{
    list_t partial_list;
    list_t full_list;
    uint32_t free_object_num;
    uint32_t full_slab_num;
}slab_list_t;

typedef struct ram_cache_s{
    list_t   cache_list; 
    uint32_t flags;         //reserved flags for page reclaim or else
    uint32_t slab_size;     //how many pages in a slab, only be 1,2,4...
    uint32_t object_size;   //cached object size in byte
    slab_list_t slab_list;
}ram_cache_t;

typedef struct slab_s{
    uint32_t fst_slice;
    uint32_t slice_off;
    uint32_t slice_num;
    ram_cache_t* cache;
    uint32_t bitmap[];
}slab_t;

//cache0 is the cache of struct ram_cache_s
static ram_cache_t cache0;
static int cache_num;

#define SLAB_UNIT 0x20
#define SLAB_MAX  0x400
#define DEFAULT_SLAB_PAGE 0x01
#define ALLOC_MAGIC 0xffffffff

void init_allocator();
void* kalloc(uint32_t);
int kfree(void*);
#endif