/*
 * define.h
 *
 * Created: 7/15/2018 9:50:26 PM
 *  Author: Do Van Phu
 */ 


#ifndef COMMON_H_
#define COMMON_H_

#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>

//Bit set define

#define _BS(R,b) R|=(1<<b)
#define _BC(R,b) R&=~(1<<b)


#define _NAKED_FUNC_ __attribute((naked))
#define _OS_TASK_    _NAKED_FUNC_

#define _INLINE_ __attribute__((always_inline))


#ifndef NULL
#define NULL 0
#endif


typedef void* RAM_ADDR;

typedef unsigned char bool;
#define true  1
#define false 0
#define TRUE  true
#define FALSE false



bool is_num_char(char c){
    return c>='0' && c<='9';
}

bool is_hex_char(char c){
    return is_num_char(c) 
    || (c>='a'&&c<='f')
    || (c>='A'&&c<='F');
}

bool is_space(char c){return c==' ';}

#endif