
// GENG 421 - Scott Kolnes
//
// UART module header file
//

#ifndef UART_H_
#define UART_H_

void UartInit(void);
Bool UartCharAvail(void);
char UartCharRead(void);
void UartCharWrite(char OutChar);
void USART2_IRQHandler(void);

#endif	// UART_H_
