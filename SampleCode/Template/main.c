/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include <string.h>
#include "NuMicro.h"

#include	"project_config.h"

#include "DataFlashProg.h"
/*_____ D E C L A R A T I O N S ____________________________________________*/
#define PLL_CLOCK   							72000000

/*_____ D E F I N I T I O N S ______________________________________________*/
volatile uint32_t BitFlag = 0;
volatile uint32_t counter_tick = 0;

uint32_t Storage_Block[BUFFER_PAGE_SIZE / 4] = {0};
/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

void tick_counter(void)
{
	counter_tick++;
}

uint32_t get_tick(void)
{
	return (counter_tick);
}

void set_tick(uint32_t t)
{
	counter_tick = t;
}

void compare_buffer(uint8_t *src, uint8_t *des, int nBytes)
{
    uint16_t i = 0;	
	
    for (i = 0; i < nBytes; i++)
    {
        if (src[i] != des[i])
        {
            printf("error idx : %4d : 0x%2X , 0x%2X\r\n", i , src[i],des[i]);
			set_flag(flag_error , ENABLE);
        }
    }

	if (!is_flag_set(flag_error))
	{
    	printf("%s finish \r\n" , __FUNCTION__);	
		set_flag(flag_error , DISABLE);
	}

}

void reset_buffer(void *dest, unsigned int val, unsigned int size)
{
    uint8_t *pu8Dest;
//    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;

	#if 1
	while (size-- > 0)
		*pu8Dest++ = val;
	#else
	memset(pu8Dest, val, size * (sizeof(pu8Dest[0]) ));
	#endif
	
}

void copy_buffer(void *dest, void *src, unsigned int size)
{
    uint8_t *pu8Src, *pu8Dest;
    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;
    pu8Src  = (uint8_t *)src;


	#if 0
	  while (size--)
	    *pu8Dest++ = *pu8Src++;
	#else
    for (i = 0; i < size; i++)
        pu8Dest[i] = pu8Src[i];
	#endif
}

void dump_buffer(uint8_t *pucBuff, int nBytes)
{
    uint16_t i = 0;
    
    printf("dump_buffer : %2d\r\n" , nBytes);    
    for (i = 0 ; i < nBytes ; i++)
    {
        printf("0x%2X," , pucBuff[i]);
        if ((i+1)%8 ==0)
        {
            printf("\r\n");
        }            
    }
    printf("\r\n\r\n");
}

void  dump_buffer_hex(uint8_t *pucBuff, int nBytes)
{
    int     nIdx, i;

    nIdx = 0;
    while (nBytes > 0)
    {
        printf("0x%04X  ", nIdx);
        for (i = 0; i < 16; i++)
            printf("%02X ", pucBuff[nIdx + i]);
        printf("  ");
        for (i = 0; i < 16; i++)
        {
            if ((pucBuff[nIdx + i] >= 0x20) && (pucBuff[nIdx + i] < 127))
                printf("%c", pucBuff[nIdx + i]);
            else
                printf(".");
            nBytes--;
        }
        nIdx += 16;
        printf("\n");
    }
    printf("\n");
}

void delay(uint16_t dly)
{
/*
	delay(100) : 14.84 us
	delay(200) : 29.37 us
	delay(300) : 43.97 us
	delay(400) : 58.5 us	
	delay(500) : 73.13 us	
	
	delay(1500) : 0.218 ms (218 us)
	delay(2000) : 0.291 ms (291 us)	
*/

	while( dly--);
}


void delay_ms(uint16_t ms)
{
	TIMER_Delay(TIMER0, 1000*ms);
}


void GPIO_Init (void)
{
    SYS->GPB_MFP &= ~(SYS_GPB_MFP_PB4_Msk);
    SYS->GPB_MFP |= (SYS_GPB_MFP_PB4_GPIO);
	
    GPIO_SetMode(PB, BIT4, GPIO_PMD_OUTPUT);

}

void loop_test(void)
{
	uint32_t idx = 0;
	uint32_t i = 0;
	
	if (is_flag_set(flag_read_wrte_test))
	{
		set_flag(flag_read_wrte_test , DISABLE);

		idx = 0xF0;
		DataFlashRead(idx , BUFFER_PAGE_SIZE , (uint32_t) &Storage_Block[0]);
		printf("0x%2X : 0x%4X \r\n" , idx , Storage_Block[0] );
		Storage_Block[0] += 2;
		DataFlashWrite(idx , BUFFER_PAGE_SIZE , (uint32_t) &Storage_Block[0]);		

		idx = 0x10;
		DataFlashRead(idx , BUFFER_PAGE_SIZE , (uint32_t) &Storage_Block[0]);
		printf("0x%2X : 0x%4X \r\n" , idx , Storage_Block[0] );
		Storage_Block[0] += 1;
		DataFlashWrite(idx , BUFFER_PAGE_SIZE , (uint32_t) &Storage_Block[0]);		
		
		idx = 0x20;
		DataFlashRead(idx , BUFFER_PAGE_SIZE , (uint32_t) &Storage_Block[0]);
		printf("0x%2X : 0x%4X \r\n" , idx , Storage_Block[0] );
		Storage_Block[0] += 1;
		DataFlashWrite(idx , BUFFER_PAGE_SIZE , (uint32_t) &Storage_Block[0]);	

		// need to align uint8_t to uint32_t  
		for (i = 0 ; i < 16 ; i++)
		{
			idx = 0x30 + i*4;
			DataFlashRead(idx , BUFFER_PAGE_SIZE , (uint32_t) &Storage_Block[0] );
			printf("0x%2X : 0x%4X  , " , idx , Storage_Block[0]);
			Storage_Block[0] += 1;
			DataFlashWrite(idx , BUFFER_PAGE_SIZE , (uint32_t) &Storage_Block[0] );
	        if ((i+1)%4 ==0)
	        {
	            printf("\r\n");
	        }  			
		}
		printf("\r\n");
	}
}

