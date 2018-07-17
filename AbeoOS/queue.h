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
    //If queue's tail is not NULL
    if(queue->tail){
        queue->tail->next = task;
        task->prev        = queue->tail;
        queue->tail       = task;
        task->next        = NULL;
    }
    else {
        queue->head = queue->tail = task;
        task->next  = task->prev  = NULL;
    }
}

//Insert a task to the beginning of a queue
void __taskqueue_insert_head(task_queue_t* queue, task_t* task){
    //If queue's head is not NULL
    if(queue->head){
        queue->head->prev = task;
        task->next        = queue->head;
        queue->head       = task;
        task->prev        = NULL;
    }
    else {
        queue->head = queue->tail = task;
        task->next  = task->prev  = NULL;
    }
}

//Query if the a queue is empty
bool __taskqueue_empty(task_queue_t* queue){
    return queue->head==NULL;
}

//Remove a task from a queue
void __taskqueue_remove(task_queue_t* queue, task_t* task){
    //If task is in the middle of the queue
    if(task->next!=NULL && task->prev!=NULL){
        task->prev->next = task->next;
        task->next->prev = task->prev;
    }
    //if there is only 1 task in the queue
    else if(task->next==NULL && task->prev==NULL){
        queue->head = queue->tail = NULL;
    }
    //if task is head
    else if(task->next){
        queue->head       = task->next;
        queue->head->prev = NULL;
    }
    //if task is tail
    else {
        queue->tail       = task->prev;
        queue->tail->next = NULL;
    }
    //clear task links
    task->next =
    task->prev = NULL;
}

//rotate the head to tail
void __taskqueue_rotate(task_queue_t* queue){
    //if there is not tasks in the queue then we are done
    if(queue->head==NULL)return;
    //if there is only 1 task in the queue then we also done
    if(queue->head->next == NULL && queue->head->prev == NULL)return;
    //rotate
    queue->tail->next=queue->head;
    queue->head->prev = queue->tail;
    queue->tail = queue->head;
    queue->head = queue->head->next;
    queue->tail->next =
    queue->head->prev = NULL;
}

//Insert a task to a queue high priority first
void __taskqueue_insert_priority(task_queue_t* queue, task_t* task){
    task_t* head = queue->head;
    if(!head || head->priority<task->priority){
        __taskqueue_insert_head(queue,task);
    }
    else{
        //find proper place to insert
        while(head && head->priority >= task->priority)head=head->next;
        if(head){
            head->prev->next=task;
            task->prev = head->prev;
            task->next = head;
            head->prev = task;
        }
        else __taskqueue_insert_tail(queue,task);
    }
}

#endif /* QUEUE_H_ */