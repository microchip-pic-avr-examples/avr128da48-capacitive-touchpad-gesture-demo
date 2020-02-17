/*
 * LEDMatrix.h
 *
 * Created: 12/27/2019 4:16:33 PM
 *  Author: I20949
 */ 


#ifndef LEDMATRIX_H_
#define LEDMATRIX_H_

void ClearDisplay(void);
void FillDisplay(void);
void BlinkDisplay(uint8_t i);
void UpdateDisplay(uint8_t Add, void *Data, uint8_t len);
void LoadLedMatrix(void);


#endif /* LEDMATRIX_H_ */