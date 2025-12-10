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

/* SRAM 加载器相关定义 */
#define SRAM_BIN_FLASH_ADDR 0x00200000U /* bin 文件在 FLASH BANK1 的起始地址 */
#define SRAM_BIN_SIZE 38000U            /* bin 文件大小（字节）*/
#define TARGET_SRAM_BASE 0x08000000U    /* 目标芯片 SRAM 起始地址 */
#define SRAM_BIN_WORDS ((SRAM_BIN_SIZE + 3) / 4) /* 按 32 位字计算 */

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

#define APACC_IR_REWRITE 1


uint32 JTAG_Read_DSCR(uint32* dscr_value)
{
    return JTAG_APB_AP_Read(DBG_DSCR_ADDR, dscr_value);
}

uint32 JTAG_Read_DRCR(uint32* drcr_value)
{
    return JTAG_APB_AP_Read(DBG_DRCR_ADDR, drcr_value);
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
    uint32 tmp = 0;
    uint32 halt_value = 0x1;

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
        sci_Printf("      - DCON 寄存器值: 0x%02X\r\n", dcon);

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
        // 注意: 是 32+1=33 位（32 位 IDCODE + 1 位 ICEPick BYPASS）
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

        JTAG_From_Pause_To_Select_DR_Scan();
        JTAG_Write_IR_Pause(DAP_IR_DPACC | ICEPICK_IR_BYPASS << DAP_IR_LENGTH,
                            DAP_IR_LENGTH + ICEPICK_IR_LENGTH);

        sci_Printf("  [7] 激活 APB-AP...\r\n");

        // 向 JTAG-DP.SELECT 写 0x01000000
        // 选择 APB-AP 并选择它的 bank0
        uint32 select_value = 0x01000000;
        ack = JTAG_DPACC_Write(DP_ADDR_SELECT, select_value);
        sci_Printf("      - 写 DP.SELECT: 0x%08X (ACK=0x%X)\r\n", select_value,
                   ack);
        CONTINUE_IF_MSG_FULL(ack != DPACC_ACK_OK,
                             "  [✗] 写 DP.SELECT 失败\r\n\r\n",
                             "  [✓] APB-AP 已激活！\r\n\r\n");

        sci_Printf("  [8] 设置 APB-AP 访问地址...\r\n");
        // 向 APB-AP.TAR 写 0x80001080
        // CPU 调试组件中 DTRRX 的地址
        // ROM table 基地址 = 0x80000000
        // ARM 内核调试组件 offset = 0x1000
        // ARM core base = 0x80000000 + 0x1000 = 0x80001000
        // DTRRX offset = 0x80
        // 最终地址: 0x80001080

        // 切换到 APACC 指令（包含 ICEPick BYPASS）
        JTAG_From_Pause_To_Select_DR_Scan();
        JTAG_Write_IR_Pause(DAP_IR_APACC | ICEPICK_IR_BYPASS << DAP_IR_LENGTH,
                            DAP_IR_LENGTH + ICEPICK_IR_LENGTH);

        uint32 dtrrx_addr = 0x80001080;
        ack = JTAG_APACC_Write(AP_REG_TAR, &dtrrx_addr);
        sci_Printf("      - 写 APB-AP.TAR: 0x%08X (ACK=0x%X)\r\n", dtrrx_addr,
                   ack);

        CONTINUE_IF_MSG_FULL(ack != DPACC_ACK_OK,
                             "  [✗] 写 APB-AP.TAR 失败\r\n\r\n",
                             "  [✓] APB-AP.TAR 已设置为 DTRRX 地址！\r\n\r\n");

        JTAG_From_Pause_To_Select_DR_Scan();
        JTAG_Write_IR_Pause(DAP_IR_APACC | ICEPICK_IR_BYPASS << DAP_IR_LENGTH,
                            DAP_IR_LENGTH + ICEPICK_IR_LENGTH);
        uint32 drcr_addr = 0x80001090;
        // 注意: JTAG_APACC_Write 会自动读取 DP.RDBUFF 来获取真实的 ACK
        // 这是因为 APACC 写操作的 ACK 是流水线化的（pipelined）
        ack = JTAG_APACC_Write(AP_REG_TAR, &drcr_addr);
        sci_Printf("      - 写 APB-AP.TAR: 0x%08X (ACK=0x%X)\r\n", drcr_addr,
                   ack);
        CONTINUE_IF_MSG_FULL(ack != DPACC_ACK_OK,
                             "  [✗] 写 APB-AP.TAR 失败\r\n\r\n",
                             "  [✓] APB-AP.TAR 已设置为 DRCR 地址！\r\n\r\n");

        sci_Printf("  [9] 等待 CPU 进入调试状态...\r\n");

        JTAG_From_Pause_To_Select_DR_Scan();
        JTAG_Write_IR_Pause(DAP_IR_APACC | ICEPICK_IR_BYPASS << DAP_IR_LENGTH,
                            DAP_IR_LENGTH + ICEPICK_IR_LENGTH);

        ack = JTAG_APACC_Write(AP_REG_DRW, &halt_value);
        CONTINUE_IF_MSG_FULL(ack != DPACC_ACK_OK, "  [✗] 写 DRCR 失败\r\n\r\n",
                             "  [✓] AP_REG_DRW 已配置 HALT 请求！\r\n\r\n");
        tmp++;
        if (tmp % 2) {
            halt_value = 0x2;
        }
        else {
            halt_value = 0x1;
            JTAG_From_Pause_To_Select_DR_Scan();
            sci_Printf("HALT 退出，CPU 继续运行，跳过后续步骤\r\n");
            continue;
        }

        /* ========== 通过 APB-AP 执行 CPU 指令写入内存 ========== */
        /* 注意: TMS570LC4357 是锁步核（Lockstep Core），无法通过 APB-AP 
         * 访问 DTRRX/ITR 等调试寄存器来执行 CPU 指令写入 SRAM。
         * 锁步核架构限制了这种调试功能，以下代码已禁用。
         */
#if 0
        sci_Printf(" [APB-AP] 通过 APB-AP 执行 CPU 指令写入内存...\r\n");
        uint32 result = JTAG_APB_AP_Write_Memory(0x08000000, 0x11111111);
        if (result == 0) {
            sci_Printf("  [✓] APB-AP 写入内存成功！\r\n\r\n");
        } else {
            sci_Printf("  [✗] APB-AP 写入内存失败，错误码: %d\r\n\r\n", result);
        }
#endif
#if 1
        uint32 cnt = 0;
        while (cnt > 10000) {
            cnt++;
        }

        uint32 dscr = 0;
        ack = JTAG_Read_DSCR(&dscr);
        sci_Printf("      - 读 DSCR: 0x%08X (ACK=0x%X)\r\n", dscr, ack);

        uint32 dscr_new = dscr | DSCR_ITR_EN;
        ack = JTAG_APB_AP_Write(DBG_DSCR_ADDR, &dscr_new);
        sci_Printf("      - 写 DSCR: 0x%08X (ACK=0x%X)\r\n", dscr_new, ack);

        ack = JTAG_Read_DSCR(&dscr);
        sci_Printf("      - 读 DSCR: 0x%08X (ACK=0x%X)\r\n", dscr, ack);

        uint32 drcr = 0xfffffff;
        ack = JTAG_Read_DRCR(&drcr);
        sci_Printf("      - 读 DRCR: 0x%08X (ACK=0x%X)\r\n", drcr, ack);

        // (2) DTRRX 地址写入 TAR
        dtrrx_addr = 0x80001080;
        ack = JTAG_APACC_Write(AP_REG_TAR, &dtrrx_addr);
        sci_Printf("      - 写 APB-AP.TAR: 0x%08X (ACK=0x%X)\r\n", dtrrx_addr,
                   ack);
        // (3) 写入 0x11111111 到 DRW
        uint32 data = 0x11111111;
        ack = JTAG_APACC_Write(AP_REG_DRW, &data);
        sci_Printf("      - 写 APB-AP.DRW: 0x%08X (ACK=0x%X)\r\n", data, ack);

        // (4.1) ITR 地址写入 TAR 
        uint32 itr_addr = 0x80001084;
        ack = JTAG_APACC_Write(AP_REG_TAR, &itr_addr);
        sci_Printf("      - 写 APB-AP.TAR: 0x%08X (ACK=0x%X)\r\n", itr_addr, ack);

        // (4.2) ITR 写入 MRC p14,0,r0,c0,c5,0 指令
        uint32 instruction = 0xEE100E15;
        ack = JTAG_APACC_Write(AP_REG_DRW, &instruction);
        sci_Printf("      - 写 APB-AP.DRW: 0x%08X (ACK=0x%X)\r\n", instruction,
                   ack);

        // (5) DTRRX 地址写入 TAR
        dtrrx_addr = 0x80001080;
        ack = JTAG_APACC_Write(AP_REG_TAR, &dtrrx_addr);
        sci_Printf("      - 写 APB-AP.TAR: 0x%08X (ACK=0x%X)\r\n", dtrrx_addr,
                   ack);

        // (6) 写入 0x08000000 到 DRW
        uint32 write_addr = 0x08000000;
        ack = JTAG_APACC_Write(AP_REG_DRW, &write_addr);
        sci_Printf("      - 写 APB-AP.DRW: 0x%08X (ACK=0x%X)\r\n", write_addr,
                   ack);

        // (7.1) ITR 地址写入 TAR 
        JTAG_Write_IR_Pause(DAP_IR_APACC | ICEPICK_IR_BYPASS << DAP_IR_LENGTH,
                            DAP_IR_LENGTH + ICEPICK_IR_LENGTH);
        itr_addr = 0x80001084;
        ack = JTAG_APACC_Write(AP_REG_TAR, &itr_addr);
        sci_Printf("      - 写 APB-AP.TAR: 0x%08X (ACK=0x%X)\r\n", itr_addr,
                   ack);

        // (7.2) ITR 写入 MRC p14,0,r1,c0,c5,0 指令
        instruction = 0xEE101E15;
        ack = JTAG_APACC_Write(AP_REG_DRW, &instruction);
        sci_Printf("      - 写 APB-AP.DRW: 0x%08X (ACK=0x%X)\r\n", instruction,
                   ack);

        // (8) DTRRX 地址写入 TAR
        itr_addr = 0x80001084;
        ack = JTAG_APACC_Write(AP_REG_TAR, &itr_addr);
        sci_Printf("      - 写 APB-AP.TAR: 0x%08X (ACK=0x%X)\r\n", itr_addr,
                   ack);
        
        // (9) 写入 STR R0,[R1] 指令
        instruction = 0xE5810000;
        ack = JTAG_APACC_Write(AP_REG_DRW, &instruction);
        sci_Printf("      - 写 APB-AP.DRW: 0x%08X (ACK=0x%X)\r\n", instruction,
                   ack);
#endif
#if 0
        // ===============================================
        // 向 JTAG-DP.SELECT 写 0x00000000
        // 选择 AHB-AP 并选择它的 bank0
        sci_Printf(" [10] 激活 AHB-AP...\r\n");
        select_value = 0x00000000;
        ack = JTAG_DPACC_Write(DP_ADDR_SELECT, select_value);
        sci_Printf("      - 写 DP.SELECT: 0x%08X (ACK=0x%X)\r\n", select_value,
                   ack);
        CONTINUE_IF_MSG_FULL(ack != DPACC_ACK_OK,
                             "  [✗] 写 DP.SELECT 失败\r\n\r\n",
                             "  [✓] AHB-AP 已激活！\r\n\r\n");
        sci_Printf(" [11] 进入 AHB-AP 访存模式...\r\n");
        JTAG_From_Pause_To_Select_DR_Scan();
        JTAG_Write_IR_Pause(DAP_IR_APACC | ICEPICK_IR_BYPASS << DAP_IR_LENGTH,
                            DAP_IR_LENGTH + ICEPICK_IR_LENGTH);
        // 配置 AHB-AP CSW 寄存器
        // CSW = 0x43000002
        // [31:24] = 0x43 - 保留位和 Debug SW Access
        // [23:12] = 0x000 - 保留
        // [11:8]  = 0x0 - Mode（基本传输模式）
        // [7]     = 0 - TrInProg（传输未进行）
        // [6]     = 0 - DeviceEn（设备特定）
        // [5:4]   = 00b - AddrInc OFF（地址不自增，手动控制 TAR）
        // [3]     = 0 - 保留
        // [2:0]   = 010b - Size（32-bit 访问）
        uint32 ahb_ap_csw = 0x43000002;
        ack = JTAG_APACC_Write(AP_REG_CSW, &ahb_ap_csw);
        sci_Printf("      - 写 AHB-AP.CSW: 0x%08X (ACK=0x%X)\r\n", ahb_ap_csw,
                   ack);
        CONTINUE_IF_MSG_FULL(ack != DPACC_ACK_OK,
                             "  [✗] 写 AHB-AP.CSW 失败\r\n\r\n",
                             "  [✓] AHB-AP.CSW 访存模式设置成功！\r\n\r\n");

        sci_Printf(" [12] 设置 AHB-AP 访问地址...\r\n");
        JTAG_From_Pause_To_Select_DR_Scan();
        JTAG_Write_IR_Pause(DAP_IR_APACC | ICEPICK_IR_BYPASS << DAP_IR_LENGTH,
                            DAP_IR_LENGTH + ICEPICK_IR_LENGTH);

        uint32 sdram_addr = 0x08000000;
        ack = JTAG_APACC_Write(AP_REG_TAR, &sdram_addr);
        sci_Printf("      - 写 AHB-AP.TAR: 0x%08X (ACK=0x%X)\r\n", sdram_addr,
                   ack);
        CONTINUE_IF_MSG_FULL(ack != DPACC_ACK_OK,
                             "  [✗] 写 AHB-AP.TAR 失败\r\n\r\n",
                             "  [✓] AHB-AP.TAR 已设置为 AHB-AP 地址！\r\n\r\n");

                             sci_Printf(" [13] 从 FLASH 读取 bin 并写入备芯片 SRAM...\r\n");
        sci_Printf("      - FLASH 源地址: 0x%08X\r\n", SRAM_BIN_FLASH_ADDR);
        sci_Printf("      - SRAM 目标地址: 0x%08X\r\n", TARGET_SRAM_BASE);
        sci_Printf("      - bin 大小: %d 字节 (%d 字)\r\n", SRAM_BIN_SIZE,
                   SRAM_BIN_WORDS);

        /* 获取 FLASH 中 bin 数据的指针 */
        uint32* flash_ptr = (uint32*)SRAM_BIN_FLASH_ADDR;
        uint32 words_written = 0;
        uint32 write_errors = 0;

        /* 
         * 循环写入每个 32 位字
         * 注意: CSW 已关闭自动递增，需要每次手动设置 TAR
         */
        // for (i = 0; i < SRAM_BIN_WORDS; i++) {
        for (i = 0; i < 1; i++) {
            /* 读取 FLASH 中的数据 */
            uint32 flash_data = flash_ptr[i];
            uint32 target_addr = TARGET_SRAM_BASE + i * 4;
            
            if(i % 0x400 == 0) {
                sci_Printf("      - FLASH 数据: 0x%08X\r\n", flash_ptr[i]);
            }

            /* 先设置 TAR 地址 */
            JTAG_From_Pause_To_Select_DR_Scan();
            JTAG_Write_IR_Pause(
                DAP_IR_APACC | ICEPICK_IR_BYPASS << DAP_IR_LENGTH,
                DAP_IR_LENGTH + ICEPICK_IR_LENGTH);
            ack = JTAG_APACC_Write(AP_REG_TAR, &target_addr);
            if (ack != DPACC_ACK_OK) {
                write_errors++;
                if (write_errors <= 5) {
                    sci_Printf("  [✗] 设置TAR失败 @0x%08X (ACK=0x%X)\r\n",
                               target_addr, ack);
                }
                continue;
            }

            /* 再写入数据到 DRW */
            JTAG_From_Pause_To_Select_DR_Scan();
            JTAG_Write_IR_Pause(
                DAP_IR_APACC | ICEPICK_IR_BYPASS << DAP_IR_LENGTH,
                DAP_IR_LENGTH + ICEPICK_IR_LENGTH);
            ack = JTAG_APACC_Write(AP_REG_DRW, &flash_data);

            if (ack != DPACC_ACK_OK) {
                write_errors++;
                if (write_errors <= 5) {
                    sci_Printf("  [✗] 写入失败 @0x%08X (ACK=0x%X)\r\n",
                               TARGET_SRAM_BASE + i * 4, ack);
                }
            }
            else {
                words_written++;
            }

            /* 每写入 1024 字（4KB）输出一次进度 */
            if ((i + 1) % 1024 == 0) {
                sci_Printf("      进度: %d/%d 字 (%d%%)\r\n", i + 1,
                           SRAM_BIN_WORDS, (i + 1) * 100 / SRAM_BIN_WORDS);
            }
        }

        sci_Printf("      - 写入完成: %d/%d 字, 错误: %d\r\n", words_written,
                   SRAM_BIN_WORDS, write_errors);

        CONTINUE_IF_MSG_FULL(
            write_errors > 0 || words_written != SRAM_BIN_WORDS,
            "  [✗] SRAM 写入失败\r\n\r\n", "  [✓] SRAM 写入成功！\r\n\r\n");
#endif

        sci_Printf(" [14] 读取复位向量（程序入口地址）...\r\n");
        /* 
         * SRAM 中 bin 文件布局（ARM 向量表）：
         * 0x08000000: 复位向量（Reset Handler 地址）
         * 0x08000004: 未定义指令向量
         * ...
         * 实际上，Cortex-R 的向量表第一个字是 Reset Handler 的跳转指令
         * 但对于 TMS570，通常第一条是 LDR PC, [PC, #24] 等指令
         * 我们直接跳转到 0x08000000 让它从向量表开始执行
         */
        // uint32 entry_addr = TARGET_SRAM_BASE;  /* 0x08000000 */
        // ENTRY POINT SYMBOL: "_c_int00"  address: 080087dc
        // uint32 entry_addr = 0x080087dc;  /* 中断向量表第一条指令是跳转到 _c_int00 的分支指令 */
        uint32 entry_addr =
            0x00000000; /* 中断向量表第一条指令是跳转到 _c_int00 的分支指令 */
        sci_Printf("      - 程序入口地址: 0x%08X\r\n", entry_addr);

        sci_Printf(" [15] 设置 PC 并启动 SRAM 程序...\r\n");

        // 此前处于 pause 状态
        uint32 result = JTAG_Set_PC_And_Run(entry_addr);

        if (result == 0) {
            sci_Printf("  [✓] CPU 已跳转到 SRAM 并开始执行！\r\n");
            sci_Printf("====================================\r\n");
            sci_Printf("  SRAM 加载完成，程序正在运行...\r\n");
            sci_Printf("====================================\r\n\r\n");
        }
        else {
            sci_Printf("  [✗] 启动失败，错误码: %d\r\n", result);
            sci_Printf("      错误含义:\r\n");
            sci_Printf("        1 = 再次激活 APB-AP 失败\r\n");
            sci_Printf("        2 = 写入 DTRRX 失败\r\n");
            sci_Printf("        3 = 写入 ITR (MRC) 失败\r\n");
            sci_Printf("        4 = 写入 ITR (BX) 失败\r\n");
            sci_Printf("        5 = 发送 RESTART 失败\r\n");
        }
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

        sci_Printf("      - 版本号: 0x%X\r\n", version);
        sci_Printf("      - 器件型号: 0x%04X\r\n", part);
        sci_Printf("      - 制造商 ID: 0x%03X\r\n", mfg);
        sci_Printf("      - 制造商: %s\r\n", get_manufacturer_name(mfg));
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
