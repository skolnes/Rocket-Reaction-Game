//
// GENG 421 - Tom Lupfer
//
// LCD module
//

#include <stdarg.h>
#include <string.h>

#include "geng421.h"
#include "lcd.h"

#define CHAR_PIXEL_WIDTH	6
#define DISP_CHAR_HEIGHT	4
#define DISP_CHAR_WIDTH		21
#define DISP_PIXEL_WIDTH	128

#define SCK_CLR()			(LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5))
#define SCK_SET()			(LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5))

#define RST_DIS()			(LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_6))
#define RST_ENA()			(LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_6))

#define MOSI_CLR()			(LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_7))
#define MOSI_SET()			(LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_7))

#define CMD_ENA()			(LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_8))
#define DAT_ENA()			(LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_8))

#define CS_DIS()			(LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_6))
#define CS_ENA()			(LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_6))

#define NUM_CHARS_MAX		32		// maximums - %b:32, %d:10, %x:8


static int	FieldWidth;
static Bool	IsLeftJustified = FALSE;
static Bool	IsMinWidth = FALSE;
static Bool	IsNegative;
static Bool	IsZeroFilled = FALSE;
static int	MinWidth;
static int	OutputCharCount;


static void LeftFill(void);
static void RightFill(void);
static void CharWrite(int CharVal);
static void CmdWrite(unsigned char Val);
static void DataWrite(unsigned char Val);


void LcdInit(void)
{
	int	CharRow;
	int PixelCol;
	int i;

	// Enable the clock for GPIO Port A
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

	// PA5 -> LCD SCK (MOSI valid on rising edge)
	// Init PA5 high, configure as an output
	LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_5, LL_GPIO_MODE_OUTPUT);

	// PA6 -> LCD RESET# (active low)
	// Init PA6 low, configure as an output
	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_6);
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_6, LL_GPIO_MODE_OUTPUT);

	// PA7 -> LCD MOSI (active high)
	// Init PA7 low, configure as an output
	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_7);
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT);

	// PA8 -> LCD D/C#
	// Init PA8 low, configure as an output
	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_8);
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);

	// Enable the clock for GPIO Port B
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	// PB6 -> LCD CS# (active low)
	// Init PB6 high, configure as an output
	LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_6);
	LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_6, LL_GPIO_MODE_OUTPUT);

	// Reset the LCD
	for (i = 0; i < 100; i++)	// hold reset line asserted
		;
	RST_DIS();					// deassert reset line
	for (i = 0; i < 100; i++)	// pause to come out of reset
		;

	// Initialize the LCD
	CmdWrite(0xA0);
	CmdWrite(0xAE);
	CmdWrite(0xC8);
	CmdWrite(0xA2);
	CmdWrite(0xA4);
	CmdWrite(0x2F);
	CmdWrite(0x21);
	CmdWrite(0x81);
	CmdWrite(0x1F);
	CmdWrite(0xAF);

	// Cleqr the display
	for (CharRow = 0; CharRow < DISP_CHAR_HEIGHT; CharRow++)
	{
		CmdWrite(0x10);
		CmdWrite(0x00);
		CmdWrite(0xB0 + CharRow);
		for (PixelCol = 0; PixelCol < DISP_PIXEL_WIDTH; PixelCol++)
		{
			DataWrite(0x00);
		}
	}
}


