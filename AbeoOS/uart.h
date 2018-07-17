/*
* uart.h
*
* Created: 7/17/2018 6:08:32 AM
*  Author: Do Van Phu
*/


#ifndef UART_H_
#define UART_H_

#include "os.h"

#define UART_FRAME_ERROR_MASK        _BV(FE0)
#define UART_DATA_OVERRUN_ERROR_MASK _BV(DOR0)
#define USART_PARITY_ERROR_MASK      _BV(UPE0)

#define UART_ERROR_MASK (UART_FRAME_ERROR_MASK | UART_DATA_OVERRUN_ERROR_MASK | USART_PARITY_ERROR_MASK)

#define UART0_RX_ERROR              (UCSR0A&UART_ERROR_MASK             )
#define UART0_FRAME_ERROR           (UCSR0A&UART_FRAME_ERROR_MASK       )
#define UART0_DATA_OVERRUN_ERROR    (UCSR0A&UART_DATA_OVERRUN_ERROR_MASK)
#define USART0_PARITY_ERROR         (UCSR0A&USART_PARITY_ERROR_MASK     )

#define UART0_RX_BUF_SIZE_MASK      (UART0_RX_BUF_SIZE-1                )
#define UART0_RX_BUF_IDX(v)         ((v)&UART0_RX_BUF_SIZE_MASK         )

//uart0 rx buf
static uint8_t uart0_rx_buf[UART0_RX_BUF_SIZE];
//uart0 rx buf end id
static volatile uint8_t uart0_rx_buf_end=0;
//uart0 rx buf start id
static volatile uint8_t uart0_rx_count=0;


#define uart0_rx_buf_start          UART0_RX_BUF_IDX(uart0_rx_buf_end-uart0_rx_count)

static void uart0_init(uint32_t baud){
    _SAVE_SREG;
    
    #if defined(USE_UART0_X2)
    //set double transmission speed
    UCSR0A = _BV(U2X0);
    //baud
    UBRR0 = F_CPU/8/baud-1;
    #else
    UCSR0A = 0x00;
    //baud
    UBRR0 = F_CPU/16/baud-1;
    #endif

    //enable tx & rx
    UCSR0B = _BV(RXEN0)|_BV(TXEN0);

    //async mode, no parity, 1 stop bit, 8 bit
    UCSR0C = _BV(UCSZ01)|_BV(UCSZ00);

    //enable RX interrupt
    UCSR0B |= _BV(RXCIE0);

    _RESTORE_SREG;
}

//Receive interrupt handler
ISR(USART_RX_vect){
    if(UART0_RX_ERROR){
        //TODO: count error
        //ack interrupt by reading UDR0
        volatile uint8_t dummy __attribute__((unused)) = UDR0;
        return;
    }

    uart0_rx_buf[uart0_rx_buf_end++]=UDR0;
    uart0_rx_count++;
    //since UART0_RX_BUF_SIZE is power of 2
    //its easy to loop back
    uart0_rx_buf_end&=(UART0_RX_BUF_SIZE-1);

    //check for overrun error
    if(uart0_rx_count==UART0_RX_BUF_SIZE){
        //count overrun error
    }
}


/*
//read to buffer
uint16_t uart0_read(uint8_t* buf, uint16_t count){
uint16_t byte_read=0;
_SAVE_SREG;
//read existing
while(uart0_rx_count){
*buf=uart0_rx_buf[uart0_rx_buf_start];
buf++;
uart0_rx_count--;
count--;
byte_read++;
}
_RESTORE_SREG;
}
*/

//Gets char from uart0 (blocking)
uint8_t uart0_getc(){
    while(uart0_rx_count==0);
    _SAVE_SREG;
    uint8_t c=uart0_rx_buf[uart0_rx_buf_start];
    uart0_rx_count--;
    _RESTORE_SREG;
    return c;
}

//Peek the first character in uart rx FIFO buffer
uint8_t uart0_peek(){
    while(uart0_rx_count==0);
    _SAVE_SREG;
    uint8_t c = uart0_rx_buf[uart0_rx_buf_start];
    _RESTORE_SREG;
    return c;
}


////UDR empty interrupt
//ISR(USART_UDRE_vect){
//
//}

//Puts char to uart0
void uart0_putc(uint8_t c){
    //TODO: make transmission a task so that the MCU can sleep while
    //transmitting

    //wait until buffer is ready
    while(!(UCSR0A & _BV(UDRE0)));
    //transmit
    UDR0 = c;
}


void uart0_puts(const char* str){
    while(*str) uart0_putc(*str++);
}

//flush uart0 buffer
void uart0_flush(){
    _SAVE_SREG;
    uart0_rx_count=0;
    _RESTORE_SREG;
}

//read integer value from uart0
int32_t uart0_get_int(){
    int32_t result=0;
    //read until first num char
    bool neg=FALSE;
    char c;
    while(!is_num_char(c=uart0_getc())){
        if(c=='-')neg^=TRUE;
    }

    do{
        result*=10;
        result+=c-'0';
    } while(is_num_char(c=uart0_getc()));
    
    return neg?-result:result;
}

void uart0_put_int(int32_t value){
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
        uart0_putc(buf[idx]);
    }
}

float uart0_get_float(){
    float result=0;
    float frac=1;
    //read until first num char
    char c;
    bool neg=FALSE;
    while(!is_num_char(c=uart0_getc())){
        if(c=='-')neg^=TRUE;
    }

    do{
        result*=10;
        result+=c-'0';
    }while(is_num_char(c=uart0_getc()));

    //read frac part
    if(c=='.'){
        while(is_num_char(c=uart0_getc())){
            frac*=10;
            result+=(c-'0')/frac;
        }
    }
    return neg?-result:result;
}

void uart0_put_float(float value){
    int32_t ipart=value;
    uart0_put_int(ipart);
    value -= ipart;
    value=value<0?-value:value;
    if(value)uart0_putc('.');
    uint8_t dec_places=FLOAT_PRINT_DECIMAL_PLACES;
    while(value && (dec_places--)){
        value*=10;
        ipart = value;
        value -= ipart;
        //round up
        if(dec_places==0){
            ipart += value>=0.5;
            if(ipart>9)ipart=9;
        }
        uart0_putc(ipart+'0');
    }
}



#endif /* UART_H_ */