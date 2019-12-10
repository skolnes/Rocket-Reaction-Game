/*
 * main.c
 *
 *  Created on: May 1, 2018
 *      Author: Scott Kolnes
 */

#include <stdio.h>
#include "geng421.h"
#include "clock.h"
#include "colors.h"
#include "eprintf.h"
#include "gpio.h"
#include "rgb.h"
#include "uart.h"
#include "nav.h"
#include "game.h"
#include "lcd.h"


int main(void)
{
	// Initialize hardware
	ClockInit();
	GpioInit();
	UartInit();
	NavInit();
	LcdInit();

	// Main loop
	while (TRUE)
	{
		// Wait for the 1msec tick timer
		ClockWaitForTick();

		// Clock the state machines
		NavClock();
		//RgbClock();
		ColorsClock();
		GameClock();

	} // End - main loop
}
