/*
* config.h
*
* Created: 7/17/2018 12:54:40 AM
*  Author: Do Van Phu
*/


#ifndef CONFIG_H_
#define CONFIG_H_

#ifndef F_CPU
#warning "F_CPU is undefined"
#warning "F_CPU = 16MHz is assumed"
#define F_CPU 16000000L
#endif

#define OS_MS_PER_TICK 1     //ms per os tick

//The main program stack size
#define MAIN_STACK_SIZE 128 

//Using uart x2 mode
//Comment this to use x1
#define USE_UART0_X2

//RX buffer size
//This value must be power of 2
#define UART0_RX_BUF_SIZE 16

//number of decimal places in float print
#define FLOAT_PRINT_DECIMAL_PLACES 4


//Scheduling algorithm
//#define SCHEDULER_ROUND_ROBIN
#define SCHEDULER_HIGH_PRIORITY_FIRST

#endif /* CONFIG_H_ */