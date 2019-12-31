#include <stdio.h>
#include <debug.h>
#include <vga.h>

char int2char(int a,uint8_t u){
    if(a>=0 && a <=9){
        return '0' + a;
    }
    else{
        if(u){
            return 'A'+(a-0xA);
        }else{
            return 'a'+(a-0xa);
        }
    }
}

void printInteger(uint32_t d, uint8_t isHex, uint8_t ex){
    // d is in unsigned hexdecimal format
    if(isHex){ //unsigned, ex is for upper or lower
        vga_putc('0');
        vga_putc('x');
        if( d == 0){
            vga_putc('0');
        }
        else{
            int i;
            uint8_t fstNZ = 0;
            for(i = 7 ; i >= 0 ; i--){
                char c = int2char((0xF & (d >>(4*i))),ex);
                if ( c != '0'){
                    fstNZ = 1;
                }
                if (fstNZ == 1){
                    vga_putc(c);
                }
            }
        }
    }
    else{ // decimal, ex is for signed or unsigned
        if(d == 0){
            vga_putc('0');
        }
        else{
            uint32_t abs = d;
            if((d & 0x80000000) > 0 && ex == 1){
                vga_putc('-');
                abs = 0 - d;
            }
            int i;
            uint8_t fstNZ = 0;
            int mod = 1000000000;
            for (i = 9; i >= 0; i--){
                int digit = abs/mod;
                if(digit != 0){
                    fstNZ = 1;
                }
                if(fstNZ){
                    char c = int2char(digit,0);
                    vga_putc(c);
                    abs = abs - digit*mod;
                }
                mod = mod/10;
            }
        }
    }
}

void printString(char* p){
    while(*p != '\0'){
        vga_putc(*p);
        p ++ ;
    }
}

void kprintln(char* pstr, ...){

    //__builtin_va_* are all macros provided by GCC compiler
    __builtin_va_list ap; 
    __builtin_va_start(ap,pstr);
    
    char* p = pstr;
    uint8_t flag = 0;
    while(*p != '\0'){
        if(*p != '%'){
            if(flag == 0){
                vga_putc(*p);
            }
            else{
                // is an arg, parse it
                switch (*p){
                    case 'd':{ // decimal int32_t
                        int arg_d = __builtin_va_arg(ap,int);
                        printInteger((uint32_t)arg_d,0,1);
                        break;
                    }
                    case 'u':{ //decimal uint32_t
                        uint32_t arg_u = __builtin_va_arg(ap,uint32_t);
                        printInteger(arg_u,0,0);
                        break;
                    }
                    case 'p': 
                    case 'x':{ 
                        uint32_t arg_x = __builtin_va_arg(ap,uint32_t);
                        printInteger(arg_x,1,0);
                        break;
                    }
                    case 'X':{
                        uint32_t arg_X = __builtin_va_arg(ap,uint32_t);
                        printInteger(arg_X,1,1);
                        break;
                    }
                    case 'c':{
                        char arg_c = __builtin_va_arg(ap,int);
                        vga_putc(arg_c);
                        break;
                    }
                    case 's':{
                        char* arg_s = __builtin_va_arg(ap,char*);
                        printString(arg_s);
                        break;
                    }
                    default:{
                        //unknown arg
                        break;
                    }
                }
                flag = 0;
            }
        }
        else{
            if(flag == 1){
                //case of %%
                vga_putc(*p);
                flag = 0;
            }else{
                // is a %
                flag = 1;
            }
        }
        p ++ ;
    }

    __builtin_va_end(ap);
}