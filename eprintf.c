//
// GENG 421 - Scott Kolnes
//
// Compact embedded printf() module
//

#include <stdarg.h>
#include <string.h>

#include "geng421.h"
#include "eprintf.h"
#include "uart.h"


#define NUM_CHARS_MAX		32		// maximums - %b:32, %d:10, %x:8


static int	FieldWidth;
static Bool	IsLeftJustified = FALSE;
static Bool	IsMinWidth = FALSE;
static Bool	IsNegative;
static Bool	IsZeroFilled = FALSE;
static int	MinWidth;
static int	OutputCharCount;


void LeftFill(void);
void RightFill(void);


int eprintf(char * FmtStrPtr, ...)
{
	va_list		ArgPtr;
	Bool		IsHexLower;
	char		NumCharArray[NUM_CHARS_MAX + 1];	// allow for null terminator
	int			NumCharArrayIdx;
	int			NumValCur;
	int			NumValNext;
	char *		StrPtr;

	//Variable argument list start-up
	va_start(ArgPtr, FmtStrPtr);

	OutputCharCount = 0;

	NumCharArray[NUM_CHARS_MAX] = '\0';		// array will be filled right-to-left

	while(*FmtStrPtr)
	{
		// Print the format string verbatim until the start of a format specifier
		if(*FmtStrPtr != '%')
		{
			UartCharWrite(*FmtStrPtr++);
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
				NumValCur &= 0x00000001;					// mask off the least significant hex digit
				NumCharArray[--NumCharArrayIdx] = '0' + NumValCur;
				NumValCur = NumValNext;
			} while (NumValCur);

			FieldWidth = NUM_CHARS_MAX - NumCharArrayIdx;
			LeftFill();
			while (NumCharArray[NumCharArrayIdx])
			{
				UartCharWrite(NumCharArray[NumCharArrayIdx++]);
				OutputCharCount++;
			}
			RightFill();
			break;
		case 'c':
			FieldWidth = 1;
			LeftFill();
			UartCharWrite(va_arg(ArgPtr, int));
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
				UartCharWrite(NumCharArray[NumCharArrayIdx++]);
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
				UartCharWrite(*StrPtr++);
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
				UartCharWrite(NumCharArray[NumCharArrayIdx++]);
				OutputCharCount++;
			}
			RightFill();
			break;

		case '%':
			FieldWidth = 1;
			LeftFill();
			UartCharWrite('%');
			OutputCharCount++;
			RightFill();
			break;

		default:
			UartCharWrite('%');
			UartCharWrite(*FmtStrPtr++);
			OutputCharCount += 2;
			break;
		}
		FmtStrPtr++;
	}

	// Variable argument list clean-up
	va_end(ArgPtr);

	return OutputCharCount;
}


void LeftFill(void)
{
	int	FillCharCount = MinWidth - FieldWidth;

	if (!IsMinWidth || IsLeftJustified || (FillCharCount < 0))
	{
		if (IsNegative)
		{
			UartCharWrite('-');
			OutputCharCount++;
		}
		return;
	}

	if (IsNegative && IsZeroFilled)
	{
		UartCharWrite('-');
		OutputCharCount++;
	}

	while (FillCharCount > 0)
	{
		UartCharWrite(IsZeroFilled ? '0' : ' ');
		OutputCharCount++;
		FillCharCount--;
	}

	if (IsNegative && !IsZeroFilled)
	{
		UartCharWrite('-');
		OutputCharCount++;
	}
}


void RightFill(void)
{
	int	FillCharCount = MinWidth - FieldWidth;

	if (!IsMinWidth || !IsLeftJustified || (FillCharCount < 0))
	{
		return;
	}

	while (FillCharCount > 0)
	{
		UartCharWrite(' ');
		OutputCharCount++;
		FillCharCount--;
	}
}
