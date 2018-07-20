/*
 * define.h
 *
 * Created: 7/15/2018 9:50:26 PM
 *  Author: Do Van Phu
 */ 


#ifndef COMMON_H_
#define COMMON_H_


#include <inttypes.h>
#include "../arch/arch.h"

//Define a string in program memory
#define __DEFINE_CONST_STR__(name,str) static const char* name=str

//OS assert
#define __OS_ASSERT__(condition, message)


#endif