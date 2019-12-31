#ifndef __TYPES_H__
#define __TYPES_H__

typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;

typedef signed char     int8_t;
typedef signed short    int16_t;
typedef signed int      int32_t;

#define NULL ((void*)0x0) 

typedef struct list_s{
    struct list_s* prev;
    struct list_s* next;
}list_t;
 
#endif