void TMR1_IRQHandler(void)
{
	static uint32_t LOG = 0;

	
    if(TIMER_GetIntFlag(TIMER1) == 1)
    {
        TIMER_ClearIntFlag(TIMER1);
		tick_counter();

		if ((get_tick() % 1000) == 0)
		{
        	printf("%s : %4d\r\n",__FUNCTION__,LOG++);
			PB4 ^= 1;

			set_flag(flag_read_wrte_test , ENABLE);				
		}

		if ((get_tick() % 50) == 0)
		{

		}	
    }
}


void TIMER1_Init(void)
{
    TIMER_Open(TIMER1, TIMER_PERIODIC_MODE, 1000);
    TIMER_EnableInt(TIMER1);
    NVIC_EnableIRQ(TMR1_IRQn);	
    TIMER_Start(TIMER1);
}

void UARTx_Process(void)
{
	uint8_t res = 0;
	res = UART_READ(UART0);

	if (res == 'x' || res == 'X')
	{
		NVIC_SystemReset();
	}

	if (res > 0x7F)
	{
		printf("invalid command\r\n");
	}
	else
	{
		switch(res)
		{
			case '1':
				break;

			case 'X':
			case 'x':
			case 'Z':
			case 'z':
				NVIC_SystemReset();		
				break;
		}
	}
}

void UART02_IRQHandler(void)
{

    if(UART_GET_INT_FLAG(UART0, UART_ISR_RDA_INT_Msk | UART_ISR_TOUT_IF_Msk))     /* UART receive data available flag */
    {
        while(UART_GET_RX_EMPTY(UART0) == 0)
        {
            UARTx_Process();
        }
    }

}

void UART0_Init(void)
{
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
    UART_EnableInt(UART0, UART_IER_RDA_IEN_Msk | UART_IER_TOUT_IEN_Msk);
    NVIC_EnableIRQ(UART02_IRQn);
	
	#if (_debug_log_UART_ == 1)	//debug
	printf("\r\nCLK_GetCPUFreq : %8d\r\n",CLK_GetCPUFreq());
	printf("CLK_GetHXTFreq : %8d\r\n",CLK_GetHXTFreq());
	printf("CLK_GetLXTFreq : %8d\r\n",CLK_GetLXTFreq());	
	printf("CLK_GetHCLKFreq : %8d\r\n",CLK_GetHCLKFreq());
	printf("CLK_GetPCLKFreq : %8d\r\n",CLK_GetPCLKFreq());	
	#endif	

}

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    CLK_EnableXtalRC(CLK_PWRCON_OSC22M_EN_Msk);
    CLK_WaitClockReady(CLK_CLKSTATUS_OSC22M_STB_Msk);

    /* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HIRC, CLK_CLKDIV_HCLK(1));

    /* Enable external XTAL 12MHz clock */
    CLK_EnableXtalRC(CLK_PWRCON_XTL12M_EN_Msk);

    /* Waiting for external XTAL clock ready */
    CLK_WaitClockReady(CLK_CLKSTATUS_XTL12M_STB_Msk);

    /* Set core clock as PLL_CLOCK from PLL */
    CLK_SetCoreClock(PLL_CLOCK);

    CLK_EnableModuleClock(UART0_MODULE);
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART_S_HIRC, CLK_CLKDIV_UART(1));
	
    CLK_EnableModuleClock(TMR1_MODULE);
  	CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR1_S_HIRC, 0);

    SYS->GPB_MFP &= ~(SYS_GPB_MFP_PB0_Msk | SYS_GPB_MFP_PB1_Msk);

    SYS->GPB_MFP |= (SYS_GPB_MFP_PB0_UART0_RXD | SYS_GPB_MFP_PB1_UART0_TXD);

   /* Update System Core Clock */
    SystemCoreClockUpdate();

    /* Lock protected registers */
    SYS_LockReg();
}

int main()
{
	uint32_t i = 0;

    SYS_Init();

	UART0_Init();
	GPIO_Init();
	TIMER1_Init();

	DataFlashInit();

	Storage_Block[0] = 0x0A;	
	DataFlashWrite(0xF0 , BUFFER_PAGE_SIZE , (uint32_t)&Storage_Block[0] );

	Storage_Block[0] = 0x1A;	
	DataFlashWrite(0x10 , BUFFER_PAGE_SIZE , (uint32_t)&Storage_Block[0] );
	
	Storage_Block[0] = 0x2A;
	DataFlashWrite(0x20 , BUFFER_PAGE_SIZE , (uint32_t)&Storage_Block[0] );

	for ( i = 0 ; i < 16 ; i++)
	{
		Storage_Block[0] = 0x3A + i;
		DataFlashWrite(0x30 + i*4 , BUFFER_PAGE_SIZE , (uint32_t)&Storage_Block[0] );
	}	

    while(1)
    {
		loop_test();
		
    }
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
