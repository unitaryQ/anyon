#ifndef __PIC_H__
#define __PIC_H__

#define PIC1_1 0x20 //port
#define PIC1_2 0x21
#define PIC2_1 0xa0
#define PIC2_2 0xa1

#define INIT_W 0x11 //init command Without ICW4
#define MODE_A 0x03 //mode with Auto EOI

#define OFF1_I 0x20 //offset in idt
#define OFF2_I 0x28
#define OFF2_1 0x02 //slave offset 

#include <utils.h>

void flip_irq_mask(uint8_t);
void init_pic();
#endif