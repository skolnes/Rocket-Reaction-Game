/*
 * colors.c
 *
 *  Created on: Mar 6, 2018
 *      Author: Scott Kolnes
 */

#include "colors.h"
#include "rgb.h"
#include "geng421.h"


#define PWM_MIN 0
#define PWM_MAX 10

#define RED_UP		1
#define RED_DOWN	2
#define GRN_UP		3
#define GRN_DOWN	4
#define BLU_UP		5
#define BLU_DOWN	6
#define YELLOW_UP	7
#define YELLOW_DOWN	8
#define TURQ_UP		9
#define TURQ_DOWN	10
#define PURPLE_UP	11
#define PURPLE_DOWN	12
#define WHITE_UP	13
#define WHITE_DOWN	14


#define DLY_100MS 100


void ColorsClock(void)
{
	static int ColorsState = RED_UP;
	static int Delay = DLY_100MS;
	static int PWMSetting = PWM_MIN;

	switch(ColorsState)
	{

	case RED_UP:
			if(--Delay == 0)
			{
				Delay = DLY_100MS; //reset the delay counter
				if(++PWMSetting == PWM_MAX)
				{
					ColorsState = RED_DOWN;
				}
				RgbRedPwmSet(PWMSetting);
			}
			break;

	case RED_DOWN:
			if(--Delay == 0)
			{
				Delay = DLY_100MS; //reset the delay counter
				if(--PWMSetting == PWM_MIN)
				{
					ColorsState = GRN_UP;
				}
				RgbRedPwmSet(PWMSetting);
			}
			break;

	case GRN_UP:
			if(--Delay == 0)
			{
				Delay = DLY_100MS; //reset the delay counter
				if(++PWMSetting == PWM_MAX)
				{
					ColorsState = GRN_DOWN;
				}
				RgbGreenPwmSet(PWMSetting);
			}
			break;

	case GRN_DOWN:
			if(--Delay == 0)
			{
				Delay = DLY_100MS; //reset the delay counter
				if(--PWMSetting == PWM_MIN)
				{
					ColorsState = BLU_UP;
				}
				RgbGreenPwmSet(PWMSetting);
			}
			break;

	case BLU_UP:
			if(--Delay == 0)
			{
				Delay = DLY_100MS; //reset the delay counter
				if(++PWMSetting == PWM_MAX)
				{
					ColorsState = BLU_DOWN;
				}
				RgbBluePwmSet(PWMSetting);
			}
			break;

	case BLU_DOWN:
			if(--Delay == 0)
			{
				Delay = DLY_100MS; //reset the delay counter
				if(--PWMSetting == PWM_MIN)
				{
					ColorsState = YELLOW_UP;
				}
				RgbBluePwmSet(PWMSetting);
			}
			break;

	case YELLOW_UP:
			if(--Delay == 0)
			{
				Delay = DLY_100MS; //reset the delay counter
				if(++PWMSetting == PWM_MAX)
				{
					ColorsState = YELLOW_DOWN;
				}
				RgbRedPwmSet(PWMSetting);
				RgbGreenPwmSet(PWMSetting);
			}
			break;

	case YELLOW_DOWN:
			if(--Delay == 0)
			{
				Delay = DLY_100MS; //reset the delay counter
				if(--PWMSetting == PWM_MIN)
				{
					ColorsState = TURQ_UP;
				}
				RgbRedPwmSet(PWMSetting);
				RgbGreenPwmSet(PWMSetting);
			}
			break;

	case TURQ_UP:
			if(--Delay == 0)
			{
				Delay = DLY_100MS; //reset the delay counter
				if(++PWMSetting == PWM_MAX)
				{
					ColorsState = TURQ_DOWN;
				}
				RgbBluePwmSet(PWMSetting);
				RgbGreenPwmSet(PWMSetting);
			}
			break;

	case TURQ_DOWN:
			if(--Delay == 0)
			{
				Delay = DLY_100MS; //reset the delay counter
				if(--PWMSetting == PWM_MIN)
				{
					ColorsState = PURPLE_UP;
				}
				RgbBluePwmSet(PWMSetting);
				RgbGreenPwmSet(PWMSetting);
			}
			break;

	case PURPLE_UP:
			if(--Delay == 0)
			{
				Delay = DLY_100MS; //reset the delay counter
				if(++PWMSetting == PWM_MAX)
				{
					ColorsState = PURPLE_DOWN;
				}
				RgbBluePwmSet(PWMSetting);
				RgbRedPwmSet(PWMSetting);
			}
			break;

	case PURPLE_DOWN:
				if(--Delay == 0)
				{
					Delay = DLY_100MS; //reset the delay counter
					if(--PWMSetting == PWM_MIN)
					{
						ColorsState = WHITE_UP;
					}
					RgbBluePwmSet(PWMSetting);
					RgbRedPwmSet(PWMSetting);
				}
				break;

	case WHITE_UP:
				if(--Delay == 0)
				{
					Delay = DLY_100MS; //reset the delay counter
					if(++PWMSetting == PWM_MAX)
					{
						ColorsState = WHITE_DOWN;
					}
					RgbBluePwmSet(PWMSetting);
					RgbRedPwmSet(PWMSetting);
					RgbGreenPwmSet(PWMSetting);
				}
				break;

	case WHITE_DOWN:
				if(--Delay == 0)
				{
					Delay = DLY_100MS; //reset the delay counter
					if(--PWMSetting == PWM_MIN)
					{
						ColorsState = RED_UP;
					}
					RgbBluePwmSet(PWMSetting);
					RgbRedPwmSet(PWMSetting);
					RgbGreenPwmSet(PWMSetting);
				}
				break;
	}
}




