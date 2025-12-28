#ifndef  TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include "main.h"
#include "pins.h"
#include "ILI9481.h"

#define TPort GPIOA
#define _xm 	3 // analog
#define _ym 	2 // digital
#define _xp 	1 // digital
#define _yp 	0 // analog

#define _XM 	ADC_CHANNEL_3 // analog
#define _YM 	ADC_CHANNEL_2 // digital
#define _XP 	ADC_CHANNEL_1 // digital
#define _YP 	ADC_CHANNEL_0 // analog

#define TS_MINX 3640
#define TS_MINY 3800
#define TS_MAXX 490
#define TS_MAXY 1060

//static void MX_ADC1_Init(void);
uint32_t analogRead(uint32_t pin);
void pinMode(uint16_t pin, GPIO_TypeDef * port, uint16_t state);
void digitalWrite(uint32_t pin, GPIO_TypeDef * port, uint32_t state);
int getRawPoint(uint16_t * x1, uint16_t * y1, uint16_t * x2, uint16_t * y2, uint16_t * z1, uint16_t * z2);
void mapPoint(uint16_t * x, uint16_t * y);
int touchscreen(uint16_t * x, uint16_t * y);


ADC_HandleTypeDef hadc1;

#define amount 20
#define _ConstantCheck 600
int screenPressed() {
	uint16_t x, y;
	int count = 0;
	for (int i = 0; i < 8; i++) {
		count += touchscreen(&x, &y);
	}
	if (count > 0) {
		return true;
	}
	return false;
}
int touchscreen(uint16_t * x, uint16_t * y) {
  uint16_t x1, y1, x2, y2, z1, z2;
  uint32_t X1 = 0, Y1 = 0, X2 = 0, Y2 = 0;
	double Z1 = 0, Z2 = 0;
	for (uint16_t i = 0; i < amount; i++) {
		getRawPoint(&x1, &y1, &x2, &y2, &z1, &z2);
		X1 += x1;
		Y1 += y1;
		
		X2 += x2;
		Y2 += y2;
		
		Z1 += z1;
		Z2 += z2;
		
		#ifdef _ConstantCheck
		float rtouch;
		rtouch = z2;
		rtouch /= z1;
		rtouch -= 1;
		rtouch *= (6000-x1);
		rtouch *= 300;
		rtouch /= 4096;
		
		if (rtouch >= 750) {
			return false;
		}
		#endif
		
	}
	X1 = X1 / amount;
	Y1 = Y1 / amount;
	
	X2 = X2 / amount;
	Y2 = Y2 / amount;
	
	Z1 = Z1 / amount;
	Z2 = Z2 / amount;
	
	(*x) = (X1 + X2) / 2;
	(*y) = (Y1 + Y2) / 2;
	
	float xx = (*x);
	
	float rtouch;
	rtouch = Z2;
	rtouch /= Z1;
	rtouch -= 1;
	rtouch *= (6000-xx);
	rtouch *= 300;
	rtouch /= 4096;
	 
	mapPoint(x, y);
	
	if (rtouch < 750) {
		return true;
	}
	return false;
}

void getPoint(uint16_t * x, uint16_t * y) {
	
}

int getRawPoint(uint16_t * x1, uint16_t * y1, uint16_t * x2, uint16_t * y2, uint16_t * z1, uint16_t * z2) {
	
	pinMode(_yp, GPIOA, ANALOG);
  pinMode(_ym, GPIOA, ANALOG);
  pinMode(_xp, GPIOA, OUTPUT);
  pinMode(_xm, GPIOA, OUTPUT);
  digitalWrite(_xp, GPIOA, LOW);
  digitalWrite(_xm, GPIOA, HIGH);
	
	(*x1) = (analogRead(ADC_CHANNEL_0)); //_yp
	
	pinMode(_yp, GPIOA, ANALOG);
  pinMode(_ym, GPIOA, ANALOG);
  pinMode(_xp, GPIOA, OUTPUT);
  pinMode(_xm, GPIOA, OUTPUT);
  digitalWrite(_xp, GPIOA, HIGH);
  digitalWrite(_xm, GPIOA, LOW);
	
	(*x2) = (analogRead(ADC_CHANNEL_0)); //_yp
	(*x2) = map((*x2), TS_MINX, TS_MAXX, TS_MAXX, TS_MINX);
	
  pinMode(_xp, GPIOA, ANALOG);
  pinMode(_xm, GPIOA, ANALOG);
  pinMode(_yp, GPIOA, OUTPUT);
  pinMode(_ym, GPIOA, OUTPUT);
  digitalWrite(_yp, GPIOA, LOW);
  digitalWrite(_ym, GPIOA, HIGH);
	
	(*y1) = (analogRead(ADC_CHANNEL_1)); //_xm
	
  pinMode(_xp, GPIOA, ANALOG);
  pinMode(_xm, GPIOA, ANALOG);
  pinMode(_yp, GPIOA, OUTPUT);
  pinMode(_ym, GPIOA, OUTPUT);
  digitalWrite(_yp, GPIOA, HIGH);
  digitalWrite(_ym, GPIOA, LOW);
	
	(*y2) = (analogRead(ADC_CHANNEL_1)); //_xm
	(*y2) = map((*y2), TS_MINY, TS_MAXY, TS_MAXY, TS_MINY);
	



	//pressure
  pinMode(_xp, GPIOA, ANALOG);
  pinMode(_xm, GPIOA, OUTPUT);
  pinMode(_yp, GPIOA, OUTPUT);
  pinMode(_ym, GPIOA, ANALOG);
  digitalWrite(_yp, GPIOA, HIGH);
  digitalWrite(_xm, GPIOA, LOW);
	
	(*z1) = (analogRead(_XP)); //_xp
	(*z2) = (analogRead(_YM)); //_xm
	
	


   pinMode(_xp, GPIOA, OUTPUT);
   pinMode(_xm, GPIOA, OUTPUT);
   pinMode(_yp, GPIOA, OUTPUT);
   pinMode(_ym, GPIOA, OUTPUT);
	
	
	return 0;
}
int mouseOver(uint16_t x, uint16_t y, int X, int Y, int W, int H) {
	if (x >= X && y >= Y && x < X + W && y < Y + H) {
		return true;
	}
	return false;
}
void mapPoint(uint16_t * x, uint16_t * y) {
	uint16_t TX = 0, TY = 0;
	if (_Rotation == 0) {
			TX = map((*x), TS_MINX, TS_MAXX, 0, 320);
			TY = map((*y), TS_MINY, TS_MAXY, 0, 480);
	}
	if (_Rotation == 1) {
			TY = map((*x), TS_MAXX, TS_MINX, 0, 320);
			TX = map((*y), TS_MINY, TS_MAXY, 0, 480);
	}
	if (_Rotation == 2) {
			TX = map((*x), TS_MAXX, TS_MINX, 0, 320);
			TY = map((*y), TS_MAXY, TS_MINY, 0, 480);
	}
	if (_Rotation == 3) {
			TY = map((*x), TS_MINX, TS_MAXX, 0, 320);
			TX = map((*y), TS_MAXY, TS_MINY, 0, 480);
	}
	(*x) = TX;
	(*y) = TY;
}
#endif




