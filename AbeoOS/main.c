/*
* AbeoOS.c
*
* Created: 7/16/2018 5:10:37 PM
* Author : Do Van Phu
*/

#include <avr/io.h>
#include "abeoos/os.h"

uint32_t ms=100;
uint8_t bit=0;
//volatile uint8_t task_stack[25][128];

DEFINE_MUTEX(m1);


//Setup SysTick ISR
//OS_SETUP_SYSTICK_ISR();

__OS_TASK__
void task0(void* data){
    while(1){
        PORTA ^= _BV(0);
        os_task_sleep_ms(1);
    }
}

__OS_TASK__
void task1(void* data){
    while(1){
        PORTA ^= _BV(bit);
        os_task_sleep_ms(ms);
    }
}

__OS_TASK__
void task2(void* data){
    while(1){
        bit++;
        bit&=7;
        ms=(bit+1)*50;
        os_task_sleep_ms(1000*(bit+1));
    }
}    

__OS_TASK__
void task7(void* data){
    while(1){
        mutex_lock(&m1);
        printf("Hello from task 7:\n");
        mutex_unlock(&m1);
        os_task_sleep_ms(500);
    }
}

__OS_TASK__
void task8(void* data){
    while(1){
        mutex_lock(&m1);
        printf("os time: %u\n",__os_tick_count);
        mutex_unlock(&m1);
        os_task_sleep_ms(1000);
    }
}

int main(void)
{
    DDRA=0xFF;//all B's are output

    //setup uart
    sysuart_init();

    os_create_task(task0,71,TASK_PRI_REALTIME,NULL);
    os_create_task(task1,71,TASK_PRI_HIGH,0);
    os_create_task(task2,71,TASK_PRI_NORMAL,0);

    os_create_task(task7,128,TASK_PRI_LOW,NULL);
    os_create_task(task8,128,TASK_PRI_NORMAL,NULL);

    os_start();
}

