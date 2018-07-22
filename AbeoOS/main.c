/*
* AbeoOS.c
*
* Created: 7/16/2018 5:10:37 PM
* Author : Do Van Phu
*/

#include "abeoos/os.h"
#define __DELAY_BACKWARD_COMPATIBLE__ 
#include <util/delay.h>

uint32_t ms=10;
uint8_t bit=1;
double pwm=0;

DEFINE_MUTEX(m1);

//__OS_TASK__
void task0(void* data){
    while(1){
        PORTA |= _BV(0);
        os_task_sleep_ms(1);
        _delay_us(pwm);
        PORTA &= ~_BV(0);
        os_task_sleep_ms(15);
    }
}

void task00(void* data){
    while(1){
        pwm+=1;
        if(pwm==1000)pwm=0;
        os_task_sleep_ms(1);
    }
}


//__OS_TASK__
void task1(void* data){
    while(1){
        PORTA ^= _BV(bit);
        os_task_sleep_ms(ms);
    }
}

//__OS_TASK__
void task2(void* data){
    while(1){
        bit++;
        if(bit==8)bit=1;
        ms=(bit+1)*50;
        os_task_sleep_ms(1000*(bit+1));
    }
}    

uint32_t idle_cnt=0;

void os_idle_task_hook_fn(){
    idle_cnt++;
}


__OS_TASK__
void task7(void* data){
    while(1){
        mutex_lock(&m1);
        printf("idle_count: %lu\n",idle_cnt);
        mutex_unlock(&m1);
        os_task_sleep_ms(1000);
    }
}

__OS_TASK__
void task8(void* data){
    while(1){
        mutex_lock(&m1);
        printf("os time: %lu\n",__os_tick_count);
        mutex_unlock(&m1);
        os_task_sleep_ms(5000);
    }
}


int main(void)
{
    DDRA=0xFF;//all B's are output

    //setup uart
    sysuart_init();

    os_create_task(task0,71,TASK_PRI_REALTIME,NULL);
    os_create_task(task00,71,TASK_PRI_REALTIME,NULL);
    os_create_task(task1,71,TASK_PRI_HIGH,0);
    os_create_task(task2,71,TASK_PRI_NORMAL,0);

    os_create_task(task7,128,TASK_PRI_LOW,NULL);
    os_create_task(task8,128,TASK_PRI_NORMAL,NULL);

    os_start();
    
    //Should never reach here
    while(TRUE){
        PORTA=0xFF;
        PORTA=0x00;
    }
}

