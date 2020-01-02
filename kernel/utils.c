#include <utils.h>

void memset(void* dst, uint32_t size){
    uint8_t* p =  (uint8_t*) dst;
    int i;
    for(i = 0; i < size ; i++){
        *(p + i) = 0x0;
    }
}

//move a piece of memory
void memmove(void* dst, void* src, uint32_t size){
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

uint32_t clog(uint32_t x){
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