int LcdPrintf(int CharRow, int CharCol, char * FmtStrPtr, ...)
{
	va_list		ArgPtr;
	Bool		IsHexLower;
	char		NumCharArray[NUM_CHARS_MAX + 1];	// allow for null terminator
	int			NumCharArrayIdx;
	int			NumValCur;
	int			NumValNext;
	char *		StrPtr;

	// Keep Row and Col within range
	CharRow = (CharRow < 0) ? 0 : CharRow;
	CharRow = (CharRow < DISP_CHAR_HEIGHT) ? CharRow : DISP_CHAR_HEIGHT - 1;
	CharCol = (CharCol < 0) ? 0 : CharCol;
	CharCol = (CharCol < DISP_CHAR_WIDTH) ? CharCol : DISP_CHAR_WIDTH - 1;

	// Set up the row and column addresses in the LCD
	CmdWrite(0x10 + ((CharCol * CHAR_PIXEL_WIDTH) >> 4));
	CmdWrite(0x00 + ((CharCol * CHAR_PIXEL_WIDTH) & 0x0F));
	CmdWrite(0xB0 + CharRow);

	//Variable argument list start-up
	va_start(ArgPtr, FmtStrPtr);

	OutputCharCount = 0;

	NumCharArray[NUM_CHARS_MAX] = '\0';		// array will be filled right-to-left

	while(*FmtStrPtr)
	{
		// Print the format string verbatim until the start of a format specifier
		if(*FmtStrPtr != '%')
		{
			CharWrite(*FmtStrPtr++);
			OutputCharCount++;
			continue;
		}

		//
		// Format specifier handling
		//

		// Advance to the next char in the format string, reset flags
		FmtStrPtr++;
		MinWidth = 0;
		IsMinWidth = FALSE;
		IsLeftJustified = FALSE;
		IsNegative = FALSE;
		IsZeroFilled = FALSE;
		IsHexLower = FALSE;

		// Left justification
		if(*FmtStrPtr == '-')
		{
			IsLeftJustified = TRUE;
			FmtStrPtr++;
		}

		// Zero fill
		if(*FmtStrPtr == '0')
		{
			IsZeroFilled = TRUE;
			FmtStrPtr++;
		}

		// Minimum field width, in decimal
		while((*FmtStrPtr >= '0') && (*FmtStrPtr <= '9'))
		{
			IsMinWidth = TRUE;
			MinWidth = (MinWidth * 10) + (*FmtStrPtr - '0');
			FmtStrPtr++;
		}

		switch (*FmtStrPtr)
		{
		case 'b':
			NumCharArrayIdx = NUM_CHARS_MAX;
			NumValCur = va_arg(ArgPtr, int);

			do
			{
				NumValNext = (NumValCur >> 1) & 0x7FFFFFFF;	// mask stops propagation of msb=1 in a signed int
				NumCharArray[--NumCharArrayIdx] = '0' + (NumValCur & 0x00000001);
				NumValCur = NumValNext;
			} while (NumValCur);

			FieldWidth = NUM_CHARS_MAX - NumCharArrayIdx;
			LeftFill();
			while (NumCharArray[NumCharArrayIdx])
			{
				CharWrite(NumCharArray[NumCharArrayIdx++]);
				OutputCharCount++;
			}
			RightFill();
			break;
			break;

		case 'c':
			FieldWidth = 1;
			LeftFill();
			CharWrite(va_arg(ArgPtr, int));
			OutputCharCount++;
			RightFill();
			break;

		case 'd':
			NumCharArrayIdx = NUM_CHARS_MAX;
			NumValCur = va_arg(ArgPtr, int);
			if (NumValCur < 0)
			{
				NumValCur = -NumValCur;
				IsNegative = TRUE;
			}

			do
			{
				NumValNext = NumValCur / 10;
				NumCharArray[--NumCharArrayIdx] = '0' + (NumValCur - (10 * NumValNext));
				NumValCur = NumValNext;
			} while (NumValCur);

			FieldWidth = (NUM_CHARS_MAX - NumCharArrayIdx) + (IsNegative ? 1 : 0);
			LeftFill();
			while (NumCharArray[NumCharArrayIdx])
			{
				CharWrite(NumCharArray[NumCharArrayIdx++]);
				OutputCharCount++;
			}
			RightFill();
			break;

		case 's':
			StrPtr = va_arg(ArgPtr, char *);
			FieldWidth = strlen(StrPtr);
			LeftFill();
			while (*StrPtr)
			{
				CharWrite(*StrPtr++);
				OutputCharCount++;
			}
			RightFill();
			break;

		case 'x':
			IsHexLower = TRUE;
			// NOTE: lack of break is intentional, flow to next case

		case 'X':
			NumCharArrayIdx = NUM_CHARS_MAX;
			NumValCur = va_arg(ArgPtr, int);

			do
			{
				NumValNext = (NumValCur >> 4) & 0x0FFFFFFF;	// mask stops propagation of msb=1 in a signed int
				NumValCur &= 0x0000000F;					// mask off the least significant hex digit
				if (NumValCur < 10)
				{
					NumCharArray[--NumCharArrayIdx] = '0' + NumValCur;
				}
				else
				{
					NumCharArray[--NumCharArrayIdx] = (IsHexLower ? 'a' : 'A') + (NumValCur - 10);
				}
				NumValCur = NumValNext;
			} while (NumValCur);

			FieldWidth = NUM_CHARS_MAX - NumCharArrayIdx;
			LeftFill();
			while (NumCharArray[NumCharArrayIdx])
			{
				CharWrite(NumCharArray[NumCharArrayIdx++]);
				OutputCharCount++;
			}
			RightFill();
			break;

		case '%':
			FieldWidth = 1;
			LeftFill();
			CharWrite('%');
			OutputCharCount++;
			RightFill();
			break;

		default:
			CharWrite('%');
			CharWrite(*FmtStrPtr++);
			OutputCharCount += 2;
			break;
		}
		FmtStrPtr++;
	}

	// Variable argument list clean-up
	va_end(ArgPtr);

	return OutputCharCount;
}


