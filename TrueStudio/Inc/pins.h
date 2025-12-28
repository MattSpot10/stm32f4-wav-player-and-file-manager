#ifndef  PINS_H
#define PINS_H

#include <stdio.h>
#include <stdlib.h>
#include "main.h"

#define OUTPUT 0
#define INPUT 1
#define ANALOG 2

#define LOW 0
#define HIGH 1

#define false			0
#define true			1


#define STOPED 		0
#define PAUSED 		1
#define BUSY 			2
#define IDLE			3

int SDState = 0;

uint32_t analogRead(uint32_t pin);
void pinMode(uint16_t pin, GPIO_TypeDef * port, uint16_t state);
void digitalWrite(uint32_t pin, GPIO_TypeDef * port, uint32_t state);
long map(long x, long in_min, long in_max, long out_min, long out_max);
static void ADC1_RE_Init(void);
uint16_t randomInt(uint16_t min, uint16_t max);


ADC_HandleTypeDef hadc1;
RNG_HandleTypeDef hrng;


uint32_t C_Channel = ADC_CHANNEL_0;

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
uint32_t analogRead(uint32_t pin) {
	pinMode(5, GPIOA, ANALOG);
	pinMode(6, GPIOA, ANALOG);
	pinMode(7, GPIOA, ANALOG);
	pinMode(0, GPIOB, ANALOG);
	uint32_t adcResult = 0;
	
	C_Channel = pin;
	ADC1_RE_Init();
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 5);
	adcResult = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	
	return adcResult;
}

void pinMode(uint16_t pin, GPIO_TypeDef * port, uint16_t state) {
	if (state == OUTPUT) {//01 - output
		port->MODER |= 0x01 << (pin * 2);//0b01
		port->MODER &= ~(0x02 << (pin * 2));//0b10
	}
	if (state == INPUT) {//00 - reset
		port->MODER &= ~(0x03 << (pin * 2));//0b11
	}
	if (state == ANALOG) {//11 - analog
		port->MODER |= 0x02 << (pin * 2);//0b11
	}
}
void digitalWrite(uint32_t pin, GPIO_TypeDef * port, uint32_t state) {
	port->BSRR = (((~state) << pin)<<16) | (state << pin);//((~d)<<16) | (d);
}

static void ADC1_RE_Init(void)
{

	ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfig.Channel = C_Channel;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

}
uint16_t randomInt(uint16_t min, uint16_t max) {
	uint16_t r = 0;
	
	r = HAL_RNG_GetRandomNumber(&hrng);
	return map(r, 0, 65536, min, max);
}

#endif
