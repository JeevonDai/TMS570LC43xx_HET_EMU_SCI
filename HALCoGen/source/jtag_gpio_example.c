/** @file jtag_gpio_example.c
*   @brief JTAG GPIO使用示例
*   @date 2025-11-19
*
*   此文件提供了使用JTAG GPIO模拟功能的示例代码
*/

/* USER CODE BEGIN (0) */
/* USER CODE END */

#include "jtag_gpio.h"
#include "HL_sci.h"
#include <stdio.h>

/* USER CODE BEGIN (1) */
/* USER CODE END */

/**
 * @brief JTAG GPIO初始化和测试示例
 * @param void
 * @return void
 * 
 * 此函数展示如何初始化和使用JTAG GPIO功能
 */
void JTAG_GPIO_Example(void)
{
    uint32 idcode;
    
    /* 1. 初始化JTAG GPIO */
    JTAG_GPIO_Init();
    
    /* 2. 复位JTAG */
    JTAG_Reset();
    
    /* 3. 进入Idle状态 */
    JTAG_Goto_Idle();
    
    /* 4. 读取IDCODE（通常是32位） */
    /* IDCODE指令通常在JTAG复位后自动加载到IR */
    idcode = JTAG_Read_DR(32);
    
    /* 打印IDCODE（如果需要调试输出） */
    /* printf("IDCODE: 0x%08X\n", idcode); */
}

/**
 * @brief JTAG写入示例
 * @param void
 * @return void
 * 
 * 展示如何通过JTAG写入指令和数据
 */
void JTAG_Write_Example(void)
{
    /* 示例：写入自定义IR指令 */
    uint32 ir_instruction = 0x05;  /* 示例指令值 */
    uint32 ir_length = 5;           /* IR长度取决于目标器件 */
    
    /* 写入IR */
    JTAG_Write_IR(ir_instruction, ir_length);
    
    /* 示例：写入DR数据 */
    uint32 dr_data = 0x12345678;
    uint32 dr_length = 32;
    
    /* 写入DR */
    JTAG_Write_DR(dr_data, dr_length);
    
    /* 返回到Idle状态 */
    JTAG_Goto_Idle();
}

/**
 * @brief JTAG读取示例
 * @param void
 * @return uint32 读取的数据
 * 
 * 展示如何通过JTAG读取数据
 */
uint32 JTAG_Read_Example(void)
{
    uint32 read_data;
    
    /* 示例：先写入要读取的IR指令 */
    uint32 ir_read_cmd = 0x03;  /* 示例读取指令 */
    uint32 ir_length = 5;
    
    JTAG_Write_IR(ir_read_cmd, ir_length);
    
    /* 读取DR数据 */
    read_data = JTAG_Read_DR(32);
    
    return read_data;
}

/**
 * @brief TMS序列示例
 * @param void
 * @return void
 * 
 * 展示如何通过特定的TMS序列控制JTAG状态机
 */
void JTAG_TMS_Sequence_Example(void)
{
    /* 示例：手动控制JTAG状态转换 */
    
    /* 从Idle到Select-DR */
    JTAG_Shift_Bit(1, 0);
    
    /* 从Select-DR到Capture-DR */
    JTAG_Shift_Bit(0, 0);
    
    /* 从Capture-DR到Shift-DR */
    JTAG_Shift_Bit(0, 0);
    
    /* 在Shift-DR状态移位8位数据 */
    uint32 tdo_data = 0;
    uint32 tdi_data = 0xAA;  /* 发送的测试数据 */
    JTAG_Shift_Data(0, tdi_data, 8, &tdo_data);
    
    /* 退出Shift-DR (TMS=1到Exit1-DR) */
    JTAG_Shift_Bit(1, 0);
    
    /* Exit1-DR到Update-DR */
    JTAG_Shift_Bit(1, 0);
    
    /* Update-DR到Idle */
    JTAG_Shift_Bit(0, 0);
}

/**
 * @brief 连续读取测试
 * @param count 读取次数
 * @return void
 * 
 * 展示如何进行连续的JTAG操作
 */