static void LeftFill(void)
{
	int	FillCharCount = MinWidth - FieldWidth;

	if (!IsMinWidth || IsLeftJustified || (FillCharCount < 0))
	{
		if (IsNegative)
		{
			CharWrite('-');
			OutputCharCount++;
		}
		return;
	}

	if (IsNegative && IsZeroFilled)
	{
		CharWrite('-');
		OutputCharCount++;
	}

	while (FillCharCount > 0)
	{
		CharWrite(IsZeroFilled ? '0' : ' ');
		OutputCharCount++;
		FillCharCount--;
	}

	if (IsNegative && !IsZeroFilled)
	{
		CharWrite('-');
		OutputCharCount++;
	}
}


static void RightFill(void)
{
	int	FillCharCount = MinWidth - FieldWidth;

	if (!IsMinWidth || !IsLeftJustified || (FillCharCount < 0))
	{
		return;
	}

	while (FillCharCount > 0)
	{
		CharWrite(' ');
		OutputCharCount++;
		FillCharCount--;
	}
}


static void CharWrite(int CharVal)
{
	int CharCol;

	//  Array containing a 5x7 font, top-left justified in a 6x8 cell.
	//  Non-printable characters (0x00-0x1f) and the characters 0x80-0xff
	//  have a space substituted for them.
	//  Each character bitmap consists of 6 bytes, with the first byte
	//  representing the left-most column of the 6x8 cell.
	//  The lsb of each byte represents the top-most bit of the column.
	static const unsigned char FontArray[] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x00  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x01  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x02  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x03  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x04  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x05  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x06  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x07  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x08  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x09  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x0a  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x0b  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x0c  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x0d  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x0e  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x0f  Char: ' ' */

		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x10  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x11  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x12  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x13  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x14  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x15  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x16  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x17  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x18  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x19  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x1a  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x1b  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x1c  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x1d  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x1e  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x1f  Char: ' ' */

		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x20  Char: ' ' */
		0x00, 0x00, 0x4f, 0x00, 0x00, 0x00,		/* Value: 0x21  Char: '!' */
		0x00, 0x07, 0x00, 0x07, 0x00, 0x00,		/* Value: 0x22  Char: '"' */
		0x14, 0x7f, 0x14, 0x7f, 0x14, 0x00,		/* Value: 0x23  Char: '#' */
		0x24, 0x2a, 0x7f, 0x2a, 0x12, 0x00,		/* Value: 0x24  Char: '$' */
		0x23, 0x13, 0x08, 0x64, 0x62, 0x00,		/* Value: 0x25  Char: '%' */
		0x36, 0x49, 0x55, 0x22, 0x50, 0x00,		/* Value: 0x26  Char: '&' */
		0x00, 0x05, 0x03, 0x00, 0x00, 0x00,		/* Value: 0x27  Char: ''' */
		0x00, 0x1c, 0x22, 0x41, 0x00, 0x00,		/* Value: 0x28  Char: '(' */
		0x00, 0x41, 0x22, 0x1c, 0x00, 0x00,		/* Value: 0x29  Char: ')' */
		0x14, 0x08, 0x3e, 0x08, 0x14, 0x00,		/* Value: 0x2a  Char: '*' */
		0x08, 0x08, 0x3e, 0x08, 0x08, 0x00,		/* Value: 0x2b  Char: '+' */
		0x00, 0x50, 0x30, 0x00, 0x00, 0x00,		/* Value: 0x2c  Char: ',' */
		0x08, 0x08, 0x08, 0x08, 0x08, 0x00,		/* Value: 0x2d  Char: '-' */
		0x00, 0x60, 0x60, 0x00, 0x00, 0x00,		/* Value: 0x2e  Char: '.' */
		0x20, 0x10, 0x08, 0x04, 0x02, 0x00,		/* Value: 0x2f  Char: '/' */

		0x3e, 0x51, 0x49, 0x45, 0x3e, 0x00,		/* Value: 0x30  Char: '0' */
		0x00, 0x42, 0x7f, 0x40, 0x00, 0x00,		/* Value: 0x31  Char: '1' */
		0x42, 0x61, 0x51, 0x49, 0x46, 0x00,		/* Value: 0x32  Char: '2' */
		0x21, 0x41, 0x45, 0x4b, 0x31, 0x00,		/* Value: 0x33  Char: '3' */
		0x18, 0x14, 0x12, 0x7f, 0x10, 0x00,		/* Value: 0x34  Char: '4' */
		0x27, 0x45, 0x45, 0x45, 0x39, 0x00,		/* Value: 0x35  Char: '5' */
		0x3c, 0x4a, 0x49, 0x49, 0x30, 0x00,		/* Value: 0x36  Char: '6' */
		0x01, 0x71, 0x09, 0x05, 0x03, 0x00,		/* Value: 0x37  Char: '7' */
		0x36, 0x49, 0x49, 0x49, 0x36, 0x00,		/* Value: 0x38  Char: '8' */
		0x06, 0x49, 0x49, 0x29, 0x1e, 0x00,		/* Value: 0x39  Char: '9' */
		0x00, 0x00, 0x36, 0x36, 0x00, 0x00,		/* Value: 0x3a  Char: ':' */
		0x00, 0x56, 0x36, 0x00, 0x00, 0x00,		/* Value: 0x3b  Char: ';' */
		0x08, 0x14, 0x22, 0x41, 0x00, 0x00,		/* Value: 0x3c  Char: '<' */
		0x14, 0x14, 0x14, 0x14, 0x14, 0x00,		/* Value: 0x3d  Char: '=' */
		0x41, 0x22, 0x14, 0x08, 0x00, 0x00,		/* Value: 0x3e  Char: '>' */
		0x02, 0x01, 0x51, 0x09, 0x06, 0x00,		/* Value: 0x3f  Char: '?' */

		0x32, 0x49, 0x79, 0x41, 0x3e, 0x00,		/* Value: 0x40  Char: '@' */
		0x7e, 0x11, 0x11, 0x11, 0x7e, 0x00,		/* Value: 0x41  Char: 'A' */
		0x7f, 0x49, 0x49, 0x49, 0x36, 0x00,		/* Value: 0x42  Char: 'B' */
		0x3e, 0x41, 0x41, 0x41, 0x22, 0x00,		/* Value: 0x43  Char: 'C' */
		0x7f, 0x41, 0x41, 0x22, 0x1c, 0x00,		/* Value: 0x44  Char: 'D' */
		0x7f, 0x49, 0x49, 0x49, 0x41, 0x00,		/* Value: 0x45  Char: 'E' */
		0x7f, 0x09, 0x09, 0x09, 0x01, 0x00,		/* Value: 0x46  Char: 'F' */
		0x3e, 0x41, 0x49, 0x49, 0x7a, 0x00,		/* Value: 0x47  Char: 'G' */
		0x7f, 0x08, 0x08, 0x08, 0x7f, 0x00,		/* Value: 0x48  Char: 'H' */
		0x00, 0x41, 0x7f, 0x41, 0x00, 0x00,		/* Value: 0x49  Char: 'I' */
		0x20, 0x40, 0x41, 0x3f, 0x01, 0x00,		/* Value: 0x4a  Char: 'J' */
		0x7f, 0x08, 0x14, 0x22, 0x41, 0x00,		/* Value: 0x4b  Char: 'K' */
		0x7f, 0x40, 0x40, 0x40, 0x40, 0x00,		/* Value: 0x4c  Char: 'L' */
		0x7f, 0x02, 0x0c, 0x02, 0x7f, 0x00,		/* Value: 0x4d  Char: 'M' */
		0x7f, 0x04, 0x08, 0x10, 0x7f, 0x00,		/* Value: 0x4e  Char: 'N' */
		0x3e, 0x41, 0x41, 0x41, 0x3e, 0x00,		/* Value: 0x4f  Char: 'O' */

		0x7f, 0x09, 0x09, 0x09, 0x06, 0x00,		/* Value: 0x50  Char: 'P' */
		0x3e, 0x41, 0x51, 0x21, 0x5e, 0x00,		/* Value: 0x51  Char: 'Q' */
		0x7f, 0x09, 0x19, 0x29, 0x46, 0x00,		/* Value: 0x52  Char: 'R' */
		0x46, 0x49, 0x49, 0x49, 0x31, 0x00,		/* Value: 0x53  Char: 'S' */
		0x01, 0x01, 0x7f, 0x01, 0x01, 0x00,		/* Value: 0x54  Char: 'T' */
		0x3f, 0x40, 0x40, 0x40, 0x3f, 0x00,		/* Value: 0x55  Char: 'U' */
		0x1f, 0x20, 0x40, 0x20, 0x1f, 0x00,		/* Value: 0x56  Char: 'V' */
		0x3f, 0x40, 0x38, 0x40, 0x3f, 0x00,		/* Value: 0x57  Char: 'W' */
		0x63, 0x14, 0x08, 0x14, 0x63, 0x00,		/* Value: 0x58  Char: 'X' */
		0x07, 0x08, 0x70, 0x08, 0x07, 0x00,		/* Value: 0x59  Char: 'Y' */
		0x61, 0x51, 0x49, 0x45, 0x43, 0x00,		/* Value: 0x5a  Char: 'Z' */
		0x00, 0x7f, 0x41, 0x41, 0x00, 0x00,		/* Value: 0x5b  Char: '[' */
		0x02, 0x04, 0x08, 0x10, 0x20, 0x00,		/* Value: 0x5c  Char: '\' */
		0x00, 0x41, 0x41, 0x7f, 0x00, 0x00,		/* Value: 0x5d  Char: ']' */
		0x04, 0x02, 0x01, 0x02, 0x04, 0x00,		/* Value: 0x5e  Char: '^' */
		0x40, 0x40, 0x40, 0x40, 0x40, 0x00,		/* Value: 0x5f  Char: '_' */

		0x00, 0x01, 0x02, 0x04, 0x00, 0x00,		/* Value: 0x60  Char: '`' */
		0x20, 0x54, 0x54, 0x54, 0x78, 0x00,		/* Value: 0x61  Char: 'a' */
		0x7f, 0x48, 0x44, 0x44, 0x38, 0x00,		/* Value: 0x62  Char: 'b' */
		0x38, 0x44, 0x44, 0x44, 0x20, 0x00,		/* Value: 0x63  Char: 'c' */
		0x38, 0x44, 0x44, 0x48, 0x7f, 0x00,		/* Value: 0x64  Char: 'd' */
		0x38, 0x54, 0x54, 0x54, 0x18, 0x00,		/* Value: 0x65  Char: 'e' */
		0x08, 0x7e, 0x09, 0x01, 0x02, 0x00,		/* Value: 0x66  Char: 'f' */
		0x0c, 0x52, 0x52, 0x52, 0x3e, 0x00,		/* Value: 0x67  Char: 'g' */
		0x7f, 0x08, 0x04, 0x04, 0x78, 0x00,		/* Value: 0x68  Char: 'h' */
		0x00, 0x44, 0x7d, 0x40, 0x00, 0x00,		/* Value: 0x69  Char: 'i' */
		0x20, 0x40, 0x44, 0x3d, 0x00, 0x00,		/* Value: 0x6a  Char: 'j' */
		0x7f, 0x10, 0x28, 0x44, 0x00, 0x00,		/* Value: 0x6b  Char: 'k' */
		0x00, 0x41, 0x7f, 0x40, 0x00, 0x00,		/* Value: 0x6c  Char: 'l' */
		0x7c, 0x04, 0x18, 0x04, 0x78, 0x00,		/* Value: 0x6d  Char: 'm' */
		0x7c, 0x08, 0x04, 0x04, 0x78, 0x00,		/* Value: 0x6e  Char: 'n' */
		0x38, 0x44, 0x44, 0x44, 0x38, 0x00,		/* Value: 0x6f  Char: 'o' */

		0x7c, 0x14, 0x14, 0x14, 0x08, 0x00,		/* Value: 0x70  Char: 'p' */
		0x08, 0x14, 0x14, 0x18, 0x7c, 0x00,		/* Value: 0x71  Char: 'q' */
		0x7c, 0x08, 0x04, 0x04, 0x08, 0x00,		/* Value: 0x72  Char: 'r' */
		0x48, 0x54, 0x54, 0x54, 0x20, 0x00,		/* Value: 0x73  Char: 's' */
		0x04, 0x3f, 0x44, 0x40, 0x20, 0x00,		/* Value: 0x74  Char: 't' */
		0x3c, 0x40, 0x40, 0x20, 0x7c, 0x00,		/* Value: 0x75  Char: 'u' */
		0x1c, 0x20, 0x40, 0x20, 0x1c, 0x00,		/* Value: 0x76  Char: 'v' */
		0x3c, 0x40, 0x30, 0x40, 0x3c, 0x00,		/* Value: 0x77  Char: 'w' */
		0x44, 0x28, 0x10, 0x28, 0x44, 0x00,		/* Value: 0x78  Char: 'x' */
		0x0c, 0x50, 0x50, 0x50, 0x3c, 0x00,		/* Value: 0x79  Char: 'y' */
		0x44, 0x64, 0x54, 0x4c, 0x44, 0x00,		/* Value: 0x7a  Char: 'z' */
		0x00, 0x00, 0x08, 0x36, 0x41, 0x00,		/* Value: 0x7b  Char: '{' */
		0x00, 0x00, 0x7f, 0x00, 0x00, 0x00,		/* Value: 0x7c  Char: '|' */
		0x00, 0x41, 0x36, 0x08, 0x00, 0x00,		/* Value: 0x7d  Char: '}' */
		0x08, 0x08, 0x2a, 0x1c, 0x08, 0x00,		/* Value: 0x7e  Char: '->'*/
		0x08, 0x1c, 0x2a, 0x08, 0x08, 0x00,		/* Value: 0x7f  Char: '<-' */

		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x80  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x81  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x82  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x83  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x84  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x85  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x86  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x87  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x88  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x89  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x8a  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x8b  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x8c  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x8d  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x8e  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x8f  Char: ' ' */

		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x90  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x91  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x92  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x93  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x94  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x95  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x96  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x97  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x98  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x99  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x9a  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x9b  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x9c  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x9d  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x9e  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0x9f  Char: ' ' */

		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xa0  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xa1  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xa2  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xa3  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xa4  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xa5  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xa6  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xa7  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xa8  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xa9  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xaa  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xab  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xac  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xad  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xae  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xaf  Char: ' ' */

		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xb0  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xb1  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xb2  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xb3  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xb4  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xb5  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xb6  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xb7  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xb8  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xb9  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xba  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xbb  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xbc  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xbd  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xbe  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xbf  Char: ' ' */

		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xc0  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xc1  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xc2  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xc3  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xc4  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xc5  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xc6  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xc7  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xc8  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xc9  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xca  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xcb  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xcc  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xcd  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xce  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xcf  Char: ' ' */

		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xd0  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xd1  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xd2  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xd3  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xd4  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xd5  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xd6  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xd7  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xd8  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xd9  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xda  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xdb  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xdc  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xdd  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xde  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xdf  Char: ' ' */

		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xe0  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xe1  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xe2  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xe3  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xe4  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xe5  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xe6  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xe7  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xe8  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xe9  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xea  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xeb  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xec  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xed  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xee  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xef  Char: ' ' */

		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xf0  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xf1  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xf2  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xf3  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xf4  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xf5  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xf6  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xf7  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xf8  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xf9  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xfa  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xfb  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xfc  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xfd  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xfe  Char: ' ' */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		/* Value: 0xff  Char: ' ' */
	};

	for (CharCol = 0; CharCol < CHAR_PIXEL_WIDTH; CharCol++)
	{
		DataWrite(FontArray[(CharVal * CHAR_PIXEL_WIDTH) + CharCol]);
	}
}


static void CmdWrite(unsigned char Val)
{
	unsigned int BitMask;

	CMD_ENA();

	CS_ENA();

	for (BitMask = 0x80; BitMask > 0; BitMask >>= 1)
	{
		SCK_CLR();
		if (Val & BitMask)
		{
			MOSI_SET();
		}
		else
		{
			MOSI_CLR();
		}
		SCK_SET();
	}

	CS_DIS();
}


static void DataWrite(unsigned char Val)
{
	unsigned int BitMask;

	DAT_ENA();

	CS_ENA();

	for (BitMask = 0x80; BitMask > 0; BitMask >>= 1)
	{
		SCK_CLR();
		if (Val & BitMask)
		{
			MOSI_SET();
		}
		else
		{
			MOSI_CLR();
		}
		SCK_SET();
	}

	CS_DIS();
}
