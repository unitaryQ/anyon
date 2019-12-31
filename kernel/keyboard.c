#include <keyboard.h>
#include <stdio.h>
#include <pic.h>

typedef struct kbd_flag_s{
    uint8_t _shift;
    uint8_t _cap;
    uint8_t _E0;
}kbd_flag_t;

kbd_flag_t kbdflg;

// write key to the buf
// when having TTY, the key should be put to IO file attached to it
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