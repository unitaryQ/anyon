#ifndef __UTILS_H__
#define __UTILS_H__

#include <types.h>

static inline uint8_t inb(uint16_t)__attribute__((always_inline));
static inline void outb(uint16_t, uint8_t)__attribute__((always_inline));

static inline void outb(uint16_t port, uint8_t data){
    //out dx, al => outb %eax, %edx
    asm volatile(
        "outb %0, %1"
        :
        :"a" (data), "Nd"(port) 
    );
}

static inline uint8_t inb(uint16_t port){
    uint8_t data;
    //in al, dx => inb %edx, %eax
    asm volatile(
        "inb %1, %0"
        :"=a"(data)
        :"Nd"(port)
    );

    return data;
}

void memset(void*, uint32_t);
void memmove(void*, void*, uint32_t);
uint32_t clog(uint32_t);

#define ALIGN_UP(data, algn) (((data) % (algn)) == 0? (data) : ((data) / (algn)) * (algn) + (algn))
#define ALIGN_DOWN(data, algn) ((data) / (algn) * (algn))
#endif