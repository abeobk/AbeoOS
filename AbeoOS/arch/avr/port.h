/*
 * port.h
 *
 * Created: 7/22/2018 12:32:47 AM
 *  Author: Do Van Phu
 */ 


#ifndef PORT_H_
#define PORT_H_


//CONFIG CPU frequency
#ifndef F_CPU
#warning "F_CPU is undefined"
#warning "F_CPU = 16MHz is assumed"
#define F_CPU 16000000L
#endif

#include <avr/io.h>
#include <avr/interrupt.h>


#include "config.h"


//---------------------------------
// COMMON DEFINITIONS
//---------------------------------
#ifndef _BV
#   define _BV(b) (1<<(b))
#endif

#define _BS(R,b) R|=_BV(b)
#define _BC(R,b) R&=~_BV(b)

//Naked function
#define __NAKED__ __attribute__((naked))
#define __OS_TASK__  __NAKED__

//Force function always inline
#define __FORCE_INLINE__ __attribute__((always_inline))

#ifndef NULL
#   define NULL 0
#endif

//stack pointer type
typedef uint8_t* stack_ptr_t;

//Boolean type
#ifndef bool_t
typedef unsigned char bool_t;
#endif

#define true  1
#define false 0

#define TRUE  true
#define FALSE false


//----------------------------------
// PORTING CONFIGURATION for AVR
//----------------------------------


#define ENABLE_SLEEP_MODE()    MCUCR = _BV(SE)


//RX buffer size
//This value must be power of 2
#define UART_RX_BUFSIZE (1<<5)  //32

#define UART_RX_BUFSIZE_MSK         (UART_RX_BUFSIZE-1                )
#define UART_RX_BUF_IDX(v)          ((v)&UART_RX_BUFSIZE_MSK          )

//Enable uart error counters
//#define UART_ENABLE_ERR_CNT


#if defined(RAMPZ)

#define SAVE_RAMPZ         \
"in     r0, 0x3b       \n" \
"push   r0             \n"


#define RESTORE_RAMPZ      \
"pop    r0             \n" \
"out    0x3b, r0       \n"

#define PUSH_RAMPZ(sp) *sp-- = 0

#else

#define SAVE_RAMPZ    ""
#define RESTORE_RAMPZ ""

#define PUSH_RAMPZ(sp)

#endif


#if defined(EIND)

#define SAVE_EIND           \
"in     r0, 0x3c        \n" \
"push   r0              \n"

#define RESTORE_EIND        \
"pop    r0              \n" \
"out    0x3c, r0        \n"

#define PUSH_EIND(sp) *sp-- = 0

#else

#define SAVE_EIND    ""
#define RESTORE_EIND ""
#define PUSH_EIND(sp)

#endif




#define AVR_REG_CNT         32      //32 register in context
#define AVR_ARG_REG_NUM     24      //input argument stored in r24 r25

//Push pointer value to stack
#define PUSH_PTR(sp,ptr)    \
*sp-- = (int16_t)ptr;       \
*sp-- = (int16_t)ptr>>8

//Code for initializing task stack
#define OS_INIT_TASK_STACK(sp,func,data)                            \
PUSH_PTR(sp,func);                                                  \
*sp--=0;                                                            \
*sp--=0;                                                            \
*sp--=30;                                                           \
*sp--=31;                                                           \
PUSH_RAMPZ(sp);                                                     \
PUSH_EIND(sp);                                                      \
uint8_t ii_;                                                        \
for(ii_=1;ii_<AVR_ARG_REG_NUM ;ii_++){*sp--=0;}                     \
PUSH_PTR(sp,data);                                                  \
for(ii_=AVR_ARG_REG_NUM+2;ii_<AVR_REG_CNT-2;ii_++){*sp--=0;}


//Restore OS stack pointer
#define OS_RESTORE_STACK_PTR(sp)    \
asm volatile(                       \
"out __SP_L__, %A0      \n"         \
"out __SP_H__, %B0      \n"         \
::"z" (sp))


