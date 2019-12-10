//
// GENG 421 - Scott Kolnes
//
// RGB LED module
//

#include "geng421.h"
#include "rgb.h"


#define RED_ST_OFF		0
#define RED_ST_ON		1

#define GRN_ST_OFF		0
#define GRN_ST_ON		1

#define BLU_ST_OFF		0
#define BLU_ST_ON		1

#define YELLOW_ST_OFF	0
#define YELLOW_ST_ON	1

#define TURQ_ST_OFF		0
#define TURQ_ST_ON		1

#define PURPLE_ST_OFF	0
#define PURPLE_ST_ON	1

#define WHITE_ST_OFF	0
#define WHITE_ST_ON		1


#define RED_PWM_MIN		0
#define RED_PWM_MAX		10

#define GRN_PWM_MIN		0
#define GRN_PWM_MAX		10

#define BLU_PWM_MIN		0
#define BLU_PWM_MAX		10

#define YELLOW_PWM_MIN	0
#define YELLOW_PWM_MAX	10

#define TURQ_PWM_MIN	0
#define TURQ_PWM_MAX	10

#define PURPLE_PWM_MIN	0
#define PURPLE_PWM_MAX	10

#define WHITE_PWM_MIN	0
#define WHITE_PWM_MAX	10


static int	RedPwmSetting = 0;
static int 	GreenPwmSetting = 0;
static int 	BluePwmSetting = 0;
static int 	YellowPwmSetting = 0;
static int 	TurqPwmSetting = 0;
static int 	PurplePwmSetting = 0;
static int 	WhitePwmSetting = 0;


void RgbClock(void)
{
	static int	RedState = RED_ST_OFF;
	static int	RedPwmCount = 0;

	static int	GreenState = GRN_ST_OFF;
	static int	GreenPwmCount = 0;

	static int	BlueState = BLU_ST_OFF;
	static int	BluePwmCount = 0;

	static int	YellowState = YELLOW_ST_OFF;
	static int	YellowPwmCount = 0;

	static int	TurqState = TURQ_ST_OFF;
	static int	TurqPwmCount = 0;

	static int	PurpleState = PURPLE_ST_OFF;
	static int	PurplePwmCount = 0;

	static int	WhiteState = WHITE_ST_OFF;
	static int	WhitePwmCount = 0;

	RedPwmCount++;
	switch(RedState)
	{
	case RED_ST_OFF:
		if (RedPwmCount == RED_PWM_MAX)
		{
			RedPwmCount = 0;
			if (RedPwmSetting != 0)
			{
				RGB_RED_ENA();
				RedState = RED_ST_ON;
				break;
			}
		}
		break;

	case RED_ST_ON:
		if (RedPwmCount >= RedPwmSetting)
		{
			if (RedPwmSetting == RED_PWM_MAX)
			{
				RedPwmCount = 0;
			}
			else
			{
				RGB_RED_DIS();
				RedState = RED_ST_OFF;
				break;
			}
		}
		break;
	}

	GreenPwmCount++;
	switch(GreenState)
	{
	case GRN_ST_OFF:
		if (GreenPwmCount == GRN_PWM_MAX)
		{
			GreenPwmCount = 0;
			if (GreenPwmSetting != 0)
			{
				RGB_GRN_ENA();
				GreenState = GRN_ST_ON;
				break;
			}
		}
		break;

	case GRN_ST_ON:
		if (GreenPwmCount >= GreenPwmSetting)
		{
			if (GreenPwmSetting == GRN_PWM_MAX)
			{
				GreenPwmCount = 0;
			}
			else
			{
				RGB_GRN_DIS();
				GreenState = GRN_ST_OFF;
				break;
			}
		}
		break;
	}

	BluePwmCount++;
	switch(BlueState)
	{
	case BLU_ST_OFF:
		if (BluePwmCount == BLU_PWM_MAX)
		{
			BluePwmCount = 0;
			if (BluePwmSetting != 0)
			{
				RGB_BLU_ENA();
				BlueState = BLU_ST_ON;
				break;
			}
		}
		break;

	case BLU_ST_ON:
		if (BluePwmCount >= BluePwmSetting)
		{
			if (BluePwmSetting == BLU_PWM_MAX)
			{
				BluePwmCount = 0;
			}
			else
			{
				RGB_BLU_DIS();
				BlueState = BLU_ST_OFF;
				break;
			}
		}
		break;
	}

	YellowPwmCount++;
	switch(YellowState)
	{
	case YELLOW_ST_OFF:
		if (YellowPwmCount == YELLOW_PWM_MAX)
		{
			YellowPwmCount = 0;
			if (YellowPwmSetting != 0)
			{
				RGB_RED_ENA();
				RGB_GRN_ENA();
				YellowState = YELLOW_ST_ON;
				break;
			}
		}
		break;

	case YELLOW_ST_ON:
		if (YellowPwmCount >= YellowPwmSetting)
		{
			if (YellowPwmSetting == YELLOW_PWM_MAX)
			{
				BluePwmCount = 0;
			}
			else
			{
				RGB_RED_DIS();
				RGB_GRN_DIS();
				YellowState = YELLOW_ST_OFF;
				break;
			}
		}
		break;
	}

	TurqPwmCount++;
	switch(TurqState)
	{
	case TURQ_ST_OFF:
		if (TurqPwmCount == TURQ_PWM_MAX)
		{
			TurqPwmCount = 0;
			if (TurqPwmSetting != 0)
			{
				RGB_BLU_ENA();
				RGB_GRN_ENA();
				TurqState = TURQ_ST_ON;
				break;
			}
		}
		break;

	case TURQ_ST_ON:
		if (TurqPwmCount >= TurqPwmSetting)
		{
			if (TurqPwmSetting == TURQ_PWM_MAX)
			{
				TurqPwmCount = 0;
			}
			else
			{
				RGB_BLU_DIS();
				RGB_GRN_DIS();
				TurqState = TURQ_ST_OFF;
				break;
			}
		}
		break;
	}

	PurplePwmCount++;
	switch(PurpleState)
	{
	case PURPLE_ST_OFF:
		if (PurplePwmCount == PURPLE_PWM_MAX)
		{
			PurplePwmCount = 0;
			if (PurplePwmSetting != 0)
			{
				RGB_BLU_ENA();
				RGB_RED_ENA();
				PurpleState = PURPLE_ST_ON;
				break;
			}
		}
		break;

	case PURPLE_ST_ON:
		if (PurplePwmCount >= PurplePwmSetting)
		{
			if (PurplePwmSetting == PURPLE_PWM_MAX)
			{
				PurplePwmCount = 0;
			}
			else
			{
				RGB_BLU_DIS();
				RGB_RED_DIS();
				PurpleState = PURPLE_ST_OFF;
				break;
			}
		}
		break;
	}

	WhitePwmCount++;
	switch(WhiteState)
	{
	case WHITE_ST_OFF:
		if (WhitePwmCount == WHITE_PWM_MAX)
		{
			WhitePwmCount = 0;
			if (WhitePwmSetting != 0)
			{
				RGB_BLU_ENA();
				RGB_GRN_ENA();
				RGB_RED_ENA();
				WhiteState = WHITE_ST_ON;
				break;
			}
		}
		break;

	case WHITE_ST_ON:
		if (WhitePwmCount >= WhitePwmSetting)
		{
			if (WhitePwmSetting == WHITE_PWM_MAX)
			{
				WhitePwmCount = 0;
			}
			else
			{
				RGB_BLU_DIS();
				RGB_RED_DIS();
				RGB_GRN_DIS();
				WhiteState = WHITE_ST_OFF;
				break;
			}
		}
		break;
	}
}


