/*
* config.h
*
* Created: 7/17/2018 12:54:40 AM
*  Author: Do Van Phu
*/


#ifndef CONFIG_H_
#define CONFIG_H_

//---------------------------------
// OS CONFIGURATIONS
//---------------------------------

//Milli second per OS tick
//#define OS_MS_PER_TICK   1

//Tested with -O3 build and OS_US_PER_TICK = 100
#ifndef OS_MS_PER_TICK
#define OS_US_PER_TICK   500
#else
#define OS_US_PER_TICK   (OS_MS_PER_TICK*1000L)
#endif



// Function hook for idle task
// void os_task_idle_hook_fn()
#define USE_IDLE_TASK_HOOK_FN


//The os stack start address
//Since AVR stack grows down, the OS stack should start at the end
#define OS_STACK_START          RAMEND

//OS stack size
#define OS_STACKSIZE 128

//OS task stack starting address
#define OS_TASK_STACK_START ((stack_ptr_t)OS_STACK_START-OS_STACKSIZE)

//Scheduling algorithm
//#define SCHED_ROUNDROBIN
#define SCHED_HI_PRI_FIRST


#define TASK_STACKSIZE_MIN     64
#define TASK_STACKSIZE_TYP     128



#endif /* CONFIG_H_ */