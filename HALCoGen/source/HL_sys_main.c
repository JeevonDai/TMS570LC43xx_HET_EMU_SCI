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
    
    while (1) {
        count++;
        
        // 每隔一定次数执行一次 JTAG 测试
        if (count % 10 == 1 && !jtag_test_done) {
            sci_Printf("\r\n\r\n====================================\r\n\r\n");
            sci_Printf("开始 JTAG 测试...\r\n");
            
            // 复位 JTAG
            JTAG_Reset();
            sci_Printf("  [1] JTAG 复位完成\r\n");
            
            // 进入空闲状态
            JTAG_Goto_Idle();
            sci_Printf("  [2] 进入 Run-Test/Idle 状态\r\n");
            
            /* 从 Idle -> Select-DR-Scan */
            JTAG_From_Idle_To_Select_DR_Scan();
            
            idcode = JTAG_Read_DR_Pause(32);
            sci_Printf("  [3] 读取 ICEPick IDCODE: 0x%08X\r\n", idcode);
            
            jtag_test_done = 1;  // 只执行一次测试
            // 解析 IDCODE
            if (idcode == 0 || idcode == 0xFFFFFFFF) {
                sci_Printf("  [✗] JTAG 连接失败或未连接目标芯片\r\n");
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
            sci_Printf("  [✓] JTAG 已连接！\r\n\r\n");
            
            sci_Printf("  [4] 开始通过 ICEPick 路由到 DAP...\r\n");

            // 发送 CONNECT 指令
            if (JTAG_ICEPick_Connect()) {
                sci_Printf("      - CONNECT 指令已发送\r\n");
                
                // 读取 DCON 寄存器验证
                uint32 dcon = JTAG_ICEPick_Read_DCON();
                sci_Printf("      - DCON 寄存器值：0x%02X\r\n", dcon);
                
                // 检查连接状态
                uint32 connect_key = dcon & 0x0F;
                if (connect_key == 0x09) {  // 1001b
                    sci_Printf("  [✓] ICEPick 已连接！(CONNECTKEY = 1001b)\r\n\r\n");
                } else {
                    sci_Printf("  [✗] ICEPick 未连接 (CONNECTKEY = 0x%X)\r\n\r\n", connect_key);
                }
            }

            // 发送 ROUTE 指令
            JTAG_From_Pause_To_Select_DR_Scan();
            JTAG_Write_IR_Pause(0x2, 6);

            JTAG_From_Pause_To_Select_DR_Scan();
            JTAG_Write_DR_Pause(0xA0002108, 32);

            JTAG_From_Pause_To_Select_DR_Scan();

            JTAG_Write_IR_Pause(0x3F, 6);
            
            // 先从 Pause-IR 回到 Idle，然后在 Idle 状态等待 10 个时钟
            JTAG_From_Pause_To_Idle();
            
            // 在 Run-Test/Idle 状态下等待 10 个时钟周期
            // 让硬件有时间将 SDTAP0 加入扫描链
            JTAG_Set_TMS(0);  // 确保停留在 Idle 状态
            for (i = 0; i < 10; i++)
            {
                JTAG_Clock_Pulse();
            }

            sci_Printf("  [5] 读取 DAP (CPU) IDCODE...\r\n");

            // 从 Idle 状态进入 Select-DR-Scan
            JTAG_From_Idle_To_Select_DR_Scan();

            // 读取 DR（默认 IDCODE 指令）
            // 注意：需要读取 32+1=33 位（32 位 IDCODE + 1 位 ICEPick bypass）
            uint32 dap_idcode_raw = JTAG_Read_DR_Pause(33);

            // 提取实际的 IDCODE（前 32 位）
            uint32 dap_idcode = dap_idcode_raw & 0xFFFFFFFF;

            sci_Printf("      - DAP IDCODE: 0x%08X\r\n", dap_idcode);

            // 解析 DAP IDCODE
            if (dap_idcode != 0 && dap_idcode != 0xFFFFFFFF) {
                uint32 dap_version = (dap_idcode >> 28) & 0x0F;
                uint32 dap_part = (dap_idcode >> 12) & 0xFFFF;
                uint32 dap_mfg = (dap_idcode >> 1) & 0x7FF;
                
                sci_Printf("      - 版本号：0x%X\r\n", dap_version);
                sci_Printf("      - 器件型号：0x%04X\r\n", dap_part);
                sci_Printf("      - 制造商 ID: 0x%03X\r\n", dap_mfg);
                
                if (dap_mfg == 0x23B) {
                    sci_Printf("      - 制造商：ARM CoreSight\r\n");
                }
                sci_Printf("  [✓] DAP (CPU) IDCODE 读取成功！\r\n\r\n");

                // ===== 新增：DPACC 访问测试 =====
                sci_Printf("  [6] 测试 DPACC 访问...\r\n");

                // 通过 DPACC 读取 IDCODE
                uint32 dp_idcode = 0;
                uint32 ack = JTAG_DPACC_Read(DP_ADDR_IDCODE, &dp_idcode);
                sci_Printf("      - DPACC Read ACK: 0x%X\r\n", ack);
                sci_Printf("      - DP IDCODE: 0x%08X\r\n", dp_idcode);

                if (ack == 0x2) {  // DPACC_ACK_OK
                    sci_Printf("  [✓] DPACC 读取成功！\r\n\r\n");

                    // 读取 CTRL/STAT 寄存器
                    uint32 ctrl_stat = 0;
                    ack = JTAG_DPACC_Read(DP_ADDR_CTRL_STAT, &ctrl_stat);
                    sci_Printf("  [7] CTRL/STAT 寄存器状态:\r\n");
                    sci_Printf("      - ACK: 0x%X\r\n", ack);
                    sci_Printf("      - CTRL/STAT: 0x%08X\r\n", ctrl_stat);

                    // 检查电源状态
                    uint32 sys_pwr_ack = (ctrl_stat >> 31) & 0x01U;
                    uint32 dbg_pwr_ack = (ctrl_stat >> 29) & 0x01U;
                    sci_Printf("      - 系统电源确认:%s\r\n", sys_pwr_ack ? "是" : "否");
                    sci_Printf("      - 调试电源确认:%s\r\n", dbg_pwr_ack ? "是" : "否");

                    // 如果电源未上电，尝试上电
                    if (!sys_pwr_ack || !dbg_pwr_ack) {
                        sci_Printf("\r\n  [8] 正在上电 DAP...\r\n");
                        if (JTAG_DAP_PowerUp()) {
                            sci_Printf("  [✓] DAP 上电成功！\r\n");

                            // 重新读取 CTRL/STAT 确认
                            ack = JTAG_DPACC_Read(DP_ADDR_CTRL_STAT, &ctrl_stat);
                            sci_Printf("      - 上电后 CTRL/STAT: 0x%08X\r\n", ctrl_stat);
                            sys_pwr_ack = (ctrl_stat >> 31) & 0x01U;
                            dbg_pwr_ack = (ctrl_stat >> 29) & 0x01U;
                            sci_Printf("      - 系统电源确认:%s\r\n", sys_pwr_ack ? "是" : "否");
                            sci_Printf("      - 调试电源确认:%s\r\n\r\n", dbg_pwr_ack ? "是" : "否");
                        }
                        else {
                            sci_Printf("  [✗] DAP 上电失败\r\n\r\n");
                        }
                    }
                    else {
                        sci_Printf("  [✓] DAP 已经上电\r\n\r\n");
                    }
                }
                else {
                    sci_Printf("  [✗] DPACC 访问失败 (ACK=0x%X)\r\n\r\n", ack);
                }
            } else {
                sci_Printf("  [✗] DAP IDCODE 读取失败\r\n\r\n");
            }
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
