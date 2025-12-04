/** @file HL_sys_main.c 
*   @brief Application main file
*   @date 11-Dec-2018
*   @version 04.07.01
*
*   This file contains an empty main function,
*   which can be used for the application.
*/

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

/* USER CODE BEGIN (0) */
/* USER CODE END */

/* Include Files */

#include "HL_sys_common.h"

/* USER CODE BEGIN (1) */
#include "HL_sci.h"
#include "HL_system.h"

#include <stdio.h>
#include <string.h>
/* USER CODE END */

/** @fn void main(void)
*   @brief Application main function
*   @note This function is empty by default.
*
*   This function is called after startup.
*   The user can use this function to implement the application.
*/

/* USER CODE BEGIN (2) */
#define SCI_REG sciREG1  // 定义 sci 端口寄存器

#define CNT 5000000

#define SRAM_BIN_SIZE 35152U            /* bin 文件大小（字节）*/
#define TARGET_SRAM_BASE 0x08000000U    /* 目标芯片 SRAM 起始地址 */
#define SRAM_BIN_WORDS ((SRAM_BIN_SIZE + 3) / 4) /* 按 32 位字计算 */

void sci_Printf(char* format, ...);
/* USER CODE END */

int main(void)
{
    /* USER CODE BEGIN (3) */
    int i;
    int count = 0;

    sciInit();
    uint32* sram_ptr = (uint32*)0x08000000U;

    /* 
        * 循环写入每个 32 位字
        * 注意: TAR 已在步骤 [12] 设置为 0x08000000
        * CSW 配置了自动递增，每次写入 DRW 后地址自动 +4
        */
    while (1) {
        count++;                                            // 计数
        sci_Printf("Hello world, count = %d.\r\n", count);  // 使用自定义 printf 函数输出
        for (i = 0; i < SRAM_BIN_WORDS; i++) {
            /* 读取 SRAM 中的数据 */
            uint32 sram_data = sram_ptr[i];
            if(i % 0x400 == 0) {
                sci_Printf("      - SRAM 数据: 0x%08X\r\n", sram_ptr[i]);
            }
        }
        for (i = 0; i < CNT; i++)
            ;  // 粗略延时
    }
    /* USER CODE END */

    return 0;
}

/* USER CODE BEGIN (4) */
/*
 * @brief       : 自定义 SCI printf 函数
 * @param       : 字符串，可实现类似于 printf 的参数输入
 * @return      : void
 * @author      : Liu Jiahao
 * @date        : 2024-03-26
 * @version     : v1.1
 * @copyright   : Copyright By Liu Jiahao, All Rights Reserved
 */
void sci_Printf(char* format, ...)
{
    uint16 i;
    va_list listdata;
    uint8 sci_TxBuff[100];

    va_start(listdata, format);
    vsprintf((char*)sci_TxBuff, format, listdata);
    va_end(listdata);

    for (i = 0; i < strlen((const char*)sci_TxBuff); i++) {
        while ((SCI_REG->FLR & 0x04U) == 4U)
            ;
        sciSendByte(SCI_REG, sci_TxBuff[i]);
    }
}
/* USER CODE END */
