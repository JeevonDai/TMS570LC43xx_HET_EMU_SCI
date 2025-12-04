/*----------------------------------------------------------------------------*/
/* sys_link_sram.cmd                                                          */
/*                                                                            */
/* SRAM版本的链接脚本 - 用于将程序加载到SRAM中运行                            */
/* 通过JTAG调试器直接加载到SRAM，无需写入Flash                                */
/*                                                                            */
/* 
* Copyright (C) 2009-2018 Texas Instruments Incorporated - www.ti.com  
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
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

/*                                                                            */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN (0) */
/* USER CODE END */


/*----------------------------------------------------------------------------*/
/* Linker Settings                                                            */

--retain="*(.intvecs)"

/* USER CODE BEGIN (1) */
/* USER CODE END */

/*----------------------------------------------------------------------------*/
/* Memory Map                                                                 */
/* TMS570LC43xx SRAM: 0x08000000 - 0x08080000 (512KB)                        */
/*                                                                            */
/* 内存布局 (全部在SRAM中):                                                   */
/*   VECTORS: 0x08000000 - 0x08000020 (32 bytes) - 中断向量表                 */
/*   CODE:    0x08000020 - 0x08040000 (~256KB)   - 代码和只读数据             */
/*   STACKS:  0x08040000 - 0x08041500 (~5KB)     - 栈区                       */
/*   RAM:     0x08041500 - 0x08080000 (~252KB)   - 数据区                     */

MEMORY
{
/* USER CODE BEGIN (2) */
/* USER CODE END */
    /* 所有内容都放在SRAM中 */
    VECTORS (X)  : origin=0x08000000 length=0x00000020
    CODE    (RX) : origin=0x08000020 length=0x0003FFE0
    STACKS  (RW) : origin=0x08040000 length=0x00001500
    RAM     (RW) : origin=0x08041500 length=0x0003EB00

/* USER CODE BEGIN (3) */
/* USER CODE END */
}

/* USER CODE BEGIN (4) */
/* USER CODE END */


/*----------------------------------------------------------------------------*/
/* Section Configuration                                                      */

SECTIONS
{
/* USER CODE BEGIN (5) */
/* USER CODE END */
    .intvecs : {} > VECTORS
    .text   align(32) : {} > CODE
    .const  align(32) : {} > CODE
    .cinit  align(32) : {} > CODE
    .pinit  align(32) : {} > CODE
    .bss     : {} > RAM
    .data    : {} > RAM
    .sysmem  : {} > RAM
	

/* USER CODE BEGIN (6) */
/* USER CODE END */
}

/* USER CODE BEGIN (7) */
/* USER CODE END */


/*----------------------------------------------------------------------------*/
/* Misc                                                                       */

/* USER CODE BEGIN (8) */
/* USER CODE END */
/*----------------------------------------------------------------------------*/


