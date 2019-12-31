#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <utils.h>

#define KEY_BUF_LEN 0x400

typedef struct key_buf_s{
    uint32_t head;
    uint32_t tail; 
    uint32_t relative;
}key_buf_t;


static key_buf_t keybuf;

#define P_ESC   0x01
#define P_LCTRL 0x1D
#define P_LSHIFT 0x2A 
#define P_RSHIFT 0x36
#define P_LALT  0x38
#define P_CAPLOCK 0x3A
#define P_NUMLOCK 0x45
#define P_SCROLLLOCK 0x46

//keyboard for US-map
static uint8_t key_map[0x80] = {
     0,  0, '1','2','3','4','5','6','7','8','9','0','-','=','\b','\t', //0xF
    'q','w','e','r','t','y','u','i','o','p','[',']','\n',0, 'a', 's', //0x1F
    'd','f','g','h','j','k','l',';','\'','`',0, '\\','z','x','c','v', //0x2F
    'b','n','m',',','.','/', 0,  0,  0,' ',  0,  0,  0,  0,   0,  0, //0x3F
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0, //0x4F
};

static uint8_t key_shift_map[0x80] = {
     0,  0, '!','@','#','$','%','^','&','*','(',')','_','+','\b','\t', //0xF
    'q','w','e','r','t','y','u','i','o','p','{','}','\n',0, 'a', 's', //0x1F
    'd','f','g','h','j','k','l',';','\"','`',0, '|','z','x','c','v', //0x2F
    'b','n','m','<','>','?', 0,  0,  0,' ',  0,  0,  0,  0,   0,  0, //0x3F
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0, //0x4F
};

//logical circular queue for saving keys
//if overlapped, ignore it and just read
static uint8_t cq[KEY_BUF_LEN]; 

void init_kbd();
void kbd_handler();
uint8_t kbd_read();
#endif