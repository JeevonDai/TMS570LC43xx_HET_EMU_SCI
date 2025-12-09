/** @file jtag_gpio.h
*   @brief JTAG GPIO Emulation Header File
*   @date 2025-11-19
*
*   此文件定义了使用N2HET1引脚模拟JTAG信号的接口
*/

#ifndef __JTAG_GPIO_H__
#define __JTAG_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "HL_reg_het.h"
#include "HL_sys_common.h"

/* USER CODE BEGIN (0) */
/* USER CODE END */

/** 
 * @brief JTAG引脚定义
 * 物理引脚映射到N2HET1引脚
 */
#define JTAG_TRST_PIN 22U /* B3  N2HET1_22 -> BMU_M_S_TRST_OUT_R */
#define JTAG_TCK_PIN 23U  /* J4  N2HET1_23 -> BMU_M_S_TCK_OUT_R  */
#define JTAG_TDI_PIN 24U  /* P1  N2HET1_24 -> BMU_M_S_TDI_OUT_R  */
#define JTAG_TDO_PIN 27U  /* A9  N2HET1_27 -> BMU_M_S_TDO_IN_R   */
#define JTAG_TMS_PIN 29U  /* A3  N2HET1_29 -> BMU_M_S_TMS_OUT_R  */

/* 引脚掩码 */
#define JTAG_TRST_MASK (1U << JTAG_TRST_PIN)
#define JTAG_TCK_MASK (1U << JTAG_TCK_PIN)
#define JTAG_TDI_MASK (1U << JTAG_TDI_PIN)
#define JTAG_TDO_MASK (1U << JTAG_TDO_PIN)
#define JTAG_TMS_MASK (1U << JTAG_TMS_PIN)

/* 输出引脚组合掩码（除TDO外都是输出） */
#define JTAG_OUTPUT_PINS                                                       \
    (JTAG_TRST_MASK | JTAG_TCK_MASK | JTAG_TDI_MASK | JTAG_TMS_MASK)
/* 输入引脚组合掩码 */
#define JTAG_INPUT_PINS (JTAG_TDO_MASK)

/**
 * @brief JTAG状态机状态定义
 */
typedef enum
{
    JTAG_STATE_TEST_LOGIC_RESET = 0,
    JTAG_STATE_RUN_TEST_IDLE,
    JTAG_STATE_SELECT_DR_SCAN,
    JTAG_STATE_CAPTURE_DR,
    JTAG_STATE_SHIFT_DR,
    JTAG_STATE_EXIT1_DR,
    JTAG_STATE_PAUSE_DR,
    JTAG_STATE_EXIT2_DR,
    JTAG_STATE_UPDATE_DR,
    JTAG_STATE_SELECT_IR_SCAN,
    JTAG_STATE_CAPTURE_IR,
    JTAG_STATE_SHIFT_IR,
    JTAG_STATE_EXIT1_IR,
    JTAG_STATE_PAUSE_IR,
    JTAG_STATE_EXIT2_IR,
    JTAG_STATE_UPDATE_IR
} jtag_state_t;

/* USER CODE BEGIN (1) */
/**
 * @brief ICEPick IR/DR 指令定义
 */
#define ICEPICK_IDCODE_LENGTH 32 /* ICEPICK IDCODE 寄存器长度 32bit */
#define ICEPICK_IR_LENGTH 6      /* ICEPICK IR 指令长度 */
#define ICEPICK_IR_ROUTE 0x02U   /* ROUTE 指令 */
#define ICEPICK_IR_IDCODE 0x04U  /* CONNECT 指令 */
#define ICEPICK_IR_CONNECT 0x07U /* CONNECT 指令 */
#define ICEPICK_IR_BYPASS 0x3FU  /* BYPASS 指令 */

/* Debug Connect Register (DCON)  */
#define ICEPICK_DCON_WRITEENABLE 0x80U /* DCON 写使能 bit[7] */
#define ICEPICK_DCON_CONNECTKEY 0x09U  /* DCON 连接密钥 bit[3:0] */
#define ICEPICK_DCON_LENGTH 8          /* ICEPICK DCON 寄存器长度 8bit */
#define ICEPICK_SDTAP0_LENGTH 24       /* ICEPICK DCON 寄存器长度 24bit */

