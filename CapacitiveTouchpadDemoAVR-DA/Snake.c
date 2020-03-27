/*
 * Snake.c
 *
 * Created: 12/27/2019 4:20:36 PM
 *  Author: I20949
 */ 

#include <atmel_start.h>
#include "Snake.h"
#include "LEDMatrix.h"
#include "struct.h"
#include "definition.h"

/*----------------------------------------------------------------------------
 *     Global Variables
 *----------------------------------------------------------------------------*/

extern volatile struct SysFlags Flags;
extern struct SnakeData Snake;
extern uint8_t I2CArray[9][2];
extern volatile uint8_t TimCount;

struct Location Food = {0};
static struct SnakeData Empty = {0};

/*============================================================================
void SnakeInit( void )
------------------------------------------------------------------------------
Purpose: Initialize the snake parameters to the reset value.
		 Start the new game on screen.
Input  : none
Output : none
Notes  :
============================================================================*/
void SnakeInit(void)
{
	//Clean the Structure
	Snake = Empty;
	
	//Initialize with default Values
	Snake.Dir = LEFT;
	Snake.Len = 2;
	
	for(uint8_t i=0; i<Snake.Len; i++)
	{
		Snake.Pos[i][0] = SIZE_X / 2;
		Snake.Pos[i][1] = SIZE_Y / 2 + i;
	}
	
	//Generate Food
	GenerateNewFood();
	
	//Display Initial Snake
	LoadLedMatrix();
	UpdateDisplay(I2C_ADD, I2CArray, 18);
}


/*============================================================================
void AdvanceSnake( void )
------------------------------------------------------------------------------
Purpose: This function is the snake game algorithm. It moves the snake and
		 checks for food eating or collision.
Input  : none
Output : none
Notes  :
============================================================================*/
void AdvanceSnake(void)
{
	//Calculate Next coordinate depending on direction
	struct Location Next;
	switch (Snake.Dir)
	{
		case UP :
		Next.PosX = Snake.Pos[0][0] + 1;
		Next.PosY = Snake.Pos[0][1];
		break;
		case DOWN :
		Next.PosX = Snake.Pos[0][0] - 1;
		Next.PosY = Snake.Pos[0][1];
		break;
		case RIGHT :
		Next.PosX = Snake.Pos[0][0];
		Next.PosY = Snake.Pos[0][1] + 1;
		break;
		case LEFT :
		default :
		Next.PosX = Snake.Pos[0][0];
		Next.PosY = Snake.Pos[0][1] - 1;
		break;
	}
	
	//Check for boundary condition
	if(Next.PosX < 0)
	{
		Next.PosX = SIZE_X - 1;
	}
	else if(Next.PosX >= SIZE_X)
	{
		Next.PosX = 0;
	}
	
	if(Next.PosY < 0)
	{
		Next.PosY = SIZE_Y - 1;
	}
	else if(Next.PosY >= SIZE_Y)
	{
		Next.PosY = 0;
	}
	
	//Check for Collision
	for(uint8_t i=0; i<Snake.Len; i++)
	{
		if((Next.PosX == Snake.Pos[i][0])&&(Next.PosY == Snake.Pos[i][1]))
		{
			Flags.GameOver = 1;
			return;
		}
	}
	
	//check for game complete
	if(Snake.Len >= 47)
	{
		Flags.GameOver = 1;
		return;
	}
	
	//Advance Snake and Grow if food found
	for(uint8_t i=Snake.Len; i>0; i--)
	{
		Snake.Pos[i][0] = Snake.Pos[i-1][0];
		Snake.Pos[i][1] = Snake.Pos[i-1][1];
	}
	Snake.Pos[0][0] = Next.PosX;
	Snake.Pos[0][1] = Next.PosY;
	
	if ((Next.PosX == Food.PosX) && (Next.PosY == Food.PosY))
	{
		Snake.Len += 1;
		GenerateNewFood();
	}
	else
	{
		Snake.Pos[Snake.Len][0] = 0;
		Snake.Pos[Snake.Len][1] = 0;
	}
}

/*============================================================================
void GenerateNewFood( void )
------------------------------------------------------------------------------
Purpose: This function generates a random coordinates for new food.
Input  : none
Output : none
Notes  :
============================================================================*/
void GenerateNewFood(void)
{
	uint8_t X, Y;
	Flags.NewFoodCreated = 0;
	srandom(TimCount*random());
	while(! Flags.NewFoodCreated)
	{
		random();
		X = (random()+1) % (SIZE_X);
		random();
		Y = (random()+1) % (SIZE_Y);
		
		Food.PosX = X;
		Food.PosY = Y;
		
		Flags.NewFoodCreated = 1;
		
		//Check for collision with Snake
		for(uint8_t i=0; i<Snake.Len; i++)
		{
			if((Food.PosX == Snake.Pos[i][0])&&(Food.PosY == Snake.Pos[i][1]))
			{
				Flags.NewFoodCreated = 0;
			}
		}
	}
}