void JTAG_Continuous_Test(uint32 count)
{
    uint32 i;
    uint32 read_value;
    
    /* 初始化 */
    JTAG_GPIO_Init();
    JTAG_Reset();
    JTAG_Goto_Idle();
    
    /* 连续读取 */
    for (i = 0; i < count; i++)
    {
        /* 写入测试数据 */
        JTAG_Write_DR(i, 32);
        
        /* 读取数据 */
        read_value = JTAG_Read_DR(32);
        
        /* 这里可以添加数据验证逻辑 */
        if (read_value != i)
        {
            /* 数据不匹配处理 */
        }
    }
}

/**
 * @brief 低层GPIO控制示例
 * @param void
 * @return void
 * 
 * 展示如何直接控制JTAG引脚（用于特殊时序控制）
 */
void JTAG_Low_Level_Example(void)
{
    uint32 tdo_value;
    volatile uint32 i;
    
    /* 初始化GPIO */
    JTAG_GPIO_Init();
    
    /* 手动产生JTAG时序 */
    
    /* 1. 设置TRST低电平（激活复位） */
    JTAG_Set_TRST(0);
    for (i = 0; i < 1000; i++);  /* 延时 */
    
    /* 2. 释放TRST */
    JTAG_Set_TRST(1);
    for (i = 0; i < 1000; i++);  /* 延时 */
    
    /* 3. 手动产生时钟 */
    for (i = 0; i < 10; i++)
    {
        /* 设置TMS和TDI */
        JTAG_Set_TMS(1);
        JTAG_Set_TDI(0);
        
        /* 产生时钟脉冲 */
        JTAG_Clock_Pulse();
        
        /* 读取TDO */
        tdo_value = JTAG_Get_TDO();
    }
}

/**
 * @brief TMS570LC4357特定的JTAG调试示例
 * @param void
 * @return void
 * 
 * 展示如何通过JTAG控制另一个TMS570LC4357芯片
 */
void JTAG_TMS570_Debug_Example(void)
{
    uint32 device_id;
    
    /* 初始化JTAG */
    JTAG_GPIO_Init();
    
    /* 复位目标芯片的JTAG */
    JTAG_Reset();
    JTAG_Goto_Idle();
    
    /* 读取器件ID（TMS570的IDCODE） */
    device_id = JTAG_Read_DR(32);
    
    /* TMS570LC4357的IDCODE格式：
     * Bits 31-28: Version
     * Bits 27-12: Part Number
     * Bits 11-1:  Manufacturer ID
     * Bit 0:      固定为1
     */
    
    /* 示例：访问调试模块 */
    /* 具体的IR/DR定义需要参考TMS570LC4357的技术参考手册 */
    
    /* 1. 写入BYPASS指令（通常是全1） */
    JTAG_Write_IR(0xFFFFFFFF, 5);  /* TMS570的IR长度是5位 */
    
    /* 2. 如果需要访问调试功能，写入相应的IR指令 */
    /* 例如：访问调试寄存器 */
    /* JTAG_Write_IR(DEBUG_IR_CODE, 5); */
    /* JTAG_Write_DR(debug_data, 32); */
}

/**
 * @brief JTAG边界扫描示例
 * @param void
 * @return void
 * 
 * 展示如何使用JTAG边界扫描功能测试连接
 */
void JTAG_Boundary_Scan_Example(void)
{
    /* TMS570的边界扫描指令 */
    const uint32 EXTEST_IR = 0x00;    /* 外部测试 */
    const uint32 SAMPLE_IR = 0x03;    /* 采样/预加载 */
    const uint32 BYPASS_IR = 0x1F;    /* 旁路 */
    
    /* 初始化 */
    JTAG_GPIO_Init();
    JTAG_Reset();
    JTAG_Goto_Idle();
    
    /* 加载SAMPLE指令 */
    JTAG_Write_IR(SAMPLE_IR, 5);
    
    /* 读取边界扫描链的当前状态 */
    /* 边界扫描链的长度取决于器件，需要查阅BSDL文件 */
    /* uint32 boundary_data = JTAG_Read_DR(boundary_scan_length); */
    
    /* 返回到BYPASS */
    JTAG_Write_IR(BYPASS_IR, 5);
}

/* USER CODE BEGIN (2) */
/* USER CODE END */

