/** @file jtag_gpio.c
*   @brief JTAG GPIO Emulation Implementation File
*   @date 2025-11-19
*
*   此文件实现了使用 N2HET1 引脚模拟 JTAG 信号的功能
*/

/* USER CODE BEGIN (0) */
/* USER CODE END */

#include "jtag_gpio.h"
#include "HL_hal_stdtypes.h"

/* USER CODE BEGIN (1) */
/* USER CODE END */

/* HET1 端口指针 - 用于 GPIO 操作 */
#define JTAG_PORT hetPORT1

/**
 * @brief 初始化 JTAG GPIO 引脚
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
    JTAG_Set_TRST(1);  /* TRST 默认高电平（非激活） */
    JTAG_Set_TCK(0);   /* TCK 默认低电平 */
    JTAG_Set_TDI(0);   /* TDI 默认低电平 */
    JTAG_Set_TMS(1);   /* TMS 默认高电平 */
    
    /* USER CODE BEGIN (2) */
    /* USER CODE END */
}

/**
 * @brief 设置 TRST 引脚电平
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
 * @brief 设置 TCK 引脚电平
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
 * @brief 设置 TDI 引脚电平
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
 * @brief 设置 TMS 引脚电平
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
 * @brief 读取 TDO 引脚电平
 */
uint32 JTAG_Get_TDO(void)
{
    return (hetREG1->DIN & JTAG_TDO_MASK) ? 1U : 0U;
}

/**
 * @brief 产生一个 TCK 时钟脉冲
 */
void JTAG_Clock_Pulse(void)
{
    /* 短暂延时以确保信号稳定 */
    volatile uint32 delay;
    
    /* TCK 低电平 */
    JTAG_Set_TCK(0);
    for (delay = 0; delay < 10; delay++);  /* 延时 */
    
    /* TCK 高电平 */
    JTAG_Set_TCK(1);
    for (delay = 0; delay < 10; delay++);  /* 延时 */
    
    /* TCK 低电平 */
    JTAG_Set_TCK(0);
    for (delay = 0; delay < 10; delay++);  /* 延时 */
}

/**
 * @brief JTAG 复位序列
 */
void JTAG_Reset(void)
{
    uint32 i;
    
    /* TMS 保持高电平，产生至少 5 个 TCK 时钟 */
    JTAG_Set_TMS(1);
    JTAG_Set_TDI(0);
    
    for (i = 0; i < 8; i++)  /* 产生 8 个时钟以确保复位 */
    {
        JTAG_Clock_Pulse();
    }
}

/**
 * @brief JTAG 进入 Run-Test/Idle 状态
 */
void JTAG_Goto_Idle(void)
{
    /* 从 Test-Logic-Reset 到 Run-Test/Idle：TMS=0，一个时钟 */
    JTAG_Set_TMS(0);
    JTAG_Set_TDI(0);
    JTAG_Clock_Pulse();
}

/**
 * @brief JTAG 移位一位数据
 */
uint32 JTAG_Shift_Bit(uint32 tms, uint32 tdi)
{
    uint32 tdo;
    volatile uint32 delay;
    
    /* TCK 低电平，设置 TMS 和 TDI */
    JTAG_Set_TCK(0);
    JTAG_Set_TMS(tms);
    JTAG_Set_TDI(tdi);
    for (delay = 0; delay < 10; delay++);
    
    /* TCK 上升沿，采样 TDO */
    JTAG_Set_TCK(1);
    for (delay = 0; delay < 10; delay++);
    tdo = JTAG_Get_TDO();
    
    /* TCK 下降沿 */
    JTAG_Set_TCK(0);
    for (delay = 0; delay < 10; delay++);
    
    return tdo;
}

void JTAG_From_Idle_To_Select_DR_Scan() {
    /* 从 Idle -> Select_DR_Scan */
    JTAG_Shift_Bit(1, 0);
}

void JTAG_From_Pause_To_Idle() {
    /* 从 Pause -> Exit2 */
    JTAG_Shift_Bit(1, 0);

    /* 从 Exit2-> Update */
    JTAG_Shift_Bit(1, 0);

    /* 从 Update-> Idle */
    JTAG_Shift_Bit(0, 0);
}

