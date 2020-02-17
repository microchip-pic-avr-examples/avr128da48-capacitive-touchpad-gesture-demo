#include <atmel_start.h>
#include "definition.h"
#include "struct.h"
#include "Snake.h"
#include "LEDMatrix.h"

#if defined(__GNUC__)
#include <avr/sleep.h>
#elif defined(__ICCAVR__)
#include <intrinsics.h>
#define sleep_cpu() __sleep()
#endif

/*----------------------------------------------------------------------------
 *   prototypes
 *----------------------------------------------------------------------------*/

void TimeSchedulerHandler(void);
static void CheckTouchInput(void);

/*----------------------------------------------------------------------------
 *     Global Variables
 *----------------------------------------------------------------------------*/
extern qtm_gestures_2d_control_t  qtm_gestures_2d_control1;

volatile uint8_t TimCount;
volatile struct SysFlags Flags;
struct SnakeData Snake = {0};
uint8_t I2CArray[9][2] = {1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 0x0C,0};


int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	
	BlinkDisplay(5);
	SnakeInit();
	
	/* Replace with your application code */
	while (1) 
	{
		//Process touch data
		touch_process();
		//read touch commands & clear gestures after read
		CheckTouchInput();
		qtm_gestures_2d_clearGesture();
		
		//1 sec time scheduler
		if(Flags.MCUSleep == 0)
		{
			if((Flags.Scheduler1s)&&(!Flags.GameOver))		//Move snake if game is in progress
			{
				AdvanceSnake();
				LoadLedMatrix();
				UpdateDisplay(I2C_ADD, I2CArray, 18);
				Flags.Scheduler1s = 0;
			}
			if(Flags.GameOver)								//Re-init game if game over
			{
				BlinkDisplay(2);
				SnakeInit();
				Flags.GameOver = 0;
			}
		}
		
		//Check for user touch inactivity (Flag read from touch_process_lowpower)
		if(Flags.Sleep)
		{
			Flags.Sleep = 0;
			ClearDisplay();
			#if (DEF_TOUCH_LOWPOWER_ENABLE == 1u)
				sleep_cpu();
			#endif
			Flags.MCUSleep = 1;
		}
		if(Flags.WakeUp)
		{
			BlinkDisplay(2);
			SnakeInit();
			Flags.WakeUp = 0;
			Flags.MCUSleep = 0;
		}	
	}
}


/*============================================================================
void TimeSchedulerHandler( void )
------------------------------------------------------------------------------
Purpose: This function is called in RTC ISR. It is used for generating
		 a 500ms time scheduler for snake movement.
Input  : none
Output : none
Notes  :
============================================================================*/
void TimeSchedulerHandler(void)
{	
	if(TimCount++ >= 25)			//RTC Interrupt is set to 20ms, 20*25 = 500ms
	{
		Flags.Scheduler1s = 1;
		TimCount = 0;
	}
}

/*============================================================================
static void CheckTouchInput( void )
------------------------------------------------------------------------------
Purpose: This function is called to read the touch gestures.
		 It provides the direction change for snake.
Input  : none
Output : none
Notes  :
============================================================================*/
static void CheckTouchInput(void)
{
	//Decode touch Swipes
	if ((qtm_gestures_2d_control1.qtm_gestures_2d_data->gestures_which_gesture & 0xf0) == RIGHT_SWIPE)
	{
		Flags.SwRight = 1;
	}
	else if ((qtm_gestures_2d_control1.qtm_gestures_2d_data->gestures_which_gesture & 0xf0) == LEFT_SWIPE)
	{
		Flags.SwLeft = 1;
	}
	else if ((qtm_gestures_2d_control1.qtm_gestures_2d_data->gestures_which_gesture & 0xf0) == UP_SWIPE)
	{
		Flags.SwUp = 1;
	}
	else if ((qtm_gestures_2d_control1.qtm_gestures_2d_data->gestures_which_gesture & 0xf0) == DOWN_SWIPE)
	{
		Flags.SwDown = 1;
	}
	else if ((qtm_gestures_2d_control1.qtm_gestures_2d_data->gestures_which_gesture & 0xF0) == TAP)
	{
		//Flags.Tap = 1;
	}
	
	//Give Snake commands depending on touch swipes
	if(Flags.SwUp)
	{
		if(Snake.Dir != DOWN)
		{
			Snake.Dir = UP;
		}
		Flags.SwUp = 0;
	}
	
	if(Flags.SwDown)
	{
		if(Snake.Dir != UP)
		{
			Snake.Dir = DOWN;
		}
		Flags.SwDown = 0;
	}
	
	if(Flags.SwLeft)
	{
		if(Snake.Dir != RIGHT)
		{
			Snake.Dir = LEFT;
		}
		Flags.SwLeft = 0;
	}
	
	if(Flags.SwRight)
	{
		if(Snake.Dir != LEFT)
		{
			Snake.Dir = RIGHT;
		}
		Flags.SwRight = 0;
	}
}