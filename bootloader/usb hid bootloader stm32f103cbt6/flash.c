#include "flash.h"

void FlashUnlock(void) {
	FLASH->KEYR = 0x45670123UL;
	FLASH->KEYR = 0xCDEF89ABUL;
}

void FlashLock(void) {
	FLASH->CR |= FLASH_CR_LOCK;
}

uint32_t FormatFlashPage(uint32_t page) {
	while(FLASH->SR & FLASH_SR_BSY);
	
	FLASH->SR = 0x34;
	
	FLASH->CR |= FLASH_CR_PER;

	FLASH->AR = page;

	FLASH->CR |= FLASH_CR_STRT;

	while(FLASH->SR & FLASH_SR_BSY);

	FLASH->CR &= ~FLASH_CR_PER;
	
	if (FLASH->SR & 0x14) {
		return 0;
	} else {
		return 1;
	}
}

uint32_t WriteFlash(uint32_t page, uint8_t *data, uint16_t size) {
	uint16_t i;
	uint16_t tmp;
	
	while(FLASH->SR & FLASH_SR_BSY);
	
	FLASH->SR = 0x34;

	FLASH->CR |= FLASH_CR_PG;

	for(i = 0; i < size; i += 2) {
		tmp = data[i] | (data[i + 1] << 8);
		*(volatile uint16_t *)(page + i) = tmp;

		while(FLASH->SR & FLASH_SR_BSY);
		if (FLASH->SR & 0x14) {
			return 0;
		}
	}

	FLASH->CR &= ~FLASH_CR_PG;
	
	return 1;
}