void JTAG_From_Pause_To_Select_DR_Scan() {
    /* 从 Pause -> Exit2 */
    JTAG_Shift_Bit(1, 0);

    /* 从 Exit2-> Update */
    JTAG_Shift_Bit(1, 0);

    /* 从 Update-> Select-DR-Scan */
    JTAG_Shift_Bit(1, 0);
}

uint32 JTAG_Write_DR_Pause(uint32 dr_value, uint32 dr_len) {
    uint32 i;
    uint32 ret = 0;
    uint32 tdo = 0;

    /* 前提：当前已经在 Select-DR-Scan 状态 */

    /* Select-DR-Scan -> Capture-DR */
    JTAG_Shift_Bit(0, 0);
    
    /* Capture-DR -> Shift-DR */
    JTAG_Shift_Bit(0, 0);
    
    /* 在 Shift-DR 状态移位数据 */
    for (i = 0; i < dr_len - 1; i++)
    {
        uint32 bit = (dr_value >> i) & 0x01U;
        tdo = JTAG_Shift_Bit(0, bit);  /* TMS=0 保持在 Shift-DR */
        ret |= (tdo << i);
    }

    /* 最后一位，TMS=1 退出 Shift-DR 到 Exit1-DR */
    uint32 last_bit = (dr_value >> (dr_len - 1)) & 0x01U;
    tdo = JTAG_Shift_Bit(1, last_bit);
    ret |= (tdo << (dr_len - 1));
    
    /* Exit1-DR -> Pause-DR */
    JTAG_Shift_Bit(0, 0);

    return ret;
}

uint32 JTAG_Read_DR_Pause(uint32 dr_len) {
    uint32 i;
    uint32 tdo = 0;
    uint32 ret = 0;

    /* 前提：当前已经在 Select-DR-Scan 状态 */
    
    /* Select-DR-Scan -> Capture-DR */
    JTAG_Shift_Bit(0, 0);
    
    /* Capture-DR -> Shift-DR 进入移位状态 */
    JTAG_Shift_Bit(0, 0);
    
    /* 在 Shift-DR 状态继续移位并读取剩余数据 */
    for (i = 0; i < dr_len - 1; i++)
    {
        tdo = JTAG_Shift_Bit(0, 0);  /* TMS=0 保持在 Shift-DR */
        ret |= (tdo << i);
    }
    
    /* 最后一位，TMS=1 退出 Shift-DR 到 Exit1-DR */
    tdo = JTAG_Shift_Bit(1, 0);
    ret |= (tdo << (dr_len - 1));
    
    /* Exit1-DR -> Pause-DR */
    JTAG_Shift_Bit(0, 0);
    
    return ret;
}

uint32 JTAG_Write_IR_Pause(uint32 ir_value, uint32 ir_len) {
    uint32 i;
    uint32 ret = 0;
    uint32 tdo = 0;

    /* Select-DR-Scan -> Select-IR-Scan */
    JTAG_Shift_Bit(1, 0);
    
    /* Select-IR-Scan -> Capture-IR */
    JTAG_Shift_Bit(0, 0);
    
    /* Capture-IR -> Shift-IR */
    JTAG_Shift_Bit(0, 0);
    
    /* 在 Shift-IR 状态移位数据 */
    for (i = 0; i < ir_len - 1; i++)
    {
        uint32 bit = (ir_value >> i) & 0x01U;
        tdo = JTAG_Shift_Bit(0, bit);  /* TMS=0 保持在 Shift-IR */
        ret |= (tdo << i);
    }
    
    /* 最后一位，TMS=1 退出 Shift-IR 到 Exit1-IR */
    uint32 last_bit = (ir_value >> (ir_len - 1)) & 0x01U;
    tdo = JTAG_Shift_Bit(1, last_bit);
    ret |= (tdo << (ir_len - 1));
    
    /* Exit1-IR -> Pause-IR */
    JTAG_Shift_Bit(0, 0);

    return ret;
}

