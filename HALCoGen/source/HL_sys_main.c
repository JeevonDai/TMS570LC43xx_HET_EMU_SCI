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

void sci_Printf(char* format, ...);
const char* get_manufacturer_name(uint32 mfg_id);
uint32 parse_IDCODE(uint32 idcode, const char* name);
#define SUCCESS 0
#define FAILED 1

#define CONTINUE_IF(condition)                                                 \
    if (condition) continue;

// 简单字符串版本
#define CONTINUE_IF_MSG(condition, msg_if_true, msg_if_false)                  \
    if (condition) {                                                           \
        sci_Printf(msg_if_true);                                               \
        continue;                                                              \
    }                                                                          \
    else {                                                                     \
        sci_Printf(msg_if_false);                                              \
    }

// 完全格式化版本 - 两个分支都支持格式化输出
#define CONTINUE_IF_MSG_FULL(condition, msg_if_true, msg_if_false, ...)        \
    if (condition) {                                                           \
        sci_Printf(msg_if_true, ##__VA_ARGS__);                                \
        continue;                                                              \
    }                                                                          \
    else {                                                                     \
        sci_Printf(msg_if_false, ##__VA_ARGS__);                               \
    }
/* USER CODE END */

int main(void)
{
    /* USER CODE BEGIN (3) */
    uint32 i;
    uint32 count = 0;
    uint32 idcode = 0;

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
        // 每隔一定次数执行一次 JTAG 测试
        if (count++ % 1000) {
            // 延时
            for (i = 0; i < 500000; i++)
                ;
            if (count % 100 == 0) {
                // 周期性输出心跳信息
                sci_Printf("运行中 count = %d\r\n", count / 100);
            }
            // if(count > 1000000000) {
            if (count > 10000) {
                count = 0;
            }
            continue;
        }
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

        idcode = JTAG_Read_DR_Pause(ICEPICK_IDCODE_LENGTH);
        sci_Printf("  [3] 读取 ICEPick IDCODE: 0x%08X\r\n", idcode);

        // 解析 IDCODE
        CONTINUE_IF_MSG(parse_IDCODE(idcode, "ICEPick"),
                        "      请检查:\r\n"
                        "      1. 目标芯片是否供电\r\n"
                        "      2. JTAG 引脚连接是否正确\r\n"
                        "      3. 两块芯片是否共地\r\n\r\n",
                        "");

        sci_Printf("  [4] 开始通过 ICEPick 路由到 DAP...\r\n");

        // 发送 CONNECT 指令
        JTAG_ICEPick_Connect();
        sci_Printf("      - CONNECT 指令已发送\r\n");

        // 读取 DCON 寄存器验证
        uint8 dcon = JTAG_ICEPick_Read_DCON();
        sci_Printf("      - DCON 寄存器值：0x%02X\r\n", dcon);

        // 检查连接状态
        CONTINUE_IF_MSG_FULL(
            dcon != ICEPICK_DCON_CONNECTKEY,
            "  [✗] ICEPick 未连接 (CONNECTKEY = 0x%X)\r\n\r\n",
            "  [✓] ICEPick 已连接！(CONNECTKEY = 0x%X)\r\n\r\n", dcon);

        // 发送 ROUTE 指令
        JTAG_From_Pause_To_Select_DR_Scan();
        JTAG_Write_IR_Pause(ICEPICK_IR_ROUTE, ICEPICK_IR_LENGTH);

        JTAG_From_Pause_To_Select_DR_Scan();
        JTAG_Write_DR_Pause(ICEPICK_DCON_SDTAP0_VALUE,
                            ICEPICK_DCON_LENGTH + ICEPICK_SDTAP0_LENGTH);

        JTAG_From_Pause_To_Select_DR_Scan();

        JTAG_Write_IR_Pause(ICEPICK_IR_BYPASS, ICEPICK_IR_LENGTH);

        // 先从 Pause-IR 回到 Idle，然后在 Idle 状态等待 10 个时钟
        JTAG_From_Pause_To_Idle();

        // 在 Run-Test/Idle 状态下等待 10 个时钟周期
        // 让硬件有时间将 SDTAP0 加入扫描链
        JTAG_Set_TMS(0);  // 确保停留在 Idle 状态
        for (i = 0; i < 10; i++) {
            JTAG_Clock_Pulse();
        }

        sci_Printf("  [5] 读取 DAP IDCODE...\r\n");

        // 从 Idle 状态进入 Select-DR-Scan
        JTAG_From_Idle_To_Select_DR_Scan();

        // 读取 DR（默认 IDCODE 指令）并提取实际的 IDCODE（前 32 位）
        // 注意：是 32+1=33 位（32 位 IDCODE + 1 位 ICEPick BYPASS）
        uint32 dap_idcode =
            JTAG_Read_DR_Pause(ICEPICK_IDCODE_LENGTH + 1) & 0xFFFFFFFF;

        // 通过 DAP 读取 IDCODE
        JTAG_From_Pause_To_Select_DR_Scan();
        JTAG_Write_IR_Pause(DAP_IR_IDCODE | ICEPICK_IR_BYPASS << DAP_IR_LENGTH,
                            DAP_IR_LENGTH + ICEPICK_IR_LENGTH);

        JTAG_From_Pause_To_Select_DR_Scan();
        dap_idcode = JTAG_Read_DR_Pause(ICEPICK_IDCODE_LENGTH + 1) & 0xFFFFFFFF;
        sci_Printf("      - DP IDCODE: 0x%08X\r\n", dap_idcode);
        CONTINUE_IF(parse_IDCODE(dap_idcode, "DAP"));

        sci_Printf("  [6] 测试 DPACC 访问...\r\n");

        // 设置 DPACC IR（包含 ICEPick BYPASS）
        JTAG_From_Pause_To_Select_DR_Scan();
        JTAG_Write_IR_Pause(DAP_IR_DPACC | ICEPICK_IR_BYPASS << DAP_IR_LENGTH,
                            DAP_IR_LENGTH + ICEPICK_IR_LENGTH);
        
        // 读取 CTRL/STAT 寄存器（函数内部会处理状态转换）
        uint32 ctrl_stat = 0;
        uint32 ack = JTAG_DPACC_Read(DP_ADDR_CTRL_STAT, &ctrl_stat);
        sci_Printf("      CTRL/STAT 寄存器状态:\r\n");
        sci_Printf("      - ACK: 0x%X\r\n", ack);
        sci_Printf("      - CTRL/STAT: 0x%08X\r\n", ctrl_stat);

        // 上电 DAP（IR 已经设置为 DPACC，无需再次设置）
        ack = JTAG_DAP_PowerUp();
        sci_Printf("      - ACK: 0x%X\r\n", ack);
        sci_Printf("      - CTRL/STAT: 0x%08X\r\n", ctrl_stat);
        // 检查电源状态
        uint32 sys_pwr_ack = (ctrl_stat >> 31) & 0x01U;
        uint32 dbg_pwr_ack = (ctrl_stat >> 29) & 0x01U;
        sci_Printf("      - 系统电源确认:%s\r\n", sys_pwr_ack ? "是" : "否");
        sci_Printf("      - 调试电源确认:%s\r\n", dbg_pwr_ack ? "是" : "否");
        sci_Printf("  [✓] DPACC 读取成功！\r\n\r\n");

    }
    /* USER CODE END */

    return 0;
}

/* USER CODE BEGIN (4) */
// 制造商 ID 查找表结构
typedef struct
{
    uint32 mfg_id;
    const char* name;
} ManufacturerEntry;

// 制造商 ID 映射表（基于 JEDEC JEP106 标准）
const ManufacturerEntry ManufacturerTable[] = {
    {0x017, "Texas Instruments"}, {0x23B, "ARM CoreSight"}, {0x025, "NXP"}
    // 可以继续添加更多制造商...
};

// 获取制造商表项数量
#define MANUFACTURER_TABLE_SIZE                                                \
    (sizeof(ManufacturerTable) / sizeof(ManufacturerEntry))

// 根据 mfg ID 查找制造商名称
const char* get_manufacturer_name(uint32 mfg_id)
{
    uint32 i;
    for (i = 0; i < MANUFACTURER_TABLE_SIZE; i++) {
        if (ManufacturerTable[i].mfg_id == mfg_id) {
            return ManufacturerTable[i].name;
        }
    }
    return "Unknown";  // 未知制造商
}

uint32 parse_IDCODE(uint32 idcode, const char* name)
{
    sci_Printf("      - IDCODE: 0x%08X\r\n", idcode);
    if (idcode != 0 && idcode != 0xFFFFFFFF) {
        uint32 version = (idcode >> 28) & 0x0F;
        uint32 part = (idcode >> 12) & 0xFFFF;
        uint32 mfg = (idcode >> 1) & 0x7FF;

        sci_Printf("      - 版本号：0x%X\r\n", version);
        sci_Printf("      - 器件型号：0x%04X\r\n", part);
        sci_Printf("      - 制造商 ID: 0x%03X\r\n", mfg);
        sci_Printf("      - 制造商：%s\r\n", get_manufacturer_name(mfg));
        sci_Printf("  [✓] %s IDCODE 读取成功！\r\n\r\n", name);
        return SUCCESS;
    }
    else {
        sci_Printf("  [✗] %s IDCODE 读取失败\r\n\r\n", name);
        return FAILED;
    }
}
/* USER CODE END */

/* USER CODE BEGIN (5) */
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