//Save task context
#define SAVE_CONTEXT(task)   \
asm volatile(                \
"cli                     \n" \
"push   r0               \n" \
"in     r0, __SREG__     \n" \
"push   r0               \n" \
"push   r30              \n" \
"push   r31              \n" \
"lds    r30,%[tsp]       \n" \
"lds    r31,%[tsp]+1     \n" \
"mov    r0,r30           \n" \
"or     r0,r31           \n" \
"brne   1f               \n" \
"pop    r31              \n" \
"pop    r30              \n" \
"pop    r0               \n" \
"out    __SREG__, r0     \n" \
"pop    r0               \n" \
"rjmp   2f               \n" \
"1:                      \n" \
SAVE_RAMPZ                   \
SAVE_EIND                    \
"push   r1               \n" \
"push   r2               \n" \
"push   r3               \n" \
"push   r4               \n" \
"push   r5               \n" \
"push   r6               \n" \
"push   r7               \n" \
"push   r8               \n" \
"push   r9               \n" \
"push   r10              \n" \
"push   r11              \n" \
"push   r12              \n" \
"push   r13              \n" \
"push   r14              \n" \
"push   r15              \n" \
"push   r16              \n" \
"push   r17              \n" \
"push   r18              \n" \
"push   r19              \n" \
"push   r20              \n" \
"push   r21              \n" \
"push   r22              \n" \
"push   r23              \n" \
"push   r24              \n" \
"push   r25              \n" \
"push   r26              \n" \
"push   r27              \n" \
"push   r28              \n" \
"push   r29              \n" \
"clr    __zero_reg__     \n" \
"in     r0, __SP_L__     \n" \
"st     Z+, r0           \n" \
"in     r0, __SP_H__     \n" \
"st     Z+, r0           \n" \
"2:                      \n" \
::[tsp] "m" (task)           \
:"r0", "r30", "r31")


//Restore task context
#define RESTORE_CONTEXT(task)\
asm volatile(                \
"lds    r26, %[tsp]       \n"\
"lds    r27, %[tsp]+1     \n"\
"ld     r0, X+            \n"\
"out    __SP_L__, r0      \n"\
"ld     r0, X+            \n"\
"out    __SP_H__, r0      \n"\
"pop    r29               \n"\
"pop    r28               \n"\
"pop    r27               \n"\
"pop    r26               \n"\
"pop    r25               \n"\
"pop    r24               \n"\
"pop    r23               \n"\
"pop    r22               \n"\
"pop    r21               \n"\
"pop    r20               \n"\
"pop    r19               \n"\
"pop    r18               \n"\
"pop    r17               \n"\
"pop    r16               \n"\
"pop    r15               \n"\
"pop    r14               \n"\
"pop    r13               \n"\
"pop    r12               \n"\
"pop    r11               \n"\
"pop    r10               \n"\
"pop    r9                \n"\
"pop    r8                \n"\
"pop    r7                \n"\
"pop    r6                \n"\
"pop    r5                \n"\
"pop    r4                \n"\
"pop    r3                \n"\
"pop    r2                \n"\
"pop    r1                \n"\
RESTORE_EIND                 \
RESTORE_RAMPZ                \
"pop    r31               \n"\
"pop    r30               \n"\
"pop    r0                \n"\
"out    __SREG__, r0      \n"\
"pop    r0                \n"\
::[tsp] "m" (task))


//Stack growing down for AVR
#define OS_STACK_GROWN_DOWN TRUE

//Enter critical section, save status register and disable all interrupts
//Restore the Status register
#define OS_ENTER_CRITICAL() uint8_t sreg=SREG; cli()
#define OS_EXIT_CRITICAL()  SREG=sreg


//Jump to a function indirectly
#define OS_INDIRECT_JUMP(fn)    asm volatile ("ijmp" :: "z" (fn))



//SysTick timer interrupt service routine
#define ISR_SYSTICK()       ISR(SYSTICK_IRQ_VECT,ISR_NAKED)
#define ISR_SYSUART_RX()    ISR(SYSUART_RX_IRQ_VECT)
#define ISR_SYSUART_UDRE()  ISR(SYSUART_UDRE_IRQ_VECT)


#define OS_SYSUART_BAUDRATE     115200


// atmega128, atmega128a
#if defined(__AVR_ATmega128__)||\
defined(__AVR_ATmega128A__)

