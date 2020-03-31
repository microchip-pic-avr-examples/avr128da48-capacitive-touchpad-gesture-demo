[![MCHP](https://cldup.com/U0qhLwBijF.png)](https://www.microchip.com)
## Objective:
This example consists a basic ‘Snake Game Demo’ implemented to demonstrate the 2D touch surface gestures with the help of Microchip® AVR128DA48 Curiosity Nano and QT2 Xplained Pro extension boards. This application example showcases the advanced touch capabilities of ‘PTC’ peripheral, and low power capabilities of the AVR-DA MCUs.

[Demo Video](https://www.youtube.com/watch?v=1dhbFVoyYqc)

## Related Application Note:
[Touchpad With Built-In Surface Gesture Recognition using Peripheral Touch Controller](https://microchip.com/DS00003433)

## Peripherals Involved:
1. Peripheral Touch Controller (PTC)
2. Event System
3. SLPCTRL
4. TWI – I2C mode
5. Real Time Counter (RTC)
6. CPUINIT


## Hardware Setup for the Snake game demo using Curiosity nano and QT2 Xplained Pro: 
![Hardware Setup](Images/Setup.jpg)


## Running the Demo in ATMEL Studio:
1.	Download the zip file or clone the example to get the source code.
2.	Open the solution with Atmel Studio.
3.	Build the application and program the device.
4.	Press Start without debugging or use CTRL+ALT+F5 hotkeys to run the application.
5.	Snake game will start on the QT2 Led matrix.
6.	Use touch surface on QT2 to steer the snake towards food.
7.	After 20 seconds of inactivity MCU will go to sleep.
8.	Tap the touch surface anywhere to wake-up the MCU.
