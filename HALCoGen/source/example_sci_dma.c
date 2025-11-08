/** @example example_sci_dma.c
*   This is an example which descibes the steps to create an example application which
*   configures SCI transfer using DMA. The transmit buffer of the Mibspi RAM is
*   loaded with data through the DMA.
*
*   @b Step @b 1:
*
*   Create a new project.
*
*   Navigate: -> File -> New -> Project
*
*   @image html example_createProject.jpg "Figure: Create a new Project"
*
*   @b Step @b 2:
*
*   Configure driver code generation:
*   - Enable SCI driver
*   - Disable others
*
*   Navigate: -> TMS570LCxx /RM5x -> Enable Drivers
*
*   @b Step @b 3:
*
*   Configure SCI3:
*
*   Navigate: -> TMS570Lxx /RMx -> SCI3
*
*   @image html sci_uart.JPG   "Figure: SCI Global settings"
*
*
*   @image html sci_uart1.JPG  "Figure: SCI Data Format"
*
*   The example file example_sciDma.c can also be found in the examples folder: ../HALCoGen/examples
*
*   @note HALCoGen generates an enpty main function in sys_main.c,
*         please make sure that you link in the right main function or copy the source into the user code sections of this file.
*
*/


/* 
* Copyright (C) 2009-2015 Texas Instruments Incorporated - www.ti.com
* 
* 
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*
*    Redistributions of source code must retain the above copyright 
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the 
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  INCLUDING NEGLIGENCE OR OTHERWISE ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

/* USER CODE BEGIN (0) */
/* USER CODE END */

/* Include Files */

#include "HL_sys_common.h"
#include "HL_system.h"

/* USER CODE BEGIN (1) */
#include "HL_sys_dma.h"
#include "HL_sci.h"
#include "stdio.h"
/* USER CODE END */

/** @fn void main(void)
*   @brief Application main function
*   @note This function is empty by default.
*
*   This function is called after startup.
*   The user can use this function to implement the application.
*/

/* USER CODE BEGIN (2) */
#define size 100
/* External connection (SCI3 TX -> SCI4 RX) is needed in case LOOPBACKMODE is defined as 0 */
#define LOOPBACKMODE 1

/* Tx and Rx data buffer */
uint8_t TX_DATA[size], RX_DATA[size] = {0};

/* Addresses of SCI 8-bit TX/Rx data */
#if ((__little_endian__ == 1) || (__LITTLE_ENDIAN__ == 1))
#define SCI3_TX_ADDR ((uint32_t)(&(sciREG3->TD)))
#define SCI3_RX_ADDR ((uint32_t)(&(sciREG3->RD)))
#define SCI4_TX_ADDR ((uint32_t)(&(sciREG4->TD)))
#define SCI4_RX_ADDR ((uint32_t)(&(sciREG4->RD)))
#else
#define SCI3_TX_ADDR ((uint32_t)(&(sciREG3->TD)) + 3)
#define SCI3_RX_ADDR ((uint32_t)(&(sciREG3->RD)) + 3)
#define SCI4_TX_ADDR ((uint32_t)(&(sciREG4->TD)) + 3)
#define SCI4_RX_ADDR ((uint32_t)(&(sciREG4->RD)) + 3)
#endif

#define DMA_SCI3_TX  DMA_REQ31
#define DMA_SCI3_RX  DMA_REQ30
#define DMA_SCI4_TX  DMA_REQ43
#define DMA_SCI4_RX  DMA_REQ42

#define SCI_SET_TX_DMA      (1<<16)
#define SCI_SET_RX_DMA      (1<<17)
#define SCI_SET_RX_DMA_ALL  (1<<18)
/* USER CODE END */

uint8	emacAddress[6U] = 	{0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};
uint32 	emacPhyAddress	=	0U;

