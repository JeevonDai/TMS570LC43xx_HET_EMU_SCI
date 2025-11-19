/** @file jtag_gpio.c
*   @brief JTAG GPIO Emulation Implementation File
*   @date 2025-11-19
*
*   此文件实现了使用N2HET1引脚模拟JTAG信号的功能
*/

/* USER CODE BEGIN (0) */
/* USER CODE END */

#include "jtag_gpio.h"

/* USER CODE BEGIN (1) */
/* USER CODE END */

/* HET1端口指针 - 用于GPIO操作 */
#define JTAG_PORT hetPORT1

/**
 * @brief 初始化JTAG GPIO引脚
 */
void JTAG_GPIO_Init(void)
{
    /* 配置数据方向寄存器 */
    /* 输出引脚：TRST, TCK, TDI, TMS */
    hetREG1->DIR |= JTAG_OUTPUT_PINS;
    
    /* 输入引脚：TDO */
    hetREG1->DIR &= ~JTAG_INPUT_PINS;
    
    /* 禁用开漏输出 */
    hetREG1->PDR &= ~(JTAG_OUTPUT_PINS | JTAG_INPUT_PINS);
    
    /* 配置上拉/下拉 */
    /* 使能上拉 */
    hetREG1->PULDIS &= ~(JTAG_OUTPUT_PINS | JTAG_INPUT_PINS);
    /* 选择上拉 */
    hetREG1->PSL |= (JTAG_OUTPUT_PINS | JTAG_INPUT_PINS);
    
    /* 初始化输出引脚状态 */
    JTAG_Set_TRST(1);  /* TRST默认高电平（非激活） */
    JTAG_Set_TCK(0);   /* TCK默认低电平 */
    JTAG_Set_TDI(0);   /* TDI默认低电平 */
    JTAG_Set_TMS(1);   /* TMS默认高电平 */
    
    /* USER CODE BEGIN (2) */
    /* USER CODE END */
}

/**
 * @brief 设置TRST引脚电平
 */
void JTAG_Set_TRST(uint32 value)
{
    if (value)
    {
        hetREG1->DSET = JTAG_TRST_MASK;  /* 设置为高电平 */
    }
    else
    {
        hetREG1->DCLR = JTAG_TRST_MASK;  /* 设置为低电平 */
    }
}

/**
 * @brief 设置TCK引脚电平
 */
void JTAG_Set_TCK(uint32 value)
{
    if (value)
    {
        hetREG1->DSET = JTAG_TCK_MASK;
    }
    else
    {
        hetREG1->DCLR = JTAG_TCK_MASK;
    }
}

/**
 * @brief 设置TDI引脚电平
 */
void JTAG_Set_TDI(uint32 value)
{
    if (value)
    {
        hetREG1->DSET = JTAG_TDI_MASK;
    }
    else
    {
        hetREG1->DCLR = JTAG_TDI_MASK;
    }
}

/**
 * @brief 设置TMS引脚电平
 */
void JTAG_Set_TMS(uint32 value)
{
    if (value)
    {
        hetREG1->DSET = JTAG_TMS_MASK;
    }
    else
    {
        hetREG1->DCLR = JTAG_TMS_MASK;
    }
}

/**
 * @brief 读取TDO引脚电平
 */
uint32 JTAG_Get_TDO(void)
{
    return (hetREG1->DIN & JTAG_TDO_MASK) ? 1U : 0U;
}

/**
 * @brief 产生一个TCK时钟脉冲
 */
void JTAG_Clock_Pulse(void)
{
    /* 短暂延时以确保信号稳定 */
    volatile uint32 delay;
    
    /* TCK低电平 */
    JTAG_Set_TCK(0);
    for (delay = 0; delay < 10; delay++);  /* 延时 */
    
    /* TCK高电平 */
    JTAG_Set_TCK(1);
    for (delay = 0; delay < 10; delay++);  /* 延时 */
    
    /* TCK低电平 */
    JTAG_Set_TCK(0);
    for (delay = 0; delay < 10; delay++);  /* 延时 */
}

/**
 * @brief JTAG复位序列
 */
void JTAG_Reset(void)
{
    uint32 i;
    
    /* TMS保持高电平，产生至少5个TCK时钟 */
    JTAG_Set_TMS(1);
    JTAG_Set_TDI(0);
    
    for (i = 0; i < 8; i++)  /* 产生8个时钟以确保复位 */
    {
        JTAG_Clock_Pulse();
    }
}

/**
 * @brief JTAG进入Run-Test/Idle状态
 */
void JTAG_Goto_Idle(void)
{
    /* 从Test-Logic-Reset到Run-Test/Idle：TMS=0，一个时钟 */
    JTAG_Set_TMS(0);
    JTAG_Set_TDI(0);
    JTAG_Clock_Pulse();
}

/**
 * @brief JTAG移位一位数据
 */
