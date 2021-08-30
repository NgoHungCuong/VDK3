#ifndef FLASH_H_
#define FLASH_H_

#include "stm32f10x.h"                  // Device header

#define DEV_INFO				"STM32F103CBT6"
#define BOOT_VER				"BOOTLOADER V210831"
#define APP_BASE_ADDRESS 		0x08001000
#define APP_SIZE 				126976 		//28KB
#define FLASH_ERASE_PAGE_SIZE 	1024
#define FLASH_WRITE_PAGE_SIZE 	2

void FlashUnlock(void);
void FlashLock(void);
uint32_t FormatFlashPage(uint32_t page);
uint32_t WriteFlash(uint32_t page, uint8_t *data, uint16_t size);

#endif
