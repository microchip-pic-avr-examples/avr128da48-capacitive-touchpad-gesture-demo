/*
 * LEDMatrix.c
 *
 * Created: 12/27/2019 4:15:44 PM
 *  Author: I20949
 */ 

#include <atmel_start.h>
#include "LEDMatrix.h"
#include "struct.h"
#include "definition.h"
#include <util/delay.h>

/*----------------------------------------------------------------------------
 *     Global Variables
 *----------------------------------------------------------------------------*/

extern volatile struct SysFlags Flags;
extern struct SnakeData Snake;
extern struct Location Food;
extern uint8_t I2CArray[9][2];

uint8_t LEDMatrix[7][7] = {0};

/*============================================================================
void ClearDisplay( void )
------------------------------------------------------------------------------
Purpose: This function turns OFF all the LEDs on 7x7 LED matrix of QT2
Input  : none
Output : none
Notes  :
============================================================================*/
void ClearDisplay(void)
{
	uint8_t LEDClrArray[9][2]={1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 0x0C,0};
	UpdateDisplay(I2C_ADD, LEDClrArray, 18);
}

/*============================================================================
void FillDisplay( void )
------------------------------------------------------------------------------
Purpose: This function turns ON all the LEDs on 7x7 LED matrix of QT2
Input  : none
Output : none
Notes  :
============================================================================*/
void FillDisplay(void)
{
	uint8_t LEDClrArray[9][2] = {1, 0xFF, 2, 0xFF, 3, 0xFF, 4, 0xFF, 5, 0xFF, 6, 0xFF, 7, 0xFF, 8, 0xFF, 0x0C, 0};
	UpdateDisplay(I2C_ADD, LEDClrArray, 18);
}

/*============================================================================
void BlinkDisplay( void )
------------------------------------------------------------------------------
Purpose: This function turn on and off the LED matrix for 
		 variable (passed as argument) number of times
Input  : Number of blink times
Output : none
Notes  :
============================================================================*/
void BlinkDisplay(uint8_t num)
{
	while (num--)
	{
		ClearDisplay();
		_delay_ms(250);
		FillDisplay();
		_delay_ms(250);
	}
}


/*============================================================================
void UpdateDisplay( void )
------------------------------------------------------------------------------
Purpose: This function update the LED matrix by sending commands via I2C
Input  : Add - I2C register address
		 *Data - pointer to the data array to be transfered
		 len - No of bytes to be sent
Output : none
Notes  :
============================================================================*/
void UpdateDisplay(uint8_t Add, void *Data, uint8_t len)
{
	I2C_0_writeNBytes(Add, Data, len);
	_delay_ms(10);
}

/*============================================================================
void LoadLedMatrix( void )
------------------------------------------------------------------------------
Purpose: This function loads the I2C array to be sent to the LED matrix.
		 It takes input from the snake array.
Input  : none
Output : none
Notes  :
============================================================================*/
void LoadLedMatrix(void)
{
	static PosBits Col;
	//Clean Array
	for(uint8_t i=0; i<sizeof(LEDMatrix); i++)
	{
		*((uint8_t *)(&LEDMatrix[0][0] + i)) = 0;
	}
	
	//Load New Array
	for(uint8_t i=0; i<Snake.Len; i++)
	{
		LEDMatrix[Snake.Pos[i][0]][Snake.Pos[i][1]] = 1;
	}
	
	//Load Food
	LEDMatrix[Food.PosX][Food.PosY] = 1;
	
	//Put Data on I2C Array
	for(uint8_t i=0; i<7; i++)
	{
		Col.PBits.B0 = LEDMatrix[0][i];
		Col.PBits.B1 = LEDMatrix[1][i];
		Col.PBits.B2 = LEDMatrix[2][i];
		Col.PBits.B3 = LEDMatrix[3][i];
		Col.PBits.B4 = LEDMatrix[4][i];
		Col.PBits.B5 = LEDMatrix[5][i];
		Col.PBits.B6 = LEDMatrix[6][i];
		I2CArray[i][1] = Col.PByte;
	}
}
