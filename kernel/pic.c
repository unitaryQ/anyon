#include <pic.h>

void flip_irq_mask(uint8_t index){
    uint16_t port;
    uint8_t  off = index;

    if(index < 8){
        port = PIC1_2;
    }else{
        port = PIC2_2;
        off -= 8;
    }

    uint8_t mask = inb(port);

    if((mask & (1<<off))==0){
       outb(port,mask | (1<<off)); // flip 0 -> 1 
    }else{
        outb(port,mask & (~(1<<off))); //flip 1 -> 0
    }
}

void init_pic(){

    //init pic without ICW4
    outb(PIC1_1,INIT_W); 
    outb(PIC2_1,INIT_W);

    //set pic idt offset
    outb(PIC1_2,OFF1_I);
    outb(PIC2_2,OFF2_I);

    //set slave port in master and vice versa
    outb(PIC1_2,1<<OFF2_1);//_ _ _ _ _ 1 _ _ 
    outb(PIC2_2,OFF2_1);

    //set pic mode
    outb(PIC1_2,MODE_A);
    outb(PIC2_2,MODE_A);
    
    //mask all irqs when initializing
    uint8_t mask1 =  0xff & (~(1<<OFF2_1)); //without slave mask 
    uint8_t mask2 =  0xff; 
    outb(PIC1_2,mask1);
    outb(PIC2_2,mask2);
}