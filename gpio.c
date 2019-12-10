//
// GENG 421 - Scott Kolnes
//
// GPIO module
//

#include "geng421.h"
#include "gpio.h"


void GpioInit(void)
{
	// Enable the clock for GPIO Port A
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

	// PA5 -> debug (active high)
	// Init PA5 low, configure as an output
	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_5, LL_GPIO_MODE_OUTPUT);

	// PA9 -> RGB blue LED (active low)
	// Init PA9 high, configure as an output
	LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_9);
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);


	// Enable the clock for GPIO Port B
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	// PB4 -> RGB red LED (active low)
	// Init PB4 high, configure as an output
	LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_4);
	LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_4, LL_GPIO_MODE_OUTPUT);


	// Enable the clock for GPIO Port C
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);

	// PC7 -> RGB green LED (active low)
	// Init PBC7 high, configure as an output
	LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_7);
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_7, LL_GPIO_MODE_OUTPUT);
}
