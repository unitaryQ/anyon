#include <keyboard.h>
#include <stdio.h>
#include <pic.h>

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
//controlled by keybuf struct
static key_buf_t keybuf;
static uint8_t cq[KEY_BUF_LEN]; 
static kbd_flag_t kbdflg;

// write key to the buf
void kbd_write(uint8_t key){

    cq[keybuf.tail] = key;
    keybuf.tail ++ ;
    if(keybuf.tail == KEY_BUF_LEN){
        keybuf.tail = 0;
    }
    keybuf.relative ++ ;
    kprintln("%c",kbd_read());
}

//read the first key from the buf
uint8_t kbd_read(){

    if(keybuf.relative > 0){
        keybuf.relative --;
        uint8_t c = cq[keybuf.head];
        keybuf.head ++ ;
        if(keybuf.head == KEY_BUF_LEN){
            keybuf.head = 0;
        }

        return c;
    }else{
        return '\0';
    }
}

void init_kbd(){

    memset(&keybuf,sizeof(key_buf_t));
    memset(&kbdflg,sizeof(kbd_flag_t));
    memset(cq,KEY_BUF_LEN);

    flip_irq_mask(0x1);
}

void kbd_handler(){

    uint8_t code = inb(0x60);

    if(code < 0x80){ 
        if(kbdflg._E0 == 1){
            //ignore this key
            kbdflg._E0 = 0;
        }
        else{
            //single key
            uint8_t ch = key_map[code];
            if(ch > 0){
                //a character
                if(ch >= 'a' && ch <= 'z'){
                    if(kbdflg._shift != kbdflg._cap){
                        // capital letters
                        kbd_write(ch - ('a' - 'A'));
                    }else{
                         kbd_write(ch);
                    }
                }else{
                    // other shifted char
                    if(kbdflg._shift == 1){
                        kbd_write(key_shift_map[code]);
                    }else{
                        kbd_write(ch);
                    }
                }
            }else{
                //a functional key
                switch (code){
                    case P_LSHIFT: kbdflg._shift = 1; break;
                    case P_RSHIFT: kbdflg._shift = 1; break;
                    case P_CAPLOCK: kbdflg._cap = (kbdflg._cap + 1) % 2 ;break;
                    default: kprintln("unknown_key_code %x\n",code); break;
                }
            }
        }
    }else{
        // release or 0xE0
        switch (code){
            case 0xE0: kbdflg._E0 = 1; break;
            case (P_LSHIFT + 0x80):kbdflg._shift = 0; break;
            case (P_RSHIFT + 0x80): kbdflg._shift = 0; break;
            default: kbdflg._E0 = 0; break;
        }
    }
}