/*
* uart.h
*
* Created: 7/17/2018 6:08:32 AM
*  Author: Do Van Phu
*/

#ifndef UART_H_
#define UART_H_

#include "core.h"
#include <stdio.h>


#ifdef UART_LOG_RX_ERR
//Uart rx error stats
typedef struct{
    uint8_t fe_cnt;  //frame error
    uint8_t doe_cnt; //data overrun error
    uint8_t pe_cnt;  //parity error
    uint8_t bdoe_cnt;//buffer data overrun
}uart_rx_err_t;
#endif

//Uart rx user buffer
typedef struct{
    task_t  *task;      //user task that reading rx
    uint8_t *data;      //user data buffer
    uint16_t count;      //byte to read
}uart_user_data_t;

#define DEFINE_UART_USER_DATA(b)  volatile uart_user_data_t b={.task=NULL,.data=NULL,.count=0}



//Uart buf structure
typedef struct{
    //buffer
    uint8_t             end;                        //current buf end data index
    uint8_t             count;                      //number of bytes in the buffer
    uint8_t             data[UART_RX_BUFSIZE];     //data buffer
    uart_user_data_t    user;                       //user buf

    #ifdef UART_LOG_RX_ERR
    uart_rx_err_t       error;                      //error
    #endif

}uart_rx_buf_t;

//
#ifdef UART_LOG_RX_ERR
#   define DEFINE_UART_BUF(b)  volatile uart_rx_buf_t b={     \
    .end=0,                                                         \
    .count=0,                                                       \
    .user={.task=NULL, .data=NULL, .count=0},                       \
    .error={.fe_cnt=0, .doe_cnt=0, .pe_cnt=0, .bdoe_cnt=0},         \
}
#else
#   define DEFINE_UART_BUF(b)  volatile uart_rx_buf_t b={     \
    .end=0,                                                         \
    .count=0,                                                       \
    .user={.task=NULL, .data=NULL, .count=0},                       \
}
#endif

DEFINE_UART_BUF(sysuart_rx_buf);
DEFINE_UART_USER_DATA(sysuart_tx_data);

__FORCE_INLINE__
uint8_t __uart_peek(uart_rx_buf_t* buf){
    return buf->data[UART_RX_BUF_IDX(buf->end-buf->count)];
}

__FORCE_INLINE__
uint8_t __uart_getc(uart_rx_buf_t* buf){
    return buf->data[UART_RX_BUF_IDX(buf->end-buf->count--)];
}

uint8_t sysuart_getc(FILE* stream);
uint8_t sysuart_putc(uint8_t c, FILE* stream);

FILE sysuart_stream = FDEV_SETUP_STREAM(sysuart_putc,sysuart_getc,_FDEV_SETUP_RW);

//Init uart 0
void sysuart_init(){
    OS_ENTER_CRITICAL();
    __os_sys_uart_init();
    stdout=&sysuart_stream;
    OS_EXIT_CRITICAL();
}

//Receive interrupt handler
ISR_SYSUART_RX(){
    //Log error
    if(SYSUART_RX_ERR){
        //TODO: count error
        #ifdef UART_LOG_RX_ERR
        if(SYSUART_FRAME_ERR         ) sysuart_rx_buf.error.fe_cnt++;
        if(SYSUART_DATA_OVERRUN_ERR  ) sysuart_rx_buf.error.doe_cnt++;
        if(SYSUART_PARITY_ERR        ) sysuart_rx_buf.error.pe_cnt++;
        #endif
        //ack interrupt by reading UDR0
        volatile uint8_t dummy __attribute__((unused)) = UDR0;
        return;
    }

    //if no user buffer specified --> fill rx buffer
    if(sysuart_rx_buf.user.data==NULL){
        sysuart_rx_buf.data[sysuart_rx_buf.end++]=UDR0;
        sysuart_rx_buf.end &= UART_RX_BUFSIZE_MSK;
        sysuart_rx_buf.count++;
        if(sysuart_rx_buf.count==UART_RX_BUFSIZE){
            sysuart_rx_buf.count--;
            #ifdef UART_LOG_RX_ERR
            sysuart_rx_buf.error.bdoe_cnt++;
            #endif
        }
        return;
    }
    
    //Fill user buffer instead
    *sysuart_rx_buf.user.data++=UDR0;
    //nothing more to read
    if(--sysuart_rx_buf.user.count==0){
        //disable user buffer
        sysuart_rx_buf.user.data=NULL;
        //wake up the task manually
        __os_wakeup_task(&__os_taskqueue_suspended,sysuart_rx_buf.user.task);
    }
}