uint32 JTAG_Shift_Bit(uint32 tms, uint32 tdi)
{
    uint32 tdo;
    volatile uint32 delay;
    
    /* TCK低电平，设置TMS和TDI */
    JTAG_Set_TCK(0);
    JTAG_Set_TMS(tms);
    JTAG_Set_TDI(tdi);
    for (delay = 0; delay < 10; delay++);
    
    /* TCK上升沿，采样TDO */
    JTAG_Set_TCK(1);
    for (delay = 0; delay < 10; delay++);
    tdo = JTAG_Get_TDO();
    
    /* TCK下降沿 */
    JTAG_Set_TCK(0);
    for (delay = 0; delay < 10; delay++);
    
    return tdo;
}

/**
 * @brief JTAG移位多位数据
 */
void JTAG_Shift_Data(uint32 tms, uint32 tdi_data, uint32 bit_count, uint32* tdo_data)
{
    uint32 i;
    uint32 tdo_result = 0;
    
    for (i = 0; i < bit_count; i++)
    {
        /* 提取当前位 */
        uint32 tdi_bit = (tdi_data >> i) & 0x01U;
        
        /* 移位并采样TDO */
        uint32 tdo_bit = JTAG_Shift_Bit(tms, tdi_bit);
        
        /* 保存TDO数据 */
        if (tdo_data != NULL)
        {
            tdo_result |= (tdo_bit << i);
        }
    }
    
    if (tdo_data != NULL)
    {
        *tdo_data = tdo_result;
    }
}

/**
 * @brief JTAG写IR寄存器
 */
void JTAG_Write_IR(uint32 ir_value, uint32 ir_len)
{
    uint32 i;
    
    /* 从Run-Test/Idle -> Select-DR-Scan */
    JTAG_Shift_Bit(1, 0);
    
    /* Select-DR-Scan -> Select-IR-Scan */
    JTAG_Shift_Bit(1, 0);
    
    /* Select-IR-Scan -> Capture-IR */
    JTAG_Shift_Bit(0, 0);
    
    /* Capture-IR -> Shift-IR */
    JTAG_Shift_Bit(0, 0);
    
    /* 在Shift-IR状态移位数据 */
    for (i = 0; i < ir_len - 1; i++)
    {
        uint32 bit = (ir_value >> i) & 0x01U;
        JTAG_Shift_Bit(0, bit);  /* TMS=0保持在Shift-IR */
    }
    
    /* 最后一位，TMS=1退出Shift-IR到Exit1-IR */
    uint32 last_bit = (ir_value >> (ir_len - 1)) & 0x01U;
    JTAG_Shift_Bit(1, last_bit);
    
    /* Exit1-IR -> Update-IR */
    JTAG_Shift_Bit(1, 0);
    
    /* Update-IR -> Run-Test/Idle */
    JTAG_Shift_Bit(0, 0);
}

/**
 * @brief JTAG写DR寄存器
 */
void JTAG_Write_DR(uint32 dr_value, uint32 dr_len)
{
    uint32 i;
    
    /* 从Run-Test/Idle -> Select-DR-Scan */
    JTAG_Shift_Bit(1, 0);
    
    /* Select-DR-Scan -> Capture-DR */
    JTAG_Shift_Bit(0, 0);
    
    /* Capture-DR -> Shift-DR */
    JTAG_Shift_Bit(0, 0);
    
    /* 在Shift-DR状态移位数据 */
    for (i = 0; i < dr_len - 1; i++)
    {
        uint32 bit = (dr_value >> i) & 0x01U;
        JTAG_Shift_Bit(0, bit);  /* TMS=0保持在Shift-DR */
    }
    
    /* 最后一位，TMS=1退出Shift-DR到Exit1-DR */
    uint32 last_bit = (dr_value >> (dr_len - 1)) & 0x01U;
    JTAG_Shift_Bit(1, last_bit);
    
    /* Exit1-DR -> Update-DR */
    JTAG_Shift_Bit(1, 0);
    
    /* Update-DR -> Run-Test/Idle */
    JTAG_Shift_Bit(0, 0);
}

/**
 * @brief JTAG读DR寄存器
 */
uint32 JTAG_Read_DR(uint32 dr_len)
{
    uint32 i;
    uint32 dr_value = 0;
    
    /* 从Run-Test/Idle -> Select-DR-Scan */
    JTAG_Shift_Bit(1, 0);
    
    /* Select-DR-Scan -> Capture-DR */
    JTAG_Shift_Bit(0, 0);
    
    /* Capture-DR -> Shift-DR */
    JTAG_Shift_Bit(0, 0);
    
    /* 在Shift-DR状态移位并读取数据 */
    for (i = 0; i < dr_len - 1; i++)
    {
        uint32 tdo_bit = JTAG_Shift_Bit(0, 0);  /* TMS=0保持在Shift-DR */
        dr_value |= (tdo_bit << i);
    }
    
    /* 最后一位，TMS=1退出Shift-DR到Exit1-DR */
    uint32 tdo_bit = JTAG_Shift_Bit(1, 0);
    dr_value |= (tdo_bit << (dr_len - 1));
    
    /* Exit1-DR -> Update-DR */
    JTAG_Shift_Bit(1, 0);
    
    /* Update-DR -> Run-Test/Idle */
    JTAG_Shift_Bit(0, 0);
    
    return dr_value;
}

/* USER CODE BEGIN (3) */
/* USER CODE END */