/*
高 8 位 DCON 配置 0xA0
bit[31] = 1 写使能
bit[30:24] = 0100000b 选择 SDTAP0
低 24 位 SDTAP0 配置 0x002108
bit[13] = 1 Enable Debug Logic
bit[8] = 1 Select SDTAP0
Bit[3] = 1 Force Active Power and Clock
*/
#define ICEPICK_DCON_SDTAP0_VALUE 0xA0002108U /* DCON 连接 SDTAP0 指令 */

/**
 * @brief ARM CoreSight DAP 指令定义
 */
#define DAP_IR_LENGTH 4    /* DAP IR 指令长度 */
#define DAP_IR_ABORT 0x8U  /* ABORT 指令 */
#define DAP_IR_DPACC 0xAU  /* DPACC 指令 - 访问 DP 寄存器 */
#define DAP_IR_APACC 0xBU  /* APACC 指令 - 访问 AP 寄存器 */
#define DAP_IR_IDCODE 0xEU /* IDCODE 指令 */
#define DAP_IR_BYPASS 0xFU /* BYPASS 指令 */

/**
 * @brief DP 寄存器地址定义（用于 DPACC）
 */
#define DP_ADDR_RESERVE 0x0U   /* ABORT 寄存器（只写）*/
#define DP_ADDR_CTRL_STAT 0x4U /* CTRL/STAT 寄存器 */
#define DP_ADDR_SELECT 0x8U    /* SELECT 寄存器 */
#define DP_ADDR_RDBUFF 0xCU    /* RDBUFF 寄存器（只读）*/

/**
 * @brief CTRL/STAT 寄存器位定义
 */
#define DP_CTRL_CSYSPWRUPREQ (1U << 30) /* 系统电源请求 */
#define DP_CTRL_CDBGPWRUPREQ (1U << 28) /* 调试电源请求 */
#define DP_CTRL_CSYSPWRUPACK (1U << 31) /* 系统电源确认 */
#define DP_CTRL_CDBGPWRUPACK (1U << 29) /* 调试电源确认 */

/**
 * @brief DPACC 请求类型定义
 */
#define DPACC_READ (1U << 0)  /* 读操作 */
#define DPACC_WRITE (0U << 0) /* 写操作 */

/**
 * @brief DPACC 响应 ACK 值定义
 */
#define DPACC_ACK_OK 0x2U    /* OK/FAULT */
#define DPACC_ACK_WAIT 0x1U  /* WAIT */
#define DPACC_ACK_FAULT 0x0U /* FAULT */

/**
 * @brief APB-AP 寄存器地址定义（相对于 AP bank）
 */
#define AP_REG_CSW 0x0U /* Control/Status Word 寄存器 */
#define AP_REG_TAR 0x4U /* Transfer Address 寄存器 */
#define AP_REG_DRW 0xCU /* Data Read/Write 寄存器 */

/**
 * @brief ARM Cortex-R 调试寄存器地址定义
 * ROM Table 基地址 = 0x80000000
 * ARM 调试组件偏移 = 0x1000
 * ARM Core Debug 基地址 = 0x80001000
 */
#define DBG_BASE_ADDR 0x80001000U  /* ARM 调试组件基地址 */
#define DBG_DTRRX_ADDR 0x80001080U /* 调试数据传输接收寄存器（外部写入）*/
#define DBG_ITR_ADDR 0x80001084U   /* 指令传输寄存器 */
#define DBG_DSCR_ADDR 0x80001088U  /* 调试状态和控制寄存器 */
#define DBG_DTRTX_ADDR 0x8000108CU /* 调试数据传输发送寄存器（外部读取）*/
#define DBG_DRCR_ADDR 0x80001090U  /* 调试运行控制寄存器 */

/**
 * @brief DSCR (Debug Status and Control Register) 位定义
 */
