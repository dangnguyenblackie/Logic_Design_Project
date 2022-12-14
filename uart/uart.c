#include "uart.h"
#include "..\timer\timer.h"
#include "..\lcd\lcd.h"

#define INIT_RECEIVE		0
#define WAIT_HEADER_1		1
#define WAIT_HEADER_2		2
#define RECEIVE_DATA		3
#define END_OF_RECEIVE_1	4
#define END_OF_RECEIVE_2	5

unsigned char i;
unsigned char statusReceive = INIT_RECEIVE;
unsigned char flagOfDataReceiveComplete = 0;
unsigned char indexOfMachine = 0;
unsigned char indexOfDataReceive = 0;
unsigned char indexOfdataSend = 0;
unsigned char numberOfDataReceive = 6;
unsigned char dataReceive [50];
unsigned char numberOfDataSend = 5;
unsigned char dataSend [50];

void init_uart()
{

	SPBRGH = 0;  //Baud Rate Generator Register 
	//Baud Rate =  FOSC/(64 ([SPBRGH:SPBRG] + 1)) 
	//when SYNC = 0, BRG16 = 1, BRGH = 0 => Baud Rate = F/[16(SPBRG + 1)]
        // => SPBRG = F/(16*Baud Rate) - 1
        // F = 20,000,000
	SPBRG = 129; //9600
        //SPBRG = 64; //19200
	TXSTAbits.TXEN = 1; //Transmit enabled
	TXSTAbits.BRGH = 0; //Low Speed
	BAUDCONbits.BRG16 = 1; //16-bit Baud Rate Generator

	TXSTAbits.SYNC = 0; //Asynchronous mode
	RCSTAbits.SPEN = 1; //Enables
	RCSTAbits.CREN = 1; //Enables receive

	PIR1bits.RCIF = 0;  // clear interrupt flag uart
	PIE1bits.RCIE = 1;	//enable interrupt uart
	IPR1bits.RCIP = 0;	// Priority: low
	TRISCbits.RC6 = 0; // RC6 output
	TRISCbits.RC7 = 1; // RC7 input
}
void uart_delay_ms(int value)
{
	int i,j;
	for(i=0;i<value;i++)
		for(j=0;j<238;j++);
}
void uart_putchar(unsigned char data)
{
	while(PIR1bits.TXIF == 0);
        TXREG = data;
	PIR1bits.TXIF = 0;
}

void uart_send_str(const char *str)
{
	while(*str)
	{
		uart_putchar(*str);
		*str++;
	}
}

void UartSendString(const rom char *str)
{
	while(*str)
	{
		uart_putchar(*str);
		*str++;
	}
}

void uart_isr_simulate_machine()
{
    unsigned char tempReceive;
    tempReceive = RCREG;
    switch(statusReceive)
    {
        case INIT_RECEIVE:
        case WAIT_HEADER_1:
            if (tempReceive == 0x21)
            {
                indexOfDataReceive = 0;
                statusReceive = WAIT_HEADER_2;
            }
            break;
        case WAIT_HEADER_2:
            if (tempReceive == 0x21)
                statusReceive = RECEIVE_DATA;
            else
                statusReceive = WAIT_HEADER_1;
            break;
        case RECEIVE_DATA:
            dataReceive [indexOfDataReceive] = tempReceive;
            indexOfDataReceive ++;
            if (indexOfDataReceive >= numberOfDataReceive)
                statusReceive = END_OF_RECEIVE_1;

            break;
        case END_OF_RECEIVE_1:
            if (tempReceive == 0x23)
                statusReceive = END_OF_RECEIVE_2;
            else
                statusReceive = WAIT_HEADER_1;
            break;
        case END_OF_RECEIVE_2:
            if (tempReceive == 0x23)
            {
                flagOfDataReceiveComplete = 1;
                statusReceive = INIT_RECEIVE;
            }
            else
                statusReceive = WAIT_HEADER_1;
            break;
        default:
            statusReceive = INIT_RECEIVE;
            break;
    }
}

void uart_isr()
{
    uart_isr_simulate_machine();
}

void UartSendDataGraph(int value_1,int value_2,int value_3,int value_4)
{
    uart_putchar(0xff);
    uart_putchar(0xff);
    uart_putchar(value_1/256);
    uart_putchar(value_1%256);
    uart_putchar(value_2/256);
    uart_putchar(value_2%256);
    uart_putchar(value_3/256);
    uart_putchar(value_3%256);
    uart_putchar(value_4/256);
    uart_putchar(value_4%256);
    uart_putchar(0xfe);
    uart_putchar(0xfe);
}

void UartSendNumToString(long num)
{
    char num_flag = 0;
    char i;

    if(num == 0) {
            uart_putchar('0');
            return;
    }
    if(num < 0) {
            uart_putchar('-');
            num *= -1;
    }
    //else
    //	uart_putchar(' ');

    for(i = 10; i > 0; i--) {
            if((num / uart_power_of(10, i-1)) != 0) {
                    num_flag = 1;
                    uart_putchar(num/uart_power_of(10, i-1) + '0');
            }
            else {
                    if(num_flag != 0)
                            uart_putchar('0');
            }
            num %= uart_power_of(10, i-1);
    }
}
unsigned long uart_power_of(int A, int x)
{
    char i;
    unsigned long temp = 1;
    for(i = 0; i < x; i++)
            temp *= A;
    return temp;
}

