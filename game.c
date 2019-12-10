/*
 * game.c
 *
 *  Created on: May 1, 2018
 *      Author: Scott Kolnes
 */

#include "geng421.h"
#include "game.h"
#include "rgb.h"
#include "eprintf.h"
#include "stdlib.h"
#include "nav.h"
#include "lcd.h"

#define READY_STATE			0
#define WAIT_STATE			1
#define WATCH_STATE			2
#define CORRECT_STATE		3
#define INCORRECT_STATE		4
#define RESTART_STATE		5

#define RAND_DELAY			rand() & 0xFFF // does a random value between 1000 and 5000
#define END_DELAY			3000

void GameClock(void)
{
	static int GameState = RESTART_STATE;
	static int TimeCounter;
	static int DelayCount;
	static int RandomDirection;
	LcdPrintf(0,0,"Rocket Reaction Timer");

	switch(GameState)
		{
	case READY_STATE:
		if (NavSelectIsPressed())
			{
				RGB_BLU_DIS();
				RGB_RED_ENA();
				RGB_GRN_ENA();

				//eprintf("Get Ready!!\n\n");
				LcdPrintf(2,0,"                     ");
				LcdPrintf(2,0,"Get Ready!!");
				DelayCount = RAND_DELAY;
				GameState = WAIT_STATE;
			}
			break;

	case WAIT_STATE:
		if(--DelayCount == 0)
			{
				RGB_RED_DIS();
				RGB_GRN_DIS();
				//eprintf("Direction is: \n");
				LcdPrintf(2,0,"                     ");
				LcdPrintf(2,0,"Direction is: ");
				RandomDirection = rand() % 4;
				if(RandomDirection == 0)
				{
					//eprintf("UP\n\n");
					LcdPrintf(2,14,"UP");
				}
				if(RandomDirection == 1)
				{
					//eprintf("DOWN\n\n");
					LcdPrintf(2,14,"DOWN");
				}
				if(RandomDirection == 2)
				{
					//eprintf("LEFT\n\n");
					LcdPrintf(2,14,"LEFT");
				}
				if(RandomDirection == 3)
				{
					//eprintf("Right\n\n");
					LcdPrintf(2,14,"RIGHT");
				}
				TimeCounter = 0;
				GameState = WATCH_STATE;
			}
		break;

	case WATCH_STATE:
		if(RandomDirection == 0)
		{
			if(NavUpIsPressed())
			{
				RGB_GRN_ENA();
				//eprintf("That is Correct!!\n");
				LcdPrintf(2,0,"                     ");
				LcdPrintf(2,0,"That is Correct!!");
				//eprintf("Your Reaction Time in miliseconds is : %d milliseconds!\n", TimeCounter);
				LcdPrintf(3,0,"Reaction Time: %d ms", TimeCounter);
				GameState = CORRECT_STATE;
				DelayCount = END_DELAY;
			}
			else if(NavDownIsPressed() || NavLeftIsPressed() || NavRightIsPressed())
			{
				RGB_RED_ENA();
				//eprintf("That is Not the Correct Direction :( \n");
				//eprintf("Your Reaction Time in miliseconds is : %d milliseconds!\n", TimeCounter);
				GameState = INCORRECT_STATE;
				DelayCount = END_DELAY;
			}
			else
			{
				TimeCounter++;
			}
		}

		if(RandomDirection == 1)
		{
			if(NavDownIsPressed())
			{
				RGB_GRN_ENA();
				//eprintf("That is Correct!!\n");
				//eprintf("Your Reaction Time in miliseconds is : %d milliseconds!\n", TimeCounter);
				LcdPrintf(2,0,"                     ");
				LcdPrintf(2,0,"That is Correct!!");
				LcdPrintf(3,0,"Reaction Time: %d ms", TimeCounter);
				GameState = CORRECT_STATE;
				DelayCount = END_DELAY;
			}
			else if(NavUpIsPressed() || NavLeftIsPressed() || NavRightIsPressed())
			{
				RGB_RED_ENA();
				//eprintf("That is Not the Correct Direction :(\n");
				//eprintf("Your Reaction Time in miliseconds is : %d milliseconds!\n", TimeCounter);
				LcdPrintf(2,0,"                     ");
				LcdPrintf(2,0,"That is  Not Correct!!");
				LcdPrintf(3,0,"Reaction Time: %d ms", TimeCounter);
				GameState = INCORRECT_STATE;
				DelayCount = END_DELAY;
			}
			else
			{
				TimeCounter++;
			}
		}

		if(RandomDirection == 2)
		{
			if(NavLeftIsPressed())
			{
				RGB_GRN_ENA();
				//eprintf("That is Correct!!\n");
				//eprintf("Your Reaction Time in miliseconds is : %d milliseconds!\n", TimeCounter);
				LcdPrintf(2,0,"                     ");
				LcdPrintf(2,0,"That is Correct!!");
				LcdPrintf(3,0,"Reaction Time: %d ms", TimeCounter);
				GameState = CORRECT_STATE;
				DelayCount = END_DELAY;
			}
			else if(NavDownIsPressed() || NavUpIsPressed() || NavRightIsPressed())
			{
				RGB_RED_ENA();
				//eprintf("That is Not the Correct Direction :(\n");
				//eprintf("Your Reaction Time in miliseconds is : %d milliseconds!\n", TimeCounter);
				LcdPrintf(2,0,"                     ");
				LcdPrintf(2,0,"That is Correct!!");
				LcdPrintf(3,0,"Reaction Time: %d ms", TimeCounter);
				GameState = INCORRECT_STATE;
				DelayCount = END_DELAY;
			}
			else
			{
				TimeCounter++;
			}
		}

		if(RandomDirection == 3)
		{
			if(NavRightIsPressed())
			{
				RGB_GRN_ENA();
				//eprintf("That is Correct!!\n");
				//eprintf("Your Reaction Time in miliseconds is : %d milliseconds!\n", TimeCounter);
				LcdPrintf(2,0,"                     ");
				LcdPrintf(2,0,"That is Correct!!");
				LcdPrintf(3,0,"Reaction Time: %d ms", TimeCounter);
				GameState = CORRECT_STATE;
				DelayCount = END_DELAY;
			}
			else if(NavDownIsPressed() || NavLeftIsPressed() || NavUpIsPressed())
			{
				RGB_RED_ENA();
				//eprintf("That is Not the Correct Direction :(\n");
				//eprintf("Your Reaction Time in miliseconds is : %d milliseconds!\n", TimeCounter);
				LcdPrintf(2,0,"                     ");
				LcdPrintf(2,0,"That is Correct!!");
				LcdPrintf(3,0,"Reaction Time: %d ms", TimeCounter);
				GameState = INCORRECT_STATE;
				DelayCount = END_DELAY;
			}
			else
			{
				TimeCounter++;
			}
		}
		break;

	case CORRECT_STATE:
		if(--DelayCount == 0)
			{
				RGB_GRN_DIS();
				GameState = RESTART_STATE;
			}
		break;

	case INCORRECT_STATE:
		if(--DelayCount == 0)
			{
				RGB_RED_DIS();
				GameState = RESTART_STATE;
			}
		break;

	case RESTART_STATE:
		RGB_BLU_ENA();
		//DelayCount = START_DELAY;
		//eprintf("\n\nPRESS SELECT TO PLAY ROCKET REACTION'S SPEED GAME!\n");
		LcdPrintf(3,0,"                     ");
		LcdPrintf(2,0,"PRESS SELECT TO PLAY ");
		GameState = READY_STATE;
		break;
		}
}

