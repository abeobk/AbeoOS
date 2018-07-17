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

typedef struct task task_t;


#define TASK_PRIORITY_LOW       0
#define TASK_PRIORITY_NORMAL    1
#define TASK_PRIORITY_HIGH      2
#define TASK_PRIORITY_REAL_TIME 3

//task data structure
struct task{
    RAM_ADDR    *sp;   //stack pointer
    union{
        uint8_t     flags; //flags and priorities
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

//task push
_INLINE_
static inline void __task_push(){
    asm volatile(
    //probably not needed but make sure 
    //global interrupt is disabled
    "cli\n"
    //save r0 then save SREG
    "push r0               \n"
    "in r0, __SREG__       \n"
    "push r0               \n"
    //save Z
    "push r30              \n"
    "push r31              \n"
    //load current task address to Z
    "lds r30, %[sp]   \n"
    "lds r31, %[sp]+1 \n"
    //if Z==0 skip context switching
    "mov r0, r30          \n"
    "or  r0, r31          \n"
    // branch if Z != 0
    "brne 1f\n"
    // restore Z if Z==0
    "pop r31       \n"
    "pop r30       \n"
    "pop r0        \n"
    "out __SREG__, r0\n"
    "pop r0        \n"
    //return
    "rjmp 2f\n"
    //store all registers
    "1:\n"
    "push r1\n"
    "push r2\n"
    "push r3\n"
    "push r4\n"
    "push r5\n"
    "push r6\n"
    "push r7\n"
    "push r8\n"
    "push r9\n"
    "push r10\n"
    "push r11\n"
    "push r12\n"
    "push r13\n"
    "push r14\n"
    "push r15\n"
    "push r16\n"
    "push r17\n"
    "push r18\n"
    "push r19\n"
    "push r20\n"
    "push r21\n"
    "push r22\n"
    "push r23\n"
    "push r24\n"
    "push r25\n"
    "push r26\n"
    "push r27\n"
    "push r28\n"
    "push r29\n"

    //r1 must be 0
    "clr r1 \n"

    //save stack pointer to [Z]
    "in r0, __SP_L__\n"
    "st Z+, r0      \n"
    "in r0, __SP_H__\n"
    "st Z+, r0      \n"

    //return
    "2:\n"
    ::[sp] "m" (__os_crr_task+0)
    );
}

//Start task
_NAKED_FUNC_
static void __task_start(){
    //load current task stack pointer
    asm volatile(
    // X = current task address
    "lds r26, %[sp]     \n"
    "lds r27, %[sp]+1   \n"
    // SP = ram[X]
    "ld  r0, x+                     \n"
    "out __SP_L__, r0               \n"
    "ld  r0, x+                     \n"
    "out __SP_H__, r0               \n"
    //pop input data address
    "pop r25 \n"
    "pop r24 \n"
    //enable interrupt
    "sei                            \n"
    //return to task function
    "ret                            \n"
    ::[sp] "m" (__os_crr_task+0)
    );
}


_NAKED_FUNC_
static void __task_pop(){
    asm volatile(
    //restore stack pointer
    "lds r26, %[sp]   \n"
    "lds r27, %[sp]+1 \n"
    "ld r0, X+\n"
    "out __SP_L__, r0\n"
    "ld r0, X+\n"
    "out __SP_H__, r0\n"
    //restore registers
    "pop r29\n"
    "pop r28\n"
    "pop r27\n"
    "pop r26\n"
    "pop r25\n"
    "pop r24\n"
    "pop r23\n"
    "pop r22\n"
    "pop r21\n"
    "pop r20\n"
    "pop r19\n"
    "pop r18\n"
    "pop r17\n"
    "pop r16\n"
    "pop r15\n"
    "pop r14\n"
    "pop r13\n"
    "pop r12\n"
    "pop r11\n"
    "pop r10\n"
    "pop r9\n"
    "pop r8\n"
    "pop r7\n"
    "pop r6\n"
    "pop r5\n"
    "pop r4\n"
    "pop r3\n"
    "pop r2\n"
    "pop r1\n"

    //restore Z
    "pop r31\n"
    "pop r30\n"

    //restore SREG and r0
    "pop r0            \n"
    "out __SREG__, r0  \n"
    //Restore r0
    "pop r0            \n"
    //Return from interrupt will enable interrupt
    "reti              \n"
    :
    :[sp] "m" (__os_crr_task+0)
    );
}

#endif /* TASK_H_ */