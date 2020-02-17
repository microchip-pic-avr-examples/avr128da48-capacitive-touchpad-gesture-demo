/*
 * definition.h
 *
 * Created: 10/9/2019 3:29:38 PM
 *  Author: I20949
 */ 

#ifndef DEFINITION_INCLUDED
#define DEFINITION_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define I2C_ADD 0x60u	//Hardware address for ISSI Led driver
#define LED_ADD 0x01u	//Register Address for LED control

#define SIZE_X	7
#define SIZE_Y	7

#define DISABLE_RTC_INT()	(RTC.INTCTRL = 0)
#define ENABLE_RTC_INT()	(RTC.INTCTRL = 2)

typedef enum
{
	UP,
	DOWN,
	LEFT,
	RIGHT,
}SnakeDir;

#define SWUP	(PORTE.IN & 0x1)
#define SWLT	(PORTE.IN & 0x2)
#define SWRT	(PORTE.IN & 0x4)
#define SWDN	(PORTE.IN & 0x8)

#ifdef __cplusplus
}
#endif

#endif /* DEFINITION_INCLUDED */