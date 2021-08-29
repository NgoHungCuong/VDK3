#ifndef DELAY_H_
#define DELAY_H_

#include "stm32f10x.h"                  // Device header

void delay_init(void);
void delay_us(uint16_t u16Delay);
void delay_ms(uint32_t u32Delay);

#endif
