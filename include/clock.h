#ifndef __CLOCK_H__
#define __CLOCK_H__

#define CLK_M 0x34 // 7-6 channel0;  5-4 lo/hi; 3-1 rate generator; 0 16bit mode
#define CLK_C 0x43 // clock command port
#define CLK_D 0x40 // dividor port

#define FREQ 1193182 //clock generator frequency ;with bochs, in configure add sync=realtime
#define DIVD 1000 //1ms every tic

#include <utils.h>

static uint32_t clk_tick;
static uint32_t sys_time;

void init_clk(); 
void clk_handler();
#endif