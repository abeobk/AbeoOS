/*
* mutex.h
*
* Created: 7/17/2018 5:14:01 AM
*  Author: Do Van Phu
*/


#ifndef MUTEX_H_
#define MUTEX_H_

#include "os.h"

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
    _SAVE_SREG;

    if(m->locked){
        __os_suspend_current_task(&m->waiting_queue);
    } else m->locked = TRUE;

    _RESTORE_SREG;
}

void mutex_unlock(mutex_t* m){
    _SAVE_SREG;

    task_t* task = m->waiting_queue.head;
    if(task){
        __os_wakeup_task(&m->waiting_queue,task);
    }
    else{
        m->locked = FALSE;
    }

    _RESTORE_SREG;
}

#endif /* MUTEX_H_ */