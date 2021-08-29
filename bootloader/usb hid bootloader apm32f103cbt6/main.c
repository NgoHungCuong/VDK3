#include "stm32f10x.h"                  // Device header
#include "delay.h"
#include "usb.h"

#define ApplicationAddress 0x08001000

typedef  void (*pFunction)(void);
pFunction Jump_To_Application;
uint32_t JumpAddress;
uint32_t *p;

int main(void)
{
	GPIO_InitTypeDef gpioInit;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	gpioInit.GPIO_Mode = GPIO_Mode_IPU;
	gpioInit.GPIO_Pin = GPIO_Pin_13;
	gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOC, &gpioInit);
	
	if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13)) {
		p = (uint32_t *)ApplicationAddress;
		if ((*p & 0xE0000000) == 0x20000000) {
			JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);
			Jump_To_Application = (pFunction) JumpAddress;
			/* Initialize user application's Stack Pointer */
			__set_MSP(*(__IO uint32_t*) ApplicationAddress);
			Jump_To_Application();
		}
	}
	
	USB_Init();
	
	while (1) {
		USB_Task();
	}
}
