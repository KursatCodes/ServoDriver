#include <xc.h>
#include "uart.h"
// uart lib
void UART_Initialize()
{
    TXSTA=0b00100100;
    RCSTA=0b10010000;
    SPBRG=12;
    TXREG=0x00;
}
 
void UART_Write(char data)
{
  while(!TRMT);
  TXREG = data;
}
 
void UART_Write_Text(char *text)
{
  int i;
  for(i=0;text[i]!='\0';i++)
    UART_Write(text[i]);
}

// uart lib son 