/* USER CODE BEGIN (3) */
/**
 * @brief 连接到 ICEPick TAP
 * @return 1 表示成功，0 表示失败
 */
void JTAG_ICEPick_Connect(void) {
    // 1. 从 Pause-DR 回到 Idle (如果当前在 Pause 状态)
    JTAG_From_Pause_To_Idle();
    
    // 2. Idle -> Select-DR-Scan
    JTAG_Shift_Bit(1, 0);
    
    // 3. 写入 CONNECT 指令 (000111b = 0x07) 到 IR
    JTAG_Write_IR_Pause(ICEPICK_IR_CONNECT, ICEPICK_IR_LENGTH);
    
    // 4. 回到 Select-DR-Scan 准备写 DR
    JTAG_From_Pause_To_Select_DR_Scan();
    
    // 5. 写入 Debug Connect Register (DCON)
    //    bit[7] = 1: 写使能 (WRITEENABLE)
    //    bit[3:0] = 1001b: 连接密钥 (CONNECTKEY)
    //    完整值：0x89 = 10001001b
    JTAG_Write_DR_Pause(ICEPICK_DCON_CONNECTKEY | ICEPICK_DCON_WRITEENABLE, ICEPICK_DCON_LENGTH);
}

/**
 * @brief 读取 ICEPick 连接状态
 * @return DCON 寄存器的值
 */
uint32 JTAG_ICEPick_Read_DCON(void) {
    uint32 dcon_value;
    
    // 6. 准备读 DR
    JTAG_From_Pause_To_Select_DR_Scan();
    
    // 7. 写入读命令 (bit[7]=0 表示读操作)
    // 读写操作都是在 Update-DR 阶段，WRITEENABLE=0 不会改变 DCON 值
    JTAG_Write_DR_Pause(0x00, ICEPICK_DCON_LENGTH);
    
    // 8. 再次进入 Shift-DR 读取实际值
    JTAG_From_Pause_To_Select_DR_Scan();
    dcon_value = JTAG_Read_DR_Pause(ICEPICK_DCON_LENGTH);
    
    return dcon_value;
}

/**
 * @brief 写 DPACC 寄存器
 * @param addr DP 寄存器地址（0x0, 0x4, 0x8, 0xC）
 * @param data 要写入的 32 位数据
 * @return ACK 响应值
 * 
 * 注意：调用此函数前需要先设置 IR 为 DPACC + ICEPick BYPASS
 * 
 * DPACC 数据格式（35 位）+ ICEPick BYPASS（1 位）= 36 位：
 * [35]    - ICEPick BYPASS 位（填充 0）
 * [34:3]  - 写入的 32 位数据
 * [2]     - RnW 位（0=写，1=读）
 * [1:0]   - A[3:2] 地址位
 */
