#include <clock.h>
#include <pic.h>
#include <stdio.h>
#include <scheduler.h>

static uint32_t schedule_tick;

void init_clk(){
    
    //init clock
    outb(CLK_C,CLK_M);

    //divide frequency, the result should be less than 0xFFFF
    uint32_t cnt = FREQ;
    cnt = cnt / DIVD;

    uint8_t lo = cnt & 0xFF;
    uint8_t hi = (cnt >> 8) & 0xFF;

    outb(CLK_D,lo);
    outb(CLK_D,hi); 
    
    flip_irq_mask(0);
    clk_tick = 0;
    sys_time = 0;
    schedule_tick = 0;
}

void clk_handler(){

    clk_tick ++;
    if(clk_tick == DIVD){
        sys_time ++ ;
        clk_tick = 0;
        //kprintln("sys time %us\n",sys_time);
    }

    schedule_tick ++ ;
    if(schedule_tick == SCHEDULE_PIECE){
        schedule_tick = 0;
        schedule();
    }
}