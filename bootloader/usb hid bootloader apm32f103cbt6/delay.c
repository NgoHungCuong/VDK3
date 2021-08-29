#include "delay.h"

void delay_init(void)
{
	TIM_TimeBaseInitTypeDef timerInit;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	timerInit.TIM_CounterMode = TIM_CounterMode_Up;
	timerInit.TIM_Period = 0xFFFF;
	timerInit.TIM_Prescaler = 72 - 1;
	
	TIM_TimeBaseInit(TIM2, &timerInit);
	
	TIM_Cmd(TIM2, ENABLE);
}

void delay_us(uint16_t u16Delay)
{
	TIM_SetCounter(TIM2, 0);
	while (TIM_GetCounter(TIM2) < u16Delay) {
	}
}

void delay_ms(uint32_t u32Delay)
{
	while (u32Delay) {
		delay_us(1000);
		--u32Delay;
	}
}