//read to buffer
uint16_t sysuart_read(uint8_t* buf, uint16_t count){
    uint16_t byte_read=0;
    OS_ENTER_CRITICAL();
    //read existing
    while(sysuart_rx_buf.count){
        //getc automatically reduce count
        *buf++=__uart_getc(&sysuart_rx_buf);
        count--;
        byte_read++;
    }
    //read remaining in interrupt with user buffer
    if(count){
        //setup user buffer
        sysuart_rx_buf.user.task  = __os_crr_task;
        sysuart_rx_buf.user.data  = buf;
        sysuart_rx_buf.user.count = count;
        byte_read+=count;
        //suspend current task
        //it will be woken up when done
        __os_suspend_crr_task(&__os_taskqueue_suspended);
    }
    OS_EXIT_CRITICAL();
    return byte_read;
}

uint16_t sysuart_write(const void* buf, uint16_t count){
    OS_ENTER_CRITICAL();
    //setup user data
    sysuart_tx_data.task  = __os_crr_task;
    sysuart_tx_data.data  = buf;
    sysuart_tx_data.count = count;
    
    //enable data register empty interrupt for automatic
    //transmitting
    SYSUART_ENABLE_UDRE_INT();

    //suspend current task
    //this task will be automatically woken up when transmissions are done
    __os_suspend_crr_task(&__os_taskqueue_suspended);
    
    OS_EXIT_CRITICAL();
    return count;
}

//UDR empty interrupt
ISR_SYSUART_UDRE(){
    UDR0 = *sysuart_tx_data.data++;
    //finished
    if(--sysuart_tx_data.count==0){
        SYSUART_DISABLE_UDRE_INT();
        //wake up task
        __os_wakeup_task(&__os_taskqueue_suspended,sysuart_tx_data.task);
    }
}

//Read a character
uint8_t sysuart_getc(FILE* stream){
    char c;
    sysuart_read(&c,1);
    return c;
}

//Peek the first character in uart rx FIFO buffer
uint8_t sysuart_peek(){
    if(sysuart_rx_buf.count==0)return 0;
    OS_ENTER_CRITICAL();
    uint8_t c = __uart_peek(&sysuart_rx_buf);
    OS_EXIT_CRITICAL();
    return c;
}

//Puts char to sysuart
uint8_t sysuart_putc(uint8_t c, FILE* stream){
    uint8_t cc=c;
    sysuart_write(&cc,1);
    return 0;
}

void sysuart_puts(const char* str){
    sysuart_write(str,strlen(str));
}

//flush sysuart buffer
void sysuart_flush(){
    OS_ENTER_CRITICAL();
    sysuart_rx_buf.count=0;
    OS_EXIT_CRITICAL();
}

#if 0

//read integer value from sysuart
int32_t sysuart_get_int(){
    int32_t result=0;
    //read until first num char
    bool neg=FALSE;
    char c;
    while(!is_num_char(c=sysuart_getc())){
        if(c=='-')neg^=TRUE;
    }

    do{
        result*=10;
        result+=c-'0';
    } while(is_num_char(c=sysuart_getc()));
    
    return neg?-result:result;
}

void sysuart_put_int(int32_t value){
    char buf[10];
    uint8_t idx=9;
    bool neg = value<0;
    if(neg)value=-value;
    while(value){
        buf[idx--]=(value%10) + '0';
        value=value/10;
    }
    if(neg)buf[idx--]='-';
    while(++idx<10){
        sysuart_putc(buf[idx]);
    }
}

float sysuart_get_float(){
    float result=0;
    float frac=1;
    //read until first num char
    char c;
    bool neg=FALSE;
    while(!is_num_char(c=sysuart_getc())){
        if(c=='-')neg^=TRUE;
    }

    do{
        result*=10;
        result+=c-'0';
    }while(is_num_char(c=sysuart_getc()));

    //read frac part
    if(c=='.'){
        while(is_num_char(c=sysuart_getc())){
            frac*=10;
            result+=(c-'0')/frac;
        }
    }
    return neg?-result:result;
}

void sysuart_put_float(float value){
    int32_t ipart=value;
    sysuart_put_int(ipart);
    value -= ipart;
    value=value<0?-value:value;
    if(value)sysuart_putc('.');
    uint8_t dec_places=FLT_DEC_PLACES;
    while(value && (dec_places--)){
        value*=10;
        ipart = value;
        value -= ipart;
        //round up
        if(dec_places==0){
            ipart += value>=0.5;
            if(ipart>9)ipart=9;
        }
        sysuart_putc(ipart+'0');
    }
}
#endif


#endif /* UART_H_ */