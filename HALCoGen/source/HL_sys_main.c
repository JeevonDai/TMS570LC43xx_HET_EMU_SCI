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

#include "HL_hal_stdtypes.h"
#include "HL_sys_common.h"

/* USER CODE BEGIN (1) */
#include "HL_sci.h"
#include "HL_system.h"
#include "jtag_gpio.h"  // JTAG GPIO 模拟头文件

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

// #define CNT 5000000
#define CNT 5000000

void sci_Printf(char* format, ...);
/* USER CODE END */

int main(void)
{
    /* USER CODE BEGIN (3) */
    int i;
    int count = 0;
    uint32 idcode = 0;
    uint8 jtag_test_done = 0;

    // 初始化 SCI 串口
    sciInit();
    
    // 初始化 JTAG GPIO
    JTAG_GPIO_Init();
    
    // sci_Printf("====================================\r\n");
    // sci_Printf("TMS570LC4357 JTAG GPIO 模拟系统\r\n");
    // sci_Printf("====================================\r\n");
    // sci_Printf("引脚映射:\r\n");
    // sci_Printf("  B3  (N2HET1_22) -> TRST 输出\r\n");
    // sci_Printf("  J4  (N2HET1_23) -> TCK  输出\r\n");
    // sci_Printf("  P1  (N2HET1_24) -> TDI  输出\r\n");
    // sci_Printf("  A9  (N2HET1_27) -> TDO  输入\r\n");
    // sci_Printf("  A3  (N2HET1_29) -> TMS  输出\r\n");
    sci_Printf("====================================\r\n\r\n");
    
    while (1) {
        count++;
        
        // 每隔一定次数执行一次 JTAG 测试
        if (count % 10 == 1 && !jtag_test_done) {
            sci_Printf("开始 JTAG 测试...\r\n");
            
            // 复位 JTAG
            JTAG_Reset();
            sci_Printf("  [1] JTAG 复位完成\r\n");
            
            // 进入空闲状态
            JTAG_Goto_Idle();
            sci_Printf("  [2] 进入 Run-Test/Idle 状态\r\n");
            
            /* 从 Idle -> Select-DR-Scan */
            JTAG_Shift_Bit(1, 0);
            
            idcode = JTAG_Read_DR_Pause(32);
            sci_Printf("  [3] 读取 ICEPick IDCODE: 0x%08X\r\n", idcode);
            
            jtag_test_done = 1;  // 只执行一次测试
            // 解析 IDCODE
            if (idcode == 0 || idcode == 0xFFFFFFFF) {
                sci_Printf("  [×] JTAG 连接失败或未连接目标芯片\r\n");
                sci_Printf("      请检查:\r\n");
                sci_Printf("      1. 目标芯片是否供电\r\n");
                sci_Printf("      2. JTAG 引脚连接是否正确\r\n");
                sci_Printf("      3. 两块芯片是否共地\r\n\r\n");
                continue;
            }

            uint32 version = (idcode >> 28) & 0x0F;
            uint32 part_num = (idcode >> 12) & 0xFFFF;
            uint32 mfg_id = (idcode >> 1) & 0x7FF;
            
            sci_Printf("      - 版本号：0x%X\r\n", version);
            sci_Printf("      - 器件型号：0x%04X\r\n", part_num);
            sci_Printf("      - 制造商 ID: 0x%03X\r\n", mfg_id);
            
            if (mfg_id == 0x017) {
                sci_Printf("      - 制造商：Texas Instruments\r\n");
            }
            sci_Printf("  [✓] ICEPick 连接成功!\r\n\r\n");
            
            sci_Printf("  [4] 开始通过 ICEPick 路由到 DAP...\r\n");

            // sci_Printf("  [✓] DAP 路由配置完成\r\n");
        }
        
        // 周期性输出心跳信息
        // sci_Printf("运行中 count = %d\r\n", count);
        
        // 延时
        for (i = 0; i < CNT; i++)
            ;
            
        // 重置测试标志，以便定期重新测试
        if (count >= 100) {
            count = 0;
            jtag_test_done = 0;
        }
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
