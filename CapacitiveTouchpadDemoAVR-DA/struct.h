/*
 * struct.h
 *
 * Created: 10/9/2019 10:48:09 AM
 *  Author: I20949
 */ 

#ifndef STRUCT_INCLUDED
#define STRUCT_INCLUDED

#include "definition.h"

#ifdef __cplusplus
extern "C" {
#endif


struct SysFlags
{
	uint8_t Scheduler1s		: 1;
	uint8_t GameOver		: 1;
	uint8_t NewFoodCreated	: 1;
	uint8_t I2CBusy			: 1;
	uint8_t Sleep			: 1;
	uint8_t WakeUp			: 1;
	uint8_t MCUSleep		: 1;
};

struct Bits
{
	uint8_t B0 : 1;
	uint8_t B1 : 1;
	uint8_t B2 : 1;
	uint8_t B3 : 1;
	uint8_t B4 : 1;
	uint8_t B5 : 1;
	uint8_t B6 : 1;
	uint8_t B7 : 1;
};

typedef union
{
	uint8_t PByte;
	struct Bits PBits;
}PosBits;

struct SnakeData
{
	SnakeDir Dir;
	uint8_t Len;
	uint8_t Pos[SIZE_X * SIZE_Y][2];
};

struct Location
{
	int8_t PosX;
	int8_t PosY;
};


#ifdef __cplusplus
}
#endif

#endif /* STRUCT_INCLUDED */