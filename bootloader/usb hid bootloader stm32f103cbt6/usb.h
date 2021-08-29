#ifndef USB_H_
#define USB_H_

#include "stm32f10x.h"                  // Device header
#include "usb_regs.h"
#include "usb_mem.h"

#define EP0_SZ 64

void USB_Init(void);
void USB_Task(void);

#endif
