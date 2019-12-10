//
// GENG 421 - Tom Lupfer
//
// LCD module header file
//

#ifndef LCD_H_
#define LCD_H_

void LcdInit(void);
int LcdPrintf(int CharRow, int CharCol, char * FmtStrPtr, ...);

#endif	// LCD_H_