uint32 JTAG_DPACC_Write(uint8 addr, uint32 data)
{
    uint32 i;
    uint8 request = 0;
    uint32 ack = 0;
    uint32 tdo = 0;

    // 构造 DPACC 写请求（35 位）
    // [0] = 0 (写操作 RnW=0)
    // [1] = addr[2] (A[2])
    // [2] = addr[3] (A[3])
    // [34:3] = data[31:0]
    request = (addr & 0xC) >> 1;  // A[3:2] -> bit[2:1]
    // RnW = 0 (写操作) 已经是 0

    // 进入 DR 扫描（假设已经在 Pause-DR 或 Pause-IR 状态）
    JTAG_From_Pause_To_Select_DR_Scan();

    // 进入 Shift-DR
    JTAG_Shift_Bit(0, 0);  // Select-DR -> Capture-DR
    JTAG_Shift_Bit(0, 0);  // Capture-DR -> Shift-DR

    // 移位前 3 位（地址和 RnW）
    for (i = 0; i < 3; i++) {
        uint32 bit = (request >> i) & 0x01U;
        JTAG_Shift_Bit(0, bit);
    }

    // 移位 32 位数据
    for (i = 0; i < 32; i++) {
        uint32 bit = (data >> i) & 0x01U;
        JTAG_Shift_Bit(0, bit);
    }

    // 移位 1 位 ICEPick BYPASS（填充 0，最后一位退出）
    tdo = JTAG_Shift_Bit(1, 0);

    // 到达 Exit1-DR，进入 Pause-DR
    JTAG_Shift_Bit(0, 0);

    // 读取 ACK（需要再次扫描 DR）
    JTAG_From_Pause_To_Select_DR_Scan();

    // 进入 Shift-DR 读取 ACK
    JTAG_Shift_Bit(0, 0);  // Select-DR -> Capture-DR
    JTAG_Shift_Bit(0, 0);  // Capture-DR -> Shift-DR

    // 读取前 3 位获取 ACK
    tdo = JTAG_Shift_Bit(0, 0);
    ack = tdo & 0x01U;
    tdo = JTAG_Shift_Bit(0, 0);
    ack |= (tdo & 0x01U) << 1;
    tdo = JTAG_Shift_Bit(0, 0);
    ack |= (tdo & 0x01U) << 2;

    // 完成扫描（32 位数据 + 1 位 bypass）
    for (i = 3; i < 35; i++) {
        JTAG_Shift_Bit(0, 0);
    }
    
    // 最后一位 ICEPick bypass，退出
    JTAG_Shift_Bit(1, 0);

    JTAG_Shift_Bit(0, 0);  // 进入 Pause-DR

    return ack;
}

/**
 * @brief 读 DPACC 寄存器
 * @param addr DP 寄存器地址（0x0, 0x4, 0x8, 0xC）
 * @param data 指向接收数据的指针
 * @return ACK 响应值
 * 
 * 注意：调用此函数前需要先设置 IR 为 DPACC + ICEPick BYPASS
 * DPACC 读操作需要两次 DR 扫描：
 * 1. 第一次发送读请求（36 位：3 位请求 + 32 位占位 + 1 位 bypass）
 * 2. 第二次获取读取的数据（36 位：3 位 ACK + 32 位数据 + 1 位 bypass）
 */
uint32 JTAG_DPACC_Read(uint8 addr, uint32* data)
{
    uint32 i;
    uint8 request = 0;
    uint32 ack = 0;
    uint32 read_data = 0;
    uint32 tdo = 0;

    // 构造 DPACC 读请求（35 位）+ ICEPick BYPASS（1 位）
    // [0] = 1 (读操作 RnW=1)
    // [1] = addr[2] (A[2])
    // [2] = addr[3] (A[3])
    // [34:3] = 数据占位（填充 0）
    // [35] = ICEPick BYPASS（填充 0）
    // 例如，CTRL/STAT 的 addr 是 0x4 对应的 request 是 01b & 1b = 011b
    request = ((addr & 0xC) >> 1) | 1U;  // A[3:2] -> bit[2:1], RnW=1 -> bit[0]

    // 进入 DR 扫描发送读请求（假设已经在 Pause-DR 或 Pause-IR 状态）
    JTAG_From_Pause_To_Select_DR_Scan();

    // 进入 Shift-DR
    JTAG_Shift_Bit(0, 0);  // Select-DR -> Capture-DR
    JTAG_Shift_Bit(0, 0);  // Capture-DR -> Shift-DR

    // 移位 35 位（3 位请求 + 32 位数据占位）
    for (i = 0; i < 35; i++) {
        uint32 bit = (request >> i) & 0x01U;
        if (i >= 3) bit = 0;  // 数据位填充 0
        JTAG_Shift_Bit(0, bit);
    }

    // 移位 1 位 ICEPick BYPASS（填充 0，最后一位退出）
    JTAG_Shift_Bit(1, 0);

    JTAG_Shift_Bit(0, 0);  // 进入 Pause-DR

    // 再次扫描 DR 获取读取的数据
    JTAG_From_Pause_To_Select_DR_Scan();

    // 进入 Shift-DR
    JTAG_Shift_Bit(0, 0);  // Select-DR -> Capture-DR
    JTAG_Shift_Bit(0, 0);  // Capture-DR -> Shift-DR

    // 读取前 3 位获取 ACK
    tdo = JTAG_Shift_Bit(0, 0);
    ack = tdo & 0x01U;
    tdo = JTAG_Shift_Bit(0, 0);
    ack |= (tdo & 0x01U) << 1;
    tdo = JTAG_Shift_Bit(0, 0);
    ack |= (tdo & 0x01U) << 2;

    // 读取 32 位数据
    for (i = 0; i < 32; i++) {
        tdo = JTAG_Shift_Bit(0, 0);
        read_data |= (tdo & 0x01U) << i;
    }

    // 读取 1 位 ICEPick BYPASS，最后一位退出
    JTAG_Shift_Bit(1, 0);

    JTAG_Shift_Bit(0, 0);  // 进入 Pause-DR

    if (data != 0) {
        *data = read_data;
    }

    return ack;
}

