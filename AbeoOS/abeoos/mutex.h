/*
* mutex.h
*
* Created: 7/17/2018 5:14:01 AM
*  Author: Do Van Phu
*/


#ifndef MUTEX_H_
#define MUTEX_H_

#include "core.h"

typedef struct mutex mutex_t;

struct mutex{
    unsigned locked:1;
    task_queue_t waiting_queue;
};

#define DEFINE_MUTEX(m) volatile mutex_t m = {  \
    .locked = FALSE,                            \
    .waiting_queue = {                          \
        .head=NULL,                             \
        .tail=NULL                              \
    }                                           \
}

//lock mutex
void mutex_lock(mutex_t* m){
    OS_ENTER_CRITICAL();

    if(m->locked){
        __os_suspend_crr_task(&m->waiting_queue);
    } else m->locked = TRUE;

    OS_EXIT_CRITICAL();
}

void mutex_unlock(mutex_t* m){
    OS_ENTER_CRITICAL();

    task_t* task = m->waiting_queue.head;
    if(task){
        __os_wakeup_task(&m->waiting_queue,task);
    }
    else{
        m->locked = FALSE;
    }

    OS_EXIT_CRITICAL();
}

#endif /* MUTEX_H_ */