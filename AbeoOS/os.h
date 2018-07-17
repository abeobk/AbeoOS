/*
* os.h
*
* Created: 7/16/2018 4:03:20 PM
*  Author: Do Van Phu
*/


#ifndef OS_H_
#define OS_H_

#include "task.h"
#include "queue.h"
#include "sched.h"

//There are 3 types of tasks
// 1. Ready to run tasks
//    will be scheduled to run
// 2. Suspended tasks
//    will be scheduled to run once woken up explicitly
// 3. Sleeping tasks
//    will sleep until the sleep timer expires

//Os's task queue that holds ready to run tasks

DEFINE_TASK_QUEUE(__os_taskqueue_running  );
DEFINE_TASK_QUEUE(__os_taskqueue_suspended);
DEFINE_TASK_QUEUE(__os_taskqueue_sleeping );

//Os tick counter
static volatile uint32_t __os_tick_count            = 0;


//Save sreg and disable interrupt
#define _SAVE_SREG uint8_t sreg=SREG; cli()
//Restore sreg
#define _RESTORE_SREG SREG=sreg

//Create a task
static void os_create_task(
task_fn_t func,         // task function pointer
uint16_t  stack_size,   // stack size
uint8_t   priority,     // priority
void      *data         // task function data
){
    //TODO:
    // Init stack space for the task starting from RAMEND
    //     RAMEND side
    // ~.................~
    // + ----------------+
    // |                 | <-- Task structure
    // +-----------------+
    // |                 | <-- Start of stack
    // |                 |
    // .                 .
    // .                 .
    // .                 .
    // +-----------------+

    //stack start address
    static RAM_ADDR stack_start = (RAM_ADDR)RAMEND-MAIN_STACK_SIZE;

    //new task
    task_t *task;

    //STACK GROWS DOWN

    //task pointer is at start of stack
    stack_start -= sizeof(task_t);
    task = stack_start;

    //stack pointer starting address
    task->sp = stack_start-1;

    //reset flags
    task->flags=0;

    //ready to run
    task->delay=0;

    //set priority
    task->priority = priority;

    //init task, preparing stack

    //insert function address to the beginning of the task
    //save current stack pointer
    RAM_ADDR sp = SP;
    SP = task->sp;

    //prepare stack for first run
    //function address
    //and data
    asm volatile(
    "push %A0 \n"
    "push %B0 \n"
    "push %A1 \n"
    "push %B1 \n"
    ::"r" (func), "r" (data)
    );

    //set new task stack pointer
    task->sp = SP;

    //restore stack pointer and continue
    SP=sp;
    
    //Compute new stack start for current task
    stack_start -= stack_size;

    //insert to running queue
#if defined(SCHEDULER_ROUND_ROBIN)
    __taskqueue_insert_tail(&__os_taskqueue_running,task);
#elif defined(SCHEDULER_HIGH_PRIORITY_FIRST)
    __taskqueue_insert_priority(&__os_taskqueue_running,task);
#endif
}


//Scheduler
_NAKED_FUNC_
static void __os_task_scheduler(){
    //Move stack pointer to RAMEND so that the scheduler doesn't
    //mess up with the task stacks
    asm volatile(
    "out __SP_L__, %A0 \n"
    "out __SP_H__, %B0 \n"
    ::"x" (RAMEND)
    );

    //Scheduling
    while(TRUE){
        while(TRUE){
            //current task = first task in queue
            __os_crr_task = __os_taskqueue_running.head;

            //no task to run
            if(__os_crr_task==NULL)break;

            //rotate head to tail allowing next task to be run
            __taskqueue_rotate(&__os_taskqueue_running);

            //continue current task
            //we don't have to save the current context
            //since the function starts from beginning
            //next time the os's timer ticks anyway
            if(__os_crr_task->started) __task_pop();
            else {
                __os_crr_task->started=TRUE;
                __task_start();
            }
        }

        //enable interrupt
        sei();
        //put the CPU to sleep
        asm volatile("sleep");
        //- if woken up by the scheduling timer
        //  this function will start from the beginning
        //- if woken up by other interrupt sources,
        //  this function will continue from here
        cli();
    }
}

//Start os
static void os_start(){
    //setup timer
    TCCR0A = _BV(WGM01); //Clear timer on compare match

    TCCR0B = _BV(CS00)|_BV(CS01); // 1/64 prescale -->250kHz input

    OCR0A  = (F_CPU/64)/(1000/OS_MS_PER_TICK);// 250 pulse per tick ~ 1kHz tick timer

    //enable os timer interrupt
    _BS(TIMSK0, OCIE0A);

    //Run scheduler
    __os_task_scheduler();
}


//wake up a suspended task
static void __os_wakeup_task(task_queue_t* queue, task_t* task){
    __taskqueue_remove(queue,task);

    //insert to running queue
#if defined(SCHEDULER_ROUND_ROBIN)
    __taskqueue_insert_tail(&__os_taskqueue_running,task);
#elif defined(SCHEDULER_HIGH_PRIORITY_FIRST)
    __taskqueue_insert_priority(&__os_taskqueue_running,task);
#endif
}

//
static void __os_tick(){
    __os_tick_count++;

    //check all sleeping tasks to find ready task
    task_t *task=__os_taskqueue_sleeping.head;
    task_t *next_task;
    while(task){
        //save next task
        next_task = task->next;

        //reduce delay of task
        if(task->delay)task->delay--;
        
        //if delay expires
        //then wake up task
        if(task->delay==0)__os_wakeup_task(&__os_taskqueue_sleeping,task);
        task = next_task;
    }
}

_NAKED_FUNC_
static void __os_task_yield(){
    //push current task context
    __task_push();

    //jump to scheduler function
    asm("ijmp"::"z" (__os_task_scheduler));
}


//Put current task to suspended queue
//This is for mutex feature
static void __os_suspend_current_task(task_queue_t* queue){
    _SAVE_SREG;

    //put current task to queue
    //1. remove current task from running queue
    __taskqueue_remove(&__os_taskqueue_running,__os_crr_task);

    //2. insert current task to the queue
#if defined(SCHEDULER_ROUND_ROBIN)
    __taskqueue_insert_tail(queue,__os_crr_task);
#elif defined(SCHEDULER_HIGH_PRIORITY_FIRST)
    __taskqueue_insert_priority(queue,__os_crr_task);
#endif

    //3. save context and move to next task
    __os_task_yield();

    _RESTORE_SREG;
}

//Put current task to sleep queue for an amount of time
static void os_task_sleep( uint16_t ms){
    if(ms==0)return;
    __os_crr_task->delay = ms/OS_MS_PER_TICK;
    __os_suspend_current_task(&__os_taskqueue_sleeping);
}

//Timer interrupt
ISR(TIMER0_COMPA_vect, ISR_NAKED){
    //push task context to its stack
    __task_push();

    //increase tick
    __os_tick();
    //return from interrupt

    //jump to scheduler function
    asm("ijmp"::"z" (__os_task_scheduler));

    //enable interrupt
    reti();
}

#endif /* OS_H_ */