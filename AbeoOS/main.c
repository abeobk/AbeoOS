/*
* AbeoOS.c
*
* Created: 7/16/2018 5:10:37 PM
* Author : Do Van Phu
*/


#include "os.h"
#include "mutex.h"
#include "uart.h"

#include <avr/delay.h>

uint32_t ms=1;



void task1(void* data){
    //init
    while(1){
        PORTB ^= _BV(PORTB5);
        os_task_sleep(ms);
    }
}

void task11(void* data){
    //init
    while(1){
        PORTB ^= _BV(PORTB4);
        os_task_sleep(3);
    }
}

void task2(void* data){
    while(1){
    }
}
DEFINE_MUTEX(m1);

void task6(void* data){
    while(1){
        ms = uart0_get_int();
        if(ms>2000000)ms=2000000;
        mutex_lock(&m1);
        uart0_puts("ms=");
        uart0_put_int(ms);
        uart0_putc('\n');
        mutex_unlock(&m1);
        os_task_sleep(1000);
    }
}

void task7(void* data){
    while(1){
        mutex_lock(&m1);
        //print os tick
        uart0_puts("os time: ");
        uart0_put_float(__os_tick_count/1000.0f);
        uart0_puts("ms\n");
        mutex_unlock(&m1);
        os_task_sleep(1000);
    }
}

int main(void)
{
    DDRB=0xFF;//all B's are output

    //setup uart
    uart0_init(115200);

    os_create_task(task2,128,TASK_PRIORITY_NORMAL,NULL);
    os_create_task(task2,128,TASK_PRIORITY_LOW,NULL);
    os_create_task(task2,128,TASK_PRIORITY_HIGH,NULL);
    os_create_task(task1,128,TASK_PRIORITY_REAL_TIME,NULL);
    os_create_task(task11,128,TASK_PRIORITY_REAL_TIME,NULL);
    //os_create_task(task2,128,TASK_PRIORITY_NORMAL,NULL);
    //os_create_task(task2,128,TASK_PRIORITY_NORMAL,NULL);
    //os_create_task(task2,128,TASK_PRIORITY_NORMAL,NULL);
    //os_create_task(task2,128,TASK_PRIORITY_NORMAL,NULL);
    //os_create_task(task2,128,TASK_PRIORITY_NORMAL,NULL);
    os_create_task(task6,128,TASK_PRIORITY_NORMAL,NULL);
    os_create_task(task7,128,TASK_PRIORITY_LOW,NULL);

    os_start();
}