//Setup SysTick timer
void __os_systick_init(){
    TCCR0 = _BV(WGM01);                                //top = OCR0, Clear OCR0 o n compare match
    //TCCR0 |= _BV(COM1);                              //toggle OC0 pin
    TCCR0 |= _BV(CS02);                                // 1/64 prescale
    #if defined(OS_MS_PER_TICK)
    OCR0  = (F_CPU/64)/(1000L/OS_MS_PER_TICK);
    #else
    OCR0  = (F_CPU/64)/(1000000L/OS_US_PER_TICK);
    #endif
    TIMSK |= _BV(OCIE0);                               //enable interrupt
}

void __os_sys_uart_init(){
    UCSR0A = _BV(U2X0);
    UBRR0L = (F_CPU/8/OS_SYSUART_BAUDRATE-1);
    UBRR0H = (F_CPU/8/OS_SYSUART_BAUDRATE-1)>>8;
    UCSR0B = _BV(RXEN0)|_BV(TXEN0);
    UCSR0C = _BV(UCSZ01)|_BV(UCSZ00);
    UCSR0B|= _BV(RXCIE0);
}


#define SYSTICK_IRQ_VECT            TIMER0_COMP_vect
#define SYSUART_RX_IRQ_VECT         USART0_RX_vect
#define SYSUART_UDRE_IRQ_VECT       USART0_UDRE_vect

#define SYSUART_DISABLE_UDRE_INT() UCSR0B &= ~_BV(UDRIE0)
#define SYSUART_ENABLE_UDRE_INT()  UCSR0B |=  _BV(UDRIE0)

#define UART_FE_MSK      _BV(FE0)
#define UART_DOE_MSK     _BV(DOR0)
#define UART_PE_MSK      _BV(UPE0)

#define UART_ERR_MSK (UART_FE_MSK | UART_DOE_MSK | UART_PE_MSK)

#define SYSUART_RX_ERR              (UCSR0A&UART_ERR_MSK              )
#define SYSUART_FRAME_ERR           (UCSR0A&UART_FE_MSK               )
#define SYSUART_DATA_OVERRUN_ERR    (UCSR0A&UART_DOE_MSK              )
#define SYSUART_PARITY_ERR          (UCSR0A&UART_PE_MSK               )


//atmega328, atmega328p
#elif defined(__AVR_ATmega328__)||\
defined(__AVR_ATmega328P__)

void __os_systick_init(){
    TCCR0A  = _BV(WGM01);                           //Clear timer on compare match
    TCCR0B  = _BV(CS00)|_BV(CS01);                  // 1/64 prescale
    #if defined(OS_MS_PER_TICK)
    OCR0A   = (F_CPU/64)/(1000L/OS_MS_PER_TICK);
    #else
    OCR0A   = (F_CPU/64)/(1000000L/OS_US_PER_TICK);
    #endif
    TIMSK0 |= _BV(OCIE0A);
}

void __os_sys_uart_init(){
    UCSR0A =_BV(U2X0);
    UBRR0 = F_CPU/8/OS_SYSUART_BAUDRATE-1;
    UCSR0B = _BV(RXEN0)|_BV(TXEN0);
    UCSR0C =_BV(UCSZ01)|_BV(UCSZ00);
    UCSR0B|= _BV(RXCIE0);
}


#define SYSTICK_IRQ_VECT            TIMER0_COMPA_vect
#define SYSUART_RX_IRQ_VECT         USART0_RX_vect
#define SYSUART_UDRE_IRQ_VECT       USART0_UDRE_vect

#define SYSUART_DISABLE_UDRE_INT() UCSR0B &= ~_BV(UDRIE0)
#define SYSUART_ENABLE_UDRE_INT()  UCSR0B |=  _BV(UDRIE0)

#define UART_FE_MSK      _BV(FE0)
#define UART_DOE_MSK     _BV(DOR0)
#define UART_PE_MSK      _BV(UPE0)

#define UART_ERR_MSK (UART_FE_MSK | UART_DOE_MSK | UART_PE_MSK)

#define SYSUART_RX_ERR              (UCSR0A&UART_ERR_MSK              )
#define SYSUART_FRAME_ERR           (UCSR0A&UART_FE_MSK               )
#define SYSUART_DATA_OVERRUN_ERR    (UCSR0A&UART_DOE_MSK              )
#define SYSUART_PARITY_ERR          (UCSR0A&UART_PE_MSK               )

#endif





#endif /* PORT_H_ */