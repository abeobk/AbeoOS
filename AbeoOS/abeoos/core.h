/*
* os.h
*
* Created: 7/16/2018 4:03:20 PM
*  Author: Do Van Phu
*/


#ifndef OS_CORE_H_
#define OS_CORE_H_

#include "common.h"
#include "task.h"
#include "queue.h"
#include "sched.h"

//There are 3 types of tasks
// 1. Ready to run tasks
//    will be scheduled to run
// 2. Suspended tasks
//    will be scheduled to run once woken up explicitly(manually)
// 3. Sleeping tasks
//    will sleep until the sleep timer expires

//Os's task queue that holds ready to run tasks

DEFINE_TASK_QUEUE(__os_taskqueue_running  );
DEFINE_TASK_QUEUE(__os_taskqueue_suspended);
DEFINE_TASK_QUEUE(__os_taskqueue_sleeping );

//Os tick counter
static volatile uint32_t __os_tick_count = 0;

//Create a task
void os_create_task(
task_fn_t func,         // task function pointer
uint16_t  stack_size,   // stack size
uint8_t   priority,     // priority
void      *data         // task function data
){

    // TODO: MODIFY THIS FUNCTION SO IT CAN WORK WITH
    // DEVICES WHOSE STACK GROWS UP

    //stack start address
    static stack_ptr_t stack_start = OS_TASK_STACK_START;

    //new task
    task_t *task;

    //task pointer is at start of stack
    stack_start -= sizeof(task_t);

    //allocate task at stack start and init it
    task = (task_t*)stack_start;

    __task_init(task,priority);
    
    OS_INIT_TASK_STACK(task->sp,func,data);

    //Compute new stack start for next task
    stack_start -= stack_size;

    //insert to running queue
    #if defined(SCHED_ROUNDROBIN)
    __taskqueue_insert_tail(&__os_taskqueue_running,task);
    #elif defined(SCHED_HI_PRI_FIRST)
    __taskqueue_insert_priority(&__os_taskqueue_running,task);
    #endif
}


//Scheduler
__NAKED__
void __os_task_scheduler(){
    //Move stack pointer to RAMEND so that the scheduler doesn't
    //mess up with the task stacks
    OS_RESTORE_STACK_PTR(OS_STACK_START);

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

            //Resume task
            TASK_RESTORE_STACK_PTR(__os_crr_task);
            RESTORE_CONTEXT();
            reti();
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
void os_start(){
    //setup timer
    __os_systick_init();

    //Run scheduler
    __os_task_scheduler();
}

__NAKED__
void __os_schedule_after_suspending_crr_task(){
    //Perform context switching
    SAVE_CONTEXT();
    TASK_SAVE_STACK_PTR(__os_crr_task);
    __os_task_scheduler();
}

//Put current task to suspended queue
void __os_suspend_crr_task(task_queue_t* queue){
    OS_ENTER_CRITICAL();

    __taskqueue_remove(&__os_taskqueue_running, __os_crr_task);

    #if defined(SCHED_ROUNDROBIN)
    __taskqueue_insert_tail(queue,__os_crr_task);
    #elif defined(SCHED_HI_PRI_FIRST)
    __taskqueue_insert_priority(queue,__os_crr_task);
    #endif
    
    //save current task context
    //then call scheduler
    //calling this function effectively push the next
    //line to crr task stack for later resume
    __os_schedule_after_suspending_crr_task();

    OS_EXIT_CRITICAL();
}

//
//Put current task to sleeping queue for an amount of time (in us)
//
#if defined(OS_MS_PER_TICK)
void os_task_sleep_ms(uint16_t ms){
    __os_crr_task->delay = ms / OS_MS_PER_TICK;
    __os_suspend_crr_task(&__os_taskqueue_sleeping);
}

//NOt supported
#define os_task_sleep_us(us) os_task_sleep_ms((us+1000L)/1000L)

#else

void os_task_sleep_us(uint32_t us){
    __os_crr_task->delay = us/OS_US_PER_TICK;
    __os_suspend_crr_task(&__os_taskqueue_sleeping);
}

#define os_task_sleep_ms(ms) os_task_sleep_us((ms)*1000L)

#endif


//
//wake up a suspended task
//
void __os_wakeup_task(task_queue_t* queue, task_t* task){
    //OS_ENTER_CRITICAL();

    __taskqueue_remove(queue,task);

    //insert to running queue
    #if defined(SCHED_ROUNDROBIN)
    __taskqueue_insert_tail(&__os_taskqueue_running,task);
    #elif defined(SCHED_HI_PRI_FIRST)
    __taskqueue_insert_priority(&__os_taskqueue_running,task);
    #endif

    //OS_EXIT_CRITICAL();
}


// Os tick function
__FORCE_INLINE__
void __os_tick(){
    //advance tick counter
    __os_tick_count++;
    //check all sleeping tasks to find ready task
    task_t *task=__os_taskqueue_sleeping.head;
    task_t *next_task;
    while(task){
        next_task=task->next;
        //reduce delay of task
        if(task->delay)task->delay--;
        //wake up task if it is ready to run
        if(task->delay==0)__os_wakeup_task(&__os_taskqueue_sleeping,task);
        //next
        task = next_task;
    }
}


//
//OS_TICK: This is the body of the SYSTICK_ISR
//
/*
#define OS_TICK()                                \
OS_SAVE_STACK_PTR(__os_crr_task->sp);            \
SAVE_CONTEXT();                                  \
__os_tick();                                     \
OS_INDIRECT_JUMP(__os_task_scheduler);
*/

//__NAKED__
//__os_systick_handler(){
//if(__os_crr_task){
//SAVE_CONTEXT();
//TASK_SAVE_STACK_PTR(__os_crr_task->sp);
//}
//
//__os_tick();
//
//OS_INDIRECT_JUMP(__os_task_scheduler);
//}

//SysTick ISR
ISR_SYSTICK(){
    //__os_systick_handler();
    //reti();
    if(__os_crr_task){
        SAVE_CONTEXT();
        TASK_SAVE_STACK_PTR(__os_crr_task);
    }

    __os_tick();

    OS_INDIRECT_JUMP(__os_task_scheduler);
}


#endif /* OS_H_ */