#include <utils.h>
#include <vga.h>
#include <stdio.h>

void __vga_putc(char c){
    uint16_t* p = (uint16_t*)VGA + cur_x + cur_y*80;
    *p = (uint16_t)(c) + (0x0f << 8);
}

void vga_set_cur(uint16_t x, uint16_t y){
    
    uint16_t pos = x + y*80;
    outb(0x3d4,14);
    outb(0x3d5,(uint8_t)(pos >> 8));
    outb(0x3d4,15);
    outb(0x3d5,(uint8_t)(pos & 0xFF));

    cur_x = x;
    cur_y = y;
}

void vga_clear(){
    uint16_t* pvideo = (uint16_t*)VGA;
    uint32_t i;
    uint16_t c = ' ' + (0x0f << 8);

    for(i=0;i<VGA_V*VGA_H;i++){
        *(pvideo+i) = c;
    }
}

void vga_down_1L(){
    uint16_t* pvideo = (uint16_t*)VGA;
    uint16_t* pvideo_L1 = pvideo + VGA_V;
    memmove((void*)pvideo,(void*)pvideo_L1,(VGA_H-1)*VGA_V*sizeof(uint16_t));

    int i;
    for(i = (VGA_H-1)*VGA_V; i<VGA_V*VGA_H; i++){
        *(pvideo + i) = ' ' + (0x0f << 8);
    }
}

void vga_putc(char c){

    switch (c) {
        case '\n':{
            cur_x = 0;
            cur_y ++;
            if(cur_y >= VGA_H){
                cur_y = VGA_H-1;
                 vga_down_1L();
            }
            break;
        }
        case '\r': {
            cur_x = 0;
            break;
        }
        case '\b':{
            if(cur_x > 0){
                cur_x --;
                __vga_putc(' ');
            }
            break;
        }
        case '\t':{
           c = ' ';
        }

        default: {
            if(cur_x == VGA_V-1){
                __vga_putc(c);
                            
                cur_x = 0;
                cur_y ++;
                if(cur_y >= VGA_H){
                    cur_y = VGA_H -1;
                    vga_down_1L();
                }
            }
            else{
                __vga_putc(c);
                cur_x ++ ;
            }
        }
    }

    vga_set_cur(cur_x,cur_y);
}

void init_vga(){
    vga_clear();
    vga_set_cur(0,0);
}