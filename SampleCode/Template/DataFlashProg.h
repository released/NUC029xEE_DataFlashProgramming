/******************************************************************************
 * @file     DataFlashProg.h
 * @brief    Data flash programming driver header
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef __DATA_FLASH_PROG_H__
#define __DATA_FLASH_PROG_H__

#define CONFIG_BASE      			0x00300000
#define DATA_FLASH_BASE  			(0x20000 - DATA_FLASH_STORAGE_SIZE)  			/* To avoid the code to write APROM */
#define DATA_FLASH_STORAGE_SIZE   	(BUFFER_PAGE_SIZE*FLASH_PAGE_SIZE)  				/* Configure the DATA FLASH storage size. To pass USB-IF MSC Test, it needs > 64KB */
#define FLASH_PAGE_SIZE           	(FMC_FLASH_PAGE_SIZE)
#define BUFFER_PAGE_SIZE          	(4)

void DataFlashInit(void);
void DataFlashWrite(uint32_t addr, uint32_t size, uint32_t buffer);
void DataFlashRead(uint32_t addr, uint32_t size, uint32_t buffer);



#endif  /* __DATA_FLASH_PROG_H__ */

/*** (C) COPYRIGHT 2018 Nuvoton Technology Corp. ***/
