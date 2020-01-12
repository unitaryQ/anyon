#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__
#include <process.h>

typedef struct process_queue_s{
    enum process_state stat;
    uint32_t num;
    list_t node;
}process_queue_t;

process_queue_t ready_queue;
int schedule();
#endif