/**
 * @brief 初始化 DAP 调试电源
 * @return 1 表示成功，0 表示失败
 */
uint32 JTAG_DAP_PowerUp(void)
{
    uint32 ctrl_stat = 0;
    uint32 ack = 0;
    uint32 timeout = 1000;
    uint32 i;

    // 1. 上电请求：设置 CSYSPWRUPREQ 和 CDBGPWRUPREQ
    ctrl_stat = DP_CTRL_CSYSPWRUPREQ | DP_CTRL_CDBGPWRUPREQ;
    ack = JTAG_DPACC_Write(DP_ADDR_CTRL_STAT, ctrl_stat);

    if (ack != DPACC_ACK_OK) {
        return 0;  // 写入失败
    }

    // 2. 等待上电确认
    for (i = 0; i < timeout; i++) {
        ack = JTAG_DPACC_Read(DP_ADDR_CTRL_STAT, &ctrl_stat);

        if (ack != DPACC_ACK_OK) {
            continue;
        }

        // 检查上电确认位
        if ((ctrl_stat & DP_CTRL_CSYSPWRUPACK) && (ctrl_stat & DP_CTRL_CDBGPWRUPACK)) {
            return 1;  // 上电成功
        }

        // 简单延时
        volatile uint32 delay;
        for (delay = 0; delay < 1000; delay++)
            ;
    }

    return 0;  // 超时
}

/**
 * @brief 挂起目标 CPU（通过 APB-AP 访问 DRCR 寄存器）
 * @return 0 表示成功，1 表示失败
 * 
 * 注意：TMS570LC4357 使用 APB-AP 访问 DRCR (Debug Run Control Register)
 * 这与标准 ARM 实现不同，不使用 MEM-AP
 */
uint32 JTAG_DAP_Halt_CPU(void)
{
    // TODO: 需要通过 APACC 访问 APB-AP
    // 1. 选择 APB-AP (通过 SELECT 寄存器)
    // 2. 配置 APB-AP 的 CSW 寄存器（设置访问大小、地址增量等）
    // 3. 写 TAR 寄存器（设置目标地址为 DRCR 寄存器地址）
    // 4. 通过 DRW 寄存器写入 DRCR，发送 HALT 请求，使 CPU 进入调试模式

    // 这里只是一个占位符，显示流程
    return 0;
}

/**
 * @brief 恢复目标 CPU（通过 APB-AP 访问 DRCR 寄存器）
 * @return 0 表示成功，1 表示失败
 * 
 * 注意：写入 DRCR 的 RESTART 请求位，使 CPU 退出调试模式
 */
uint32 JTAG_DAP_Resume_CPU(void)
{
    // TODO: 需要通过 APACC 访问 APB-AP
    // 1. 选择 APB-AP (通过 SELECT 寄存器)
    // 2. 配置 APB-AP 的 CSW 寄存器（设置访问大小、地址增量等）
    // 3. 写 TAR 寄存器（设置目标地址为 DRCR 寄存器地址）
    // 4. 通过 DRW 寄存器写入 DRCR，发送 RESTART 请求，使 CPU 退出调试模式
    
    return 0;
}

/* USER CODE END */
