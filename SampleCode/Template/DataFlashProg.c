/****************************************************************************
 * @file     DataFlashProg.c
 * @brief    Data Flash Access API
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/


/*---------------------------------------------------------------------------------------------------------*/
/* Includes of system headers                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "NuMicro.h"
#include "DataFlashProg.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
extern int IsDebugFifoEmpty(void);

uint32_t g_sectorBuf[FLASH_PAGE_SIZE / 4];


void DataFlashInit(void)
{
    uint32_t au32Config[2];
	uint32_t u32Data = 0;

    SYS_UnlockReg();
    FMC_Open();

	#if 1
    /* Check if Data Flash Size is 64K. If not, to re-define Data Flash size and to enable Data Flash function */
    if (FMC_ReadConfig(au32Config, 2) < 0)
    {
		printf("%s : 1\r\n" , __FUNCTION__ );
    }

    if (((au32Config[0] & 0x01) == 1) || (au32Config[1] != DATA_FLASH_BASE) )
    {
        FMC_ENABLE_CFG_UPDATE();
        au32Config[0] &= ~0x1;
        au32Config[1] = DATA_FLASH_BASE;
        if (FMC_WriteConfig(au32Config, 2) < 0)
	    {
			printf("%s : 2\r\n" , __FUNCTION__ );
	    }

        FMC_ReadConfig(au32Config, 2);
        if (((au32Config[0] & 0x01) == 1) || (au32Config[1] != DATA_FLASH_BASE))
        {
            printf("Error: Program Config Failed!\n");
            /* Disable FMC ISP function */
            FMC_Close();
            SYS_LockReg();
		    {
				printf("%s : 3\r\n" , __FUNCTION__ );
		    }

        }

        printf("chip reset\n");

        /* To check if all the debug messages are finished */
        while(!IsDebugFifoEmpty());
		
        /* Reset Chip to reload new CONFIG value */
        SYS->IPRSTC1 |= SYS_IPRSTC1_CHIP_RST_Msk;
    }
	#endif

    u32Data = FMC_ReadDataFlashBaseAddr();
    printf("Data Flash Base Address [0x%08X]\r\n", u32Data);
	
    FMC_Close();
    SYS_LockReg();
}


uint32_t FMC_ReadPage(uint32_t u32StartAddr, uint32_t * u32Buf)
{
    uint32_t i;

    for(i = 0; i < FLASH_PAGE_SIZE / 4; i++)
        u32Buf[i] = FMC_Read(u32StartAddr + i * 4);

    return 0;
}

void DataFlashReadPage(uint32_t addr, uint32_t buffer)
{
    uint32_t i;
    uint32_t * pu32Buf = (uint32_t *)buffer;

    /* Modify the address to MASS_STORAGE_OFFSET */
    addr += DATA_FLASH_BASE;

    for(i = 0; i < FLASH_PAGE_SIZE / 4; i++)
        pu32Buf[i] = FMC_Read(addr + i * 4);
}

void DataFlashRead(uint32_t addr, uint32_t size, uint32_t buffer)
{
    /* This is low level read function of USB Mass Storage */
    int32_t len;
    uint32_t i;
    uint32_t * pu32Buf = (uint32_t *)buffer;
	
    /* Modify the address to DATA_FLASH_BASE */
    addr += DATA_FLASH_BASE;

    len = (int32_t)size;

    SYS_UnlockReg();
    FMC_Open();
    FMC_ENABLE_AP_UPDATE();
	
    while(len >= BUFFER_PAGE_SIZE)
    {
        for(i = 0; i < BUFFER_PAGE_SIZE / 4; i++)
            pu32Buf[i] = FMC_Read(addr + i * 4);
        addr   += BUFFER_PAGE_SIZE;
        buffer += BUFFER_PAGE_SIZE;
        len  -= BUFFER_PAGE_SIZE;
        pu32Buf = (uint32_t *)buffer;
    }

	FMC_DISABLE_AP_UPDATE();
    FMC_Close();
    SYS_LockReg();
}


uint32_t DataFlashProgramPage(uint32_t u32StartAddr, uint32_t * u32Buf)
{
    uint32_t i;

    for(i = 0; i < FLASH_PAGE_SIZE / 4; i++)
    {
        FMC_Write(u32StartAddr + i * 4, u32Buf[i]);
    }

    return 0;
}


void DataFlashWrite(uint32_t addr, uint32_t size, uint32_t buffer)
{
    /* This is low level write function of USB Mass Storage */
    int32_t len, i, offset;
    uint32_t *pu32;
    uint32_t alignAddr;

    /* Modify the address to DATA_FLASH_BASE */
    addr += DATA_FLASH_BASE;

    len = (int32_t)size;

    SYS_UnlockReg();
    FMC_Open();
    FMC_ENABLE_AP_UPDATE();
	
    if(len == FLASH_PAGE_SIZE && ((addr & (FLASH_PAGE_SIZE - 1)) == 0))
    {
        FMC_Erase(addr);

        while(len >= FLASH_PAGE_SIZE)
        {
            DataFlashProgramPage(addr, (uint32_t *) buffer);
            len    -= FLASH_PAGE_SIZE;
            buffer += FLASH_PAGE_SIZE;
            addr   += FLASH_PAGE_SIZE;
        }
    }
    else
    {
        do
        {
            alignAddr = addr & 0x1FE00;

            /* Get the sector offset*/
            offset = (addr & (FLASH_PAGE_SIZE - 1));

            if(offset || (size < FLASH_PAGE_SIZE))
            {
                /* Non 4k alignment. Note: It needs to avoid add DATA_FLASH_BASE twice. */
//                DataFlashRead(alignAddr - DATA_FLASH_BASE, FLASH_PAGE_SIZE, (uint32_t)&g_sectorBuf[0]);
				DataFlashReadPage(alignAddr - DATA_FLASH_BASE, /*FLASH_PAGE_SIZE,*/ (uint32_t)&g_sectorBuf[0]);
            }

            /* Update the data */
            pu32 = (uint32_t *)buffer;
//			printf("pu32 : 0x%4X , 0x%4X , 0x%4X\r\n" , pu32[0] , buffer , &buffer);
			
            len = FLASH_PAGE_SIZE - offset;
            if(size < len)
                len = size;

            for(i = 0; i < len / 4; i++)
            {
                g_sectorBuf[offset / 4 + i] = pu32[i];
            }
//			printf("g_sectorBuf : 0x%4X\r\n" , g_sectorBuf[0]);

            FMC_Erase(alignAddr);

			#if 1
            DataFlashProgramPage(alignAddr, (uint32_t *) g_sectorBuf);
			#else
            for(i = 0; i < 16; i++)
            {
                FMC_ProgramPage(alignAddr + (i << 8), (uint32_t *) g_sectorBuf + (i << 8));
            }
			#endif

            size -= len;
            addr += len;
            buffer += len;

        }
        while(size > 0);
    }

    FMC_DISABLE_AP_UPDATE(); 	
    FMC_Close();
    SYS_LockReg();
}

