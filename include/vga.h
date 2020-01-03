#ifndef __VGA_H__
#define __VGA_H__

#define VGA     0xb8000
#define VGA_H   25 //emmm, upside down...
#define VGA_V   80 

void init_vga();
void vga_putc(char);
#endif