#define DSCR_HALTED (1U << 0)         /* CPU 已停止 */
#define DSCR_RESTARTED (1U << 1)      /* CPU 已重启 */
#define DSCR_SDABORT_L (1U << 6)      /* 同步数据中止（粘滞）*/
#define DSCR_ITR_EN (1U << 13)        /* ITR 使能（允许执行指令）*/
#define DSCR_HALT_DBG_MODE (1U << 14) /* Halt 调试模式使能 */
#define DSCR_INSTRCOML_L (1U << 24)   /* 指令执行完成（粘滞）*/
#define DSCR_DTR_TX_FULL (1U << 29)   /* DTRTX 满 */
#define DSCR_DTR_RX_FULL (1U << 30)   /* DTRRX 满 */

/**
 * @brief DRCR (Debug Run Control Register) 位定义
 */
#define DRCR_HALT (1U << 0)           /* 请求 CPU 停止 */
#define DRCR_RESTART (1U << 1)        /* 请求 CPU 重启 */
#define DRCR_CLR_EXCEPTIONS (1U << 2) /* 清除粘滞异常 */

/**
 * @brief ARM 指令编码（用于 ITR 执行）
 * MRC/MCR p14, 0, Rd, c0, c5, 0 - 读写 DTRRX/DTRTX
 */
/* MRC p14, 0, R0, c0, c5, 0 - 从 DTRRX 读取到 R0 */
#define ARM_INSTR_MRC_DTRRX_R0 0xEE100E15U
/* MCR p14, 0, R0, c0, c5, 0 - 从 R0 写入到 DTRTX */
#define ARM_INSTR_MCR_R0_DTRTX 0xEE000E15U
/* MOV PC, R0 - 将 R0 值加载到 PC */
#define ARM_INSTR_MOV_PC_R0 0xE1A0F000U
/* BX R0 - 跳转到 R0 指向的地址 */
#define ARM_INSTR_BX_R0 0xE12FFF10U
/* MOV R0, PC - 将 PC 值写入 R0 */
#define ARM_INSTR_MOV_R0_PC 0xE1A0000FU

/**
 * @brief APACC 请求类型定义
 */
#define APACC_READ (1U << 0)  /* 读操作 */
#define APACC_WRITE (0U << 0) /* 写操作 */
/* USER CODE END */

/**
 * @brief 初始化JTAG GPIO引脚
 * @param[in] void
 * @return void
 * 
 * 此函数配置N2HET1引脚作为GPIO使用：
 * - TRST, TCK, TDI, TMS 配置为输出
 * - TDO 配置为输入
 */
void JTAG_GPIO_Init(void);

/**
 * @brief 设置TRST引脚电平
 * @param[in] value - 0: 低电平, 非0: 高电平
 * @return void
 */
void JTAG_Set_TRST(uint32 value);

/**
 * @brief 设置TCK引脚电平
 * @param[in] value - 0: 低电平, 非0: 高电平
 * @return void
 */
void JTAG_Set_TCK(uint32 value);

/**
 * @brief 设置TDI引脚电平
 * @param[in] value - 0: 低电平, 非0: 高电平
 * @return void
 */
void JTAG_Set_TDI(uint32 value);

/**
 * @brief 设置TMS引脚电平
 * @param[in] value - 0: 低电平, 非0: 高电平
 * @return void
 */
void JTAG_Set_TMS(uint32 value);

/**
 * @brief 读取TDO引脚电平
 * @param[in] void
 * @return uint32 - 0: 低电平, 1: 高电平
 */
uint32 JTAG_Get_TDO(void);

/**
 * @brief 产生一个TCK时钟脉冲
 * @param[in] void
 * @return void
 */
void JTAG_Clock_Pulse(void);

/**
 * @brief JTAG复位序列
 * @param[in] void
 * @return void
 * 
 * 通过TMS保持高电平并产生至少5个TCK时钟，
 * 使JTAG状态机进入Test-Logic-Reset状态
 */
void JTAG_Reset(void);

/**
 * @brief JTAG进入Run-Test/Idle状态
 * @param[in] void
 * @return void
 */
void JTAG_Goto_Idle(void);

/**
 * @brief JTAG移位数据
 * @param[in] tms - TMS信号值
 * @param[in] tdi - TDI信号值
 * @return uint32 - TDO返回的数据位
 * 
 * 在TCK下降沿设置TMS和TDI，
 * 在TCK上升沿采样TDO
 */
uint32 JTAG_Shift_Bit(uint32 tms, uint32 tdi);

