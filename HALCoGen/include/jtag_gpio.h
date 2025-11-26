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
#define JTAG_TRST_PIN   22U   /* B3  N2HET1_22 -> BMU_M_S_TRST_OUT_R */
#define JTAG_TCK_PIN    23U   /* J4  N2HET1_23 -> BMU_M_S_TCK_OUT_R  */
#define JTAG_TDI_PIN    24U   /* P1  N2HET1_24 -> BMU_M_S_TDI_OUT_R  */
#define JTAG_TDO_PIN    27U   /* A9  N2HET1_27 -> BMU_M_S_TDO_IN_R   */
#define JTAG_TMS_PIN    29U   /* A3  N2HET1_29 -> BMU_M_S_TMS_OUT_R  */

/* 引脚掩码 */
#define JTAG_TRST_MASK  (1U << JTAG_TRST_PIN)
#define JTAG_TCK_MASK   (1U << JTAG_TCK_PIN)
#define JTAG_TDI_MASK   (1U << JTAG_TDI_PIN)
#define JTAG_TDO_MASK   (1U << JTAG_TDO_PIN)
#define JTAG_TMS_MASK   (1U << JTAG_TMS_PIN)

/* 输出引脚组合掩码（除TDO外都是输出） */
#define JTAG_OUTPUT_PINS (JTAG_TRST_MASK | JTAG_TCK_MASK | JTAG_TDI_MASK | JTAG_TMS_MASK)
/* 输入引脚组合掩码 */
#define JTAG_INPUT_PINS  (JTAG_TDO_MASK)

/**
 * @brief JTAG状态机状态定义
 */
typedef enum {
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
/* USER CODE END */

#ifdef __cplusplus
}
#endif /*extern "C" */

#endif /* __JTAG_GPIO_H__ */

