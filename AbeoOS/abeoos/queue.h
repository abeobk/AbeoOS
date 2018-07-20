/*
* queue.h
*
* Created: 7/16/2018 4:02:43 PM
*  Author: Do Van Phu
*/


#ifndef QUEUE_H_
#define QUEUE_H_

#include "task.h"

typedef struct task_queue task_queue_t;

struct task_queue{
    task_t       *head;
    task_t       *tail;
};

#define DEFINE_TASK_QUEUE(q) static task_queue_t q ={               \
    .head=NULL, \
    .tail=NULL  \
}

//Init an empty queue
void __taskqueue_init(task_queue_t* queue){
    //Init an empty queue
    queue->head =
    queue->tail = NULL;
}


//Insert a task to the end of a queue
void __taskqueue_insert_tail(task_queue_t* queue, task_t* task){
    __OS_ASSERT__(!queue, "queue cant be NULL");
    __OS_ASSERT__(!task , "task cant be NULL");
    __OS_ASSERT__(!(task->prev|task->next),"Task belong to other queue");
    //If queue's tail is not NULL
    if(queue->tail){
        queue->tail->next = task;
        task->prev        = queue->tail;
        queue->tail       = task;
    }
    else {
        queue->head = queue->tail = task;
    }
}

//Insert a task to the beginning of a queue
//Task must not belong to other queue
void __taskqueue_insert_head(task_queue_t* queue, task_t* task){
    //if task->prev != NULL or -> ERROR
    __OS_ASSERT__(!queue, "queue cant be NULL");
    __OS_ASSERT__(!task , "task cant be NULL");
    __OS_ASSERT__(!(task->prev|task->next),"Task belong to other queue");

    //queue is not empty
    if(queue->head){
        queue->head->prev = task;
        task->next        = queue->head;
        queue->head       = task;
    }
    else {
        queue->head = queue->tail = task;
    }
}

//Query if the a queue is empty
#define __taskqueue_empty(queue) (queue->head==NULL)

//Remove a task from a queue
//Requirements:
//task must belong to queue
//task must not be NULL
void __taskqueue_remove(task_queue_t* queue, task_t* task){
    __OS_ASSERT__(!queue, "queue cant be NULL");
    __OS_ASSERT__(!task , "task cant be NULL");
    
    //queue has 1 task
    if(queue->head==queue->tail)
    {
        queue->head = queue->tail = NULL;
    }
    //task is head
    else if(queue->head==task){
        queue->head       = task->next;
        queue->head->prev = NULL;
    }
    //task is tail
    else if(queue->tail==task){
        queue->tail       = task->prev;
        queue->tail->next = NULL;
    }
    //If task is in the middle of the queue
    else{
        task->prev->next = task->next;
        task->next->prev = task->prev;
    }
    //reset link
    task->next =
    task->prev = NULL;
}

//rotate the head to tail
void __taskqueue_rotate(task_queue_t* queue){
    __OS_ASSERT__(!queue, "queue cant be NULL");
    //if there is no tasks or only 1 task in the queue then we done
    // no task:= head==tail==null
    // one task:= head==tail
    if(queue->head==queue->tail)return;

    //rotate
    queue->tail->next = queue->head;
    queue->head->prev = queue->tail;
    queue->tail = queue->head;
    queue->head = queue->head->next;
    queue->tail->next =
    queue->head->prev = NULL;
}

//Insert a task to a queue high priority first
void __taskqueue_insert_priority(task_queue_t* queue, task_t* task){
    __OS_ASSERT__(!queue, "queue cant be NULL");
    __OS_ASSERT__(!task , "task cant be NULL");

    task_t* t = queue->head;
    if(!t || t->priority < task->priority){
        __taskqueue_insert_head(queue,task);
    }
    else{
        //find proper place to insert
        while(t && t->priority >= task->priority)t=t->next;
        if(t){
            t->prev->next=task;
            task->prev = t->prev;
            task->next = t;
            t->prev = task;
        }
        else __taskqueue_insert_tail(queue,task);
    }
}

#endif /* QUEUE_H_ */