uint32 JTAG_Write_DR_Pause(uint32 dr_value, uint32 dr_len);

uint32 JTAG_Read_DR_Pause(uint32 dr_len);

uint32 JTAG_Write_IR_Pause(uint32 ir_value, uint32 ir_len);

uint32 JTAG_Read_IR_Pause(uint32 ir_len);

void JTAG_From_Idle_To_Select_DR_Scan();

void JTAG_From_Pause_To_Select_DR_Scan();

void JTAG_From_Pause_To_Idle();

/* USER CODE BEGIN (2) */
/**
 * @brief ICEPick 连接函数
 */
void JTAG_ICEPick_Connect(void);
uint32 JTAG_ICEPick_Read_DCON(void);

/**
 * @brief 写 DPACC 寄存器
 * @param addr DP 寄存器地址（0x0, 0x4, 0x8, 0xC）
 * @param data 要写入的 32 位数据
 * @return ACK 响应值
 */
uint32 JTAG_DPACC_Write(uint8 addr, uint32 data);

/**
 * @brief 读 DPACC 寄存器
 * @param addr DP 寄存器地址（0x0, 0x4, 0x8, 0xC）
 * @param data 指向接收数据的指针
 * @return ACK 响应值
 */
uint32 JTAG_DPACC_Read(uint8 addr, uint32* data);

/**
 * @brief 写 APACC 寄存器
 * @param addr AP 寄存器地址（0x0, 0x4, 0x8, 0xC）
 * @param data 指向要写入的 32 位数据指针
 * @return ACK 响应值
 */
uint32 JTAG_APACC_Write(uint8 addr, uint32* data);

/**
 * @brief 读 APACC 寄存器
 * @param addr AP 寄存器地址（0x0, 0x4, 0x8, 0xC）
 * @param data 指向接收数据的指针
 * @return ACK 响应值
 */
uint32 JTAG_APACC_Read(uint8 addr, uint32* data);

/**
 * @brief 初始化 DAP 调试电源
 * @return 1 表示成功，0 表示失败
 */
uint32 JTAG_DAP_PowerUp(void);

/**
 * @brief 挂起目标 CPU
 * @return 1 表示成功，0 表示失败
 */
uint32 JTAG_DAP_Halt_CPU(void);

/**
 * @brief 恢复目标 CPU
 * @return 1 表示成功，0 表示失败
 */
uint32 JTAG_DAP_Resume_CPU(void);

/**
 * @brief 通过 APB-AP 写入指定地址的数据
 * @param tar_addr 目标地址（调试寄存器地址）
 * @param write_data 要写入的数据指针
 * @return ACK 响应值
 */
uint32 JTAG_APB_AP_Write(uint32 tar_addr, uint32* write_data);

/**
 * @brief 通过 APB-AP 读取指定地址的数据
 * @param tar_addr 目标地址（调试寄存器地址）
 * @param read_data 输出参数，存储读取到的数据
 * @return ACK 响应值
 */
uint32 JTAG_APB_AP_Read(uint32 tar_addr, uint32* read_data);

/**
 * @brief 设置 CPU PC 指针并启动执行
 * @param entry_addr 程序入口地址
 * @return 0 表示成功，1 表示失败
 */
uint32 JTAG_Set_PC_And_Run(uint32 entry_addr);

/**
 * @brief 通过 APB-AP 执行 CPU 指令，将数据写入指定内存地址
 * @param mem_addr 目标内存地址（将存储到 R1）
 * @param data 要写入的数据（将存储到 R0）
 * @return 0 表示成功，非0 表示失败
 * 
 * 操作流程：
 * 1. 激活 APB-AP（写 SELECT 寄存器选择 APB-AP）
 * 2. 将 data 写入 DTRRX，再执行 MRC 指令将其移到 R0
 * 3. 将 mem_addr 写入 DTRRX，再执行 MRC 指令将其移到 R1
 * 4. 执行 STR R0, [R1] 指令，将 R0 的值写入 R1 指向的内存
 */
uint32 JTAG_APB_AP_Write_Memory(uint32 mem_addr, uint32 data);

/* USER CODE END */

#ifdef __cplusplus
}
#endif /*extern "C" */

#endif /* __JTAG_GPIO_H__ */