void RgbRedPwmSet(int PwmSetting)
{
	if (PwmSetting < RED_PWM_MIN)
	{
		PwmSetting = RED_PWM_MIN;
	}
	if (PwmSetting > RED_PWM_MAX)
	{
		PwmSetting = RED_PWM_MAX;
	}
	RedPwmSetting = PwmSetting;
}

void RgbGreenPwmSet(int PwmSetting)
{
	if (PwmSetting < GRN_PWM_MIN)
	{
		PwmSetting = GRN_PWM_MIN;
	}
	if (PwmSetting > GRN_PWM_MAX)
	{
		PwmSetting = GRN_PWM_MAX;
	}
	GreenPwmSetting = PwmSetting;
}

void RgbBluePwmSet(int PwmSetting)
{
	if (PwmSetting < BLU_PWM_MIN)
	{
		PwmSetting = BLU_PWM_MIN;
	}
	if (PwmSetting > BLU_PWM_MAX)
	{
		PwmSetting = BLU_PWM_MAX;
	}
	BluePwmSetting = PwmSetting;
}

void RgbYellowPwmSet(int PwmSetting)
{
	if (PwmSetting < YELLOW_PWM_MIN)
	{
		PwmSetting = YELLOW_PWM_MIN;
	}
	if (PwmSetting > YELLOW_PWM_MAX)
	{
		PwmSetting = YELLOW_PWM_MAX;
	}
	YellowPwmSetting = PwmSetting;
}

void RgbTurqPwmSet(int PwmSetting)
{
	if (PwmSetting < TURQ_PWM_MIN)
	{
		PwmSetting = TURQ_PWM_MIN;
	}
	if (PwmSetting > TURQ_PWM_MAX)
	{
		PwmSetting = TURQ_PWM_MAX;
	}
	TurqPwmSetting = PwmSetting;
}

void RgbPurplePwmSet(int PwmSetting)
{
	if (PwmSetting < PURPLE_PWM_MIN)
	{
		PwmSetting = PURPLE_PWM_MIN;
	}
	if (PwmSetting > PURPLE_PWM_MAX)
	{
		PwmSetting = PURPLE_PWM_MAX;
	}
	PurplePwmSetting = PwmSetting;
}

void RgbWhitePwmSet(int PwmSetting)
{
	if (PwmSetting < WHITE_PWM_MIN)
	{
		PwmSetting = WHITE_PWM_MIN;
	}
	if (PwmSetting > WHITE_PWM_MAX)
	{
		PwmSetting = WHITE_PWM_MAX;
	}
	WhitePwmSetting = PwmSetting;
}

int RgbRedPwmGet(void)
{
	return RedPwmSetting;
}

int RgbGreenPwmGet(void)
{
	return GreenPwmSetting;
}

int RgbBluePwmGet(void)
{
	return BluePwmSetting;
}

int RgbYellowPwmGet(void)
{
	return YellowPwmSetting;
}

int RgbTurqPwmGet(void)
{
	return TurqPwmSetting;
}

int RgbPurplePwmGet(void)
{
	return PurplePwmSetting;
}

int RgbWhitePwmGet(void)
{
	return WhitePwmSetting;
}
