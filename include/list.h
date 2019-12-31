#ifndef __LIST_H__
#define __LIST_H__

#include <types.h>

#define offsetof(st,m) ((uint8_t*)(&((st*)0) -> m) - (uint8_t*)0)
#define to_struct(list, type, list_name_in_type) ((type*)((uint8_t*)list - offsetof(type,list_name_in_type)))

static inline void list_init(list_t* l){
    l->prev = l;
    l->next = l;
}

static inline void list_add(list_t* l, list_t* node){
    node -> next = l -> next;
    node -> prev = l;
    l -> next = node;
    node -> next -> prev = node;
}

static inline void list_del(list_t* node){
    node -> prev -> next = node -> next;
    node -> next -> prev = node -> prev;
    list_init(node);
}
#endif