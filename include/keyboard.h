#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <types.h>

#define KEY_BUF_LEN 0x400

typedef struct key_buf_s{
    uint32_t head;
    uint32_t tail; 
    uint32_t relative;
}key_buf_t;

typedef struct kbd_flag_s{
    uint8_t _shift;
    uint8_t _cap;
    uint8_t _E0;
}kbd_flag_t;

#define P_ESC   0x01
#define P_LCTRL 0x1D
#define P_LSHIFT 0x2A 
#define P_RSHIFT 0x36
#define P_LALT  0x38
#define P_CAPLOCK 0x3A
#define P_NUMLOCK 0x45
#define P_SCROLLLOCK 0x46

void init_kbd();
void kbd_handler();
uint8_t kbd_read();
#endif