void main(void)
{
/* USER CODE BEGIN (3) */
	uint32 sciTxData, sciRxData;
	int i;
	g_dmaCTRL g_dmaCTRLPKT1, g_dmaCTRLPKT2;
	_cacheDisable_();

	/*Load source data*/
	for (i=0; i<size; i++)
	{
		TX_DATA[i] = i+1;
	}

	/*Initialize SCI*/
	sciInit();

#if LOOPBACKMODE == 1
	/* Enable SCI loopback */
	sciEnableLoopback(sciREG3, Digital_Lbk);
	while (((sciREG3->FLR & SCI_TX_INT) == 0U) || ((sciREG3->FLR & 0x4) == 0x4))
    {
	} /* Wait */

	/*Assign DMA request SCI3 transmit to Channel 0*/
	dmaReqAssign(DMA_CH0, DMA_SCI3_TX);

	/*Assign DMA request SCI3 receive to Channel 1*/
	dmaReqAssign(DMA_CH1, DMA_SCI3_RX);

	sciTxData = SCI3_TX_ADDR;
	sciRxData = SCI3_RX_ADDR;

#else
	while (((sciREG3->FLR & SCI_TX_INT) == 0U) || ((sciREG3->FLR & 0x4) == 0x4))
    {
	} /* Wait */

	/*Assign DMA request SCI3 transmit to Channel 0*/
	dmaReqAssign(DMA_CH0, DMA_SCI3_TX);

	/*Assign DMA request SCI4 receive to Channel 1*/
	dmaReqAssign(DMA_CH1, DMA_SCI4_RX);

	sciTxData = SCI3_TX_ADDR;
	sciRxData = SCI4_RX_ADDR;

#endif

	/*Configure control packet for Channel 0*/
	g_dmaCTRLPKT1.SADD      = (uint32_t)TX_DATA;	/* source address             */
	g_dmaCTRLPKT1.DADD      = sciTxData;		    /* destination  address       */
	g_dmaCTRLPKT1.CHCTRL    = 0;                	/* channel control            */
	g_dmaCTRLPKT1.FRCNT	   = size;                  /* frame count                */
	g_dmaCTRLPKT1.ELCNT     = 1;             	    /* element count              */
	g_dmaCTRLPKT1.ELDOFFSET = 0;                	/* element destination offset */
	g_dmaCTRLPKT1.ELSOFFSET = 0;		          	/* element destination offset */
	g_dmaCTRLPKT1.FRDOFFSET = 0;		          	/* frame destination offset   */
	g_dmaCTRLPKT1.FRSOFFSET = 0;                	/* frame destination offset   */
	g_dmaCTRLPKT1.PORTASGN  = PORTA_READ_PORTB_WRITE;
	g_dmaCTRLPKT1.RDSIZE    = ACCESS_8_BIT;		    /* read size                  */
	g_dmaCTRLPKT1.WRSIZE    = ACCESS_8_BIT;	 	    /* write size                 */
	g_dmaCTRLPKT1.TTYPE     = FRAME_TRANSFER;  	    /* transfer type              */
	g_dmaCTRLPKT1.ADDMODERD = ADDR_INC1;        	/* address mode read          */
	g_dmaCTRLPKT1.ADDMODEWR = ADDR_FIXED;      	    /* address mode write         */
	g_dmaCTRLPKT1.AUTOINIT  = AUTOINIT_OFF;      	/* autoinit                   */

	/*Configure control packet for Channel 1*/
	g_dmaCTRLPKT2.SADD      = sciRxData;	        /* source address             */
	g_dmaCTRLPKT2.DADD      = (uint32_t)RX_DATA;	/* destination  addr ss       */
	g_dmaCTRLPKT2.CHCTRL    = 0;                	/* channel control            */
	g_dmaCTRLPKT2.FRCNT	   = size;                  /* frame count                */
	g_dmaCTRLPKT2.ELCNT     = 1;                 	/* element count              */
	g_dmaCTRLPKT2.ELDOFFSET = 0;                	/* element destination offset */
	g_dmaCTRLPKT2.ELSOFFSET = 0;		          	/* element destination offset */
	g_dmaCTRLPKT2.FRDOFFSET = 0;		          	/* frame destination offset   */
	g_dmaCTRLPKT2.FRSOFFSET = 0;                	/* frame destination offset   */
	g_dmaCTRLPKT2.PORTASGN  = PORTB_READ_PORTA_WRITE;
	g_dmaCTRLPKT2.RDSIZE    = ACCESS_8_BIT;		    /* read size                  */
	g_dmaCTRLPKT2.WRSIZE    = ACCESS_8_BIT;	 	    /* write size                 */
	g_dmaCTRLPKT2.TTYPE     = FRAME_TRANSFER;  	    /* transfer type              */
	g_dmaCTRLPKT2.ADDMODERD = ADDR_FIXED;        	/* address mode read          */
	g_dmaCTRLPKT2.ADDMODEWR = ADDR_INC1;      	    /* address mode write         */
	g_dmaCTRLPKT2.AUTOINIT  = AUTOINIT_OFF;      	/* autoinit                   */

	/*Set control packet for channel 0 and 1*/
	dmaSetCtrlPacket(DMA_CH0, g_dmaCTRLPKT1);
	dmaSetCtrlPacket(DMA_CH1, g_dmaCTRLPKT2);

	/*Set dma channel 0 and 1 to trigger on hardware request*/
	dmaSetChEnable(DMA_CH0, DMA_HW);
	dmaSetChEnable(DMA_CH1, DMA_HW);

	/*Enable DMA*/
	dmaEnable();

#if LOOPBACKMODE == 1
	/*Enable SCI3 Transmit and Receive DMA Request*/
	sciREG3->SETINT |= SCI_SET_TX_DMA | SCI_SET_RX_DMA | SCI_SET_RX_DMA_ALL;

#else
	/*Enable SCI3 Transmit and SCI4 Receive DMA Request*/
	sciREG3->SETINT |= SCI_SET_TX_DMA;
	sciREG4->SETINT |= SCI_SET_RX_DMA | SCI_SET_RX_DMA_ALL;
#endif

	while(dmaGetInterruptStatus(DMA_CH1, BTC) != TRUE);

	for(i=0; i<size; i++)
	{
		if(RX_DATA[i] != TX_DATA[i])
		{
			break;
		}
	}
	if(i<size)
	{
		printf("Fail\n");
	}
	else
	{
		printf("Pass\n");
	}
	while(1);

/* USER CODE END */
}

/* USER CODE BEGIN (4) */
/* USER CODE END */
