/*
* task.h
*
* Created: 7/15/2018 9:57:36 PM
*  Author: Do Van Phu
*/


#ifndef TASK_H_
#define TASK_H_

#include "common.h"

// task function pointer
typedef void (*task_fn_t)(void *);


//Task priority levels
#define TASK_PRI_LOW       0
#define TASK_PRI_NORMAL    1
#define TASK_PRI_HIGH      2
#define TASK_PRI_REALTIME  3

//task data structure
typedef struct task task_t;

struct task{
    stack_ptr_t        sp;          //stack pointer
    union{
        uint8_t     flags;          //flags and priorities
        struct{
            unsigned priority  : 2; //priority of the task
            unsigned started   : 1; //task is started
            unsigned reserved  : 5; //reserved
        };
    };
    uint16_t    delay; //number of ticks until the task ready to run
    task_t      *prev; //next task in the queue
    task_t      *next; //next task in the queue
};


//Current running task
static volatile task_t * __os_crr_task    = NULL;

#define __os_get_current_task() __os_crr_task

void __task_init(task_t* task, uint8_t priority){
    task->sp = (stack_ptr_t)task-1;
    task->flags=0;
    task->priority =priority;
    task->delay=0;
    task->prev=
    task->next=NULL;
}

#endif /* TASK_H_ */