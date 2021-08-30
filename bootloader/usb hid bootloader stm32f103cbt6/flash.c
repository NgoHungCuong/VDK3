#include "flash.h"

void FlashUnlock(void) {
	FLASH->KEYR = 0x45670123UL;
	FLASH->KEYR = 0xCDEF89ABUL;
}

void FlashLock(void) {
	FLASH->CR |= FLASH_CR_LOCK;
}

uint32_t FormatFlashPage(uint32_t page) {
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	
	return FLASH_ErasePage(page) == FLASH_COMPLETE;
}

uint32_t WriteFlash(uint32_t page, uint8_t *data, uint16_t size) {
	uint16_t i;
	uint16_t tmp;
	
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

	for(i = 0; i < size; i += 2) {
		tmp = data[i] | (data[i + 1] << 8);
		if (FLASH_ProgramHalfWord(page + i, tmp) != FLASH_COMPLETE) {
			return 0;
		}
	}
	
	return 1;
}
