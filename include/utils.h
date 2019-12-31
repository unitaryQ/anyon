#ifndef __UTILS_H__
#define __UTILS_H__

#include <types.h>

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

//set a piece of memory to zero
static void memset(void* dst, uint32_t size){
    uint8_t* p =  (uint8_t*) dst;
    int i;
    for(i = 0; i < size ; i++){
        *(p + i) = 0x0;
    }
}

//move a piece of memory
static void memmove(void* dst, void* src, uint32_t size){
    uint8_t* p =  (uint8_t*) dst;
    uint8_t* q =  (uint8_t*) src;
    
    if ((p == q) || (size == 0)){
    }
    // dst lower than src, move from the beginning
    else if (p < q){ 
        int i;
        for (i = 0; i < size; i++){
            *(p + i) = *(q + i);
            *(q + i) = 0x0;
        }
    }
    // dst higher than src, move from the end
    else{
        int i;
        p = p + size - 1;
        q = q + size - 1;
        for (i = 0; i < size; i++){
            *(p - i) = *(q - i);
            *(q - i) = 0x0;
        }
    }
}

#define ALIGN_UP(data, algn) (((data) % (algn)) == 0? (data) : ((data) / (algn)) * (algn) + (algn))
#define ALIGN_DOWN(data, algn) ((data) / (algn) * (algn))

static uint32_t clog(uint32_t x){
    if(x > 0 && x <= (1<<31)){
        uint32_t y = 1;
        uint32_t ord = 0;
        while(x > y){
            y = y << 1;
            ord ++ ;
        }
        return ord;
    }
    return 0;
}

#endif