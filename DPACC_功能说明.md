# DPACC 访问功能说明

## 概述

本文档说明了通过 JTAG-DP 的 DPACC (Debug Port Access) 模块访问 ARM CoreSight DAP 寄存器的功能实现。

## 功能列表

### 1. 新增的宏定义 (`jtag_gpio.h`)

#### DAP 指令定义
```c
#define DAP_IR_ABORT    0x8U   /* ABORT 指令 */
#define DAP_IR_DPACC    0xAU   /* DPACC 指令 - 访问 DP 寄存器 */
#define DAP_IR_APACC    0xBU   /* APACC 指令 - 访问 AP 寄存器 */
#define DAP_IR_IDCODE   0xEU   /* IDCODE 指令 */
#define DAP_IR_BYPASS   0xFU   /* BYPASS 指令 */
```

#### DP 寄存器地址定义
```c
#define DP_ADDR_IDCODE      0x0U   /* IDCODE 寄存器 */
#define DP_ADDR_ABORT       0x0U   /* ABORT 寄存器（只写）*/
#define DP_ADDR_CTRL_STAT   0x4U   /* CTRL/STAT 寄存器 */
#define DP_ADDR_SELECT      0x8U   /* SELECT 寄存器 */
#define DP_ADDR_RDBUFF      0xCU   /* RDBUFF 寄存器（只读）*/
```

#### CTRL/STAT 寄存器位定义
```c
#define DP_CTRL_CSYSPWRUPREQ   (1U << 30)  /* 系统电源请求 */
#define DP_CTRL_CDBGPWRUPREQ   (1U << 28)  /* 调试电源请求 */
#define DP_CTRL_CSYSPWRUPACK   (1U << 31)  /* 系统电源确认 */
#define DP_CTRL_CDBGPWRUPACK   (1U << 29)  /* 调试电源确认 */
```

#### DPACC 响应 ACK 值
```c
#define DPACC_ACK_OK    0x2U   /* OK/FAULT */
#define DPACC_ACK_WAIT  0x1U   /* WAIT */
```

### 2. 新增的 API 函数

#### `JTAG_DPACC_Write()`
**功能**: 写 DPACC 寄存器

**参数**:
- `addr`: DP 寄存器地址（0x0, 0x4, 0x8, 0xC）
- `data`: 要写入的 32 位数据

**返回值**: ACK 响应值（0x2 表示成功）

**DPACC 数据格式** (35 位):
- [34:3]: 32 位数据
- [2]: RnW 位（0=写，1=读）
- [1:0]: A[3:2] 地址位

**示例**:
```c
uint32 ack = JTAG_DPACC_Write(DP_ADDR_CTRL_STAT, 0x50000000);
if (ack == DPACC_ACK_OK) {
    // 写入成功
}
```

#### `JTAG_DPACC_Read()`
**功能**: 读 DPACC 寄存器

**参数**:
- `addr`: DP 寄存器地址
- `data`: 指向接收数据的指针

**返回值**: ACK 响应值

**注意**: DPACC 读操作需要两次 DR 扫描：
1. 第一次发送读请求
2. 第二次获取读取的数据

**示例**:
```c
uint32 ctrl_stat = 0;
uint32 ack = JTAG_DPACC_Read(DP_ADDR_CTRL_STAT, &ctrl_stat);
if (ack == DPACC_ACK_OK) {
    // 读取成功，数据在 ctrl_stat 中
}
```

#### `JTAG_DAP_PowerUp()`
**功能**: 初始化 DAP 调试电源

**流程**:
1. 向 CTRL/STAT 寄存器写入上电请求位（CSYSPWRUPREQ | CDBGPWRUPREQ）
2. 轮询读取 CTRL/STAT 寄存器
3. 检查上电确认位（CSYSPWRUPACK | CDBGPWRUPACK）
4. 超时时间：1000 次

**返回值**: 1 表示成功，0 表示失败

**示例**:
```c
if (JTAG_DAP_PowerUp()) {
    sci_Printf("DAP 上电成功！\r\n");
}
```

#### `JTAG_DAP_Halt_CPU()` 和 `JTAG_DAP_Resume_CPU()`
**状态**: 函数框架已实现，但需要进一步完善

**说明**: 要真正挂起/恢复 CPU (ARM Cortex-R5F)，需要：
1. 通过 APACC 访问 APB-AP
2. 配置 APB-AP 的 TAR 寄存器指向 DRCR (Debug Run Control Register)
3. 通过 APB-AP 的 DRW 寄存器写入 DRCR
4. 写入控制位：
   - **挂起 CPU**: 写入 DRCR 的 HALT 请求位，使 CPU 进入调试模式
   - **恢复 CPU**: 写入 DRCR 的 RESTART 请求位，使 CPU 退出调试模式

## 使用示例

### 完整的 DAP 访问流程

```c
// 1. JTAG 复位并连接到 ICEPick
JTAG_Reset();
JTAG_Goto_Idle();
JTAG_ICEPick_Connect();

// 2. 路由到 DAP
// ... (使用 ICEPick ROUTE 指令) ...

// 3. 通过 DPACC 读取 DP IDCODE
uint32 dp_idcode = 0;
uint32 ack = JTAG_DPACC_Read(DP_ADDR_IDCODE, &dp_idcode);
sci_Printf("DP IDCODE: 0x%08X, ACK: 0x%X\r\n", dp_idcode, ack);

// 4. 读取 CTRL/STAT 寄存器状态
uint32 ctrl_stat = 0;
ack = JTAG_DPACC_Read(DP_ADDR_CTRL_STAT, &ctrl_stat);
sci_Printf("CTRL/STAT: 0x%08X\r\n", ctrl_stat);

// 5. 上电 DAP
if (JTAG_DAP_PowerUp()) {
    sci_Printf("DAP 已上电\r\n");
    
    // 6. 现在可以通过 APACC 访问 AP 寄存器
    // TODO: 实现 APACC 访问
}
```

## 主程序更新

主程序 `HL_sys_main.c` 中已添加以下测试代码：

1. **DPACC 读取 IDCODE**: 验证 DPACC 访问是否正常
2. **读取 CTRL/STAT**: 检查 DAP 电源状态
3. **自动上电**: 如果检测到 DAP 未上电，自动调用上电函数
4. **状态显示**: 显示系统电源和调试电源的确认状态

## CoreSight 调试架构层次

```
JTAG Interface
    ↓
ICEPick TAP (TI 特定)
    ↓ (通过 ROUTE 指令)
Debug Port (DP)
    ↓ (通过 DPACC)
    ├─ IDCODE 寄存器
    ├─ CTRL/STAT 寄存器
    ├─ SELECT 寄存器
    └─ RDBUFF 寄存器
    ↓ (通过 APACC)
Access Port (AP) - TMS570LC4357
    ↓
    ├─ APB-AP (APB 总线访问 - 用于调试寄存器)
    │   ├─ TAR (传输地址寄存器)
    │   ├─ DRW (数据读写寄存器)
    │   └─ CSW (控制/状态字)
    └─ AHB-AP (AHB 总线访问 - 用于内存和外设)
    ↓
目标处理器内部调试寄存器 (ARM Cortex-R5F)
    ├─ DRCR (调试运行控制寄存器)
    ├─ DCRSR (调试核心寄存器选择)
    ├─ DCRDR (调试核心寄存器数据)
    └─ 其他调试寄存器
```

## ARM Cortex-R5F 调试寄存器说明

### DRCR (Debug Run Control Register)
Debug Run Control Register (DRCR) 用于控制 ARM Cortex-R5F 处理器进入和退出调试模式。

**寄存器地址**: 通过 APB-AP (APB Access Port) 访问的调试寄存器空间

**关键控制位**:
- **HALT 请求位**: 写入此位使处理器进入调试模式（挂起 CPU）
- **RESTART 请求位**: 写入此位使处理器退出调试模式（恢复 CPU 运行）

**操作流程**:
1. 通过 APACC 选择 APB-AP
2. 配置 APB-AP 的 TAR 寄存器指向 DRCR 寄存器地址
3. 通过 DRW 寄存器写入相应的控制命令

**参考文档**: [TI Application Note SPNA230](https://www.ti.com/lit/an/spna230/spna230.pdf)

**注意事项**:
- TMS570LC4357 使用 ARM Cortex-R5F 双核架构
- 每个核心有独立的调试寄存器
- 需要通过正确的 APB-AP 通道访问对应核心的调试寄存器
- TI 芯片的调试架构与标准 ARM 实现有所不同，使用 APB-AP 而非 MEM-AP

## 下一步工作

要实现完整的 CPU 控制功能，还需要：

1. **实现 APACC 访问函数**
   - `JTAG_APACC_Write()`
   - `JTAG_APACC_Read()`

2. **实现 APB-AP 操作**
   - 选择 APB-AP（通过 SELECT 寄存器）
   - 配置 CSW 寄存器（设置访问大小、地址增量等）
   - 写 TAR 寄存器（设置目标地址）
   - 通过 DRW 寄存器访问调试寄存器

3. **实现 DRCR 访问**
   - 挂起 CPU：写 DRCR，发送 HALT 请求，使 CPU 进入调试模式
   - 恢复 CPU：写 DRCR，发送 RESTART 请求，使 CPU 退出调试模式
   - 读取 CPU 状态：通过调试状态寄存器检查 CPU 是否处于调试模式

4. **实现单步执行和断点功能**

## 参考文档

- ARM Debug Interface Architecture Specification (ADIv5)
- ARM CoreSight Components Technical Reference Manual
- TMS570LC43xx Technical Reference Manual
- ICEPick-C Debug Subsystem Specification
- [TI Application Note SPNA230: Debugging TMS570 Cortex-R Microcontrollers](https://www.ti.com/lit/an/spna230/spna230.pdf)

## 注意事项

1. **时序要求**: JTAG 时序必须严格遵守规范，当前代码中使用软件延时
2. **ACK 响应**: 每次 DPACC/APACC 操作后都要检查 ACK 响应
3. **WAIT 响应处理**: 如果收到 WAIT 响应，需要重试操作
4. **多 TAP 链**: 在多 TAP 链中，需要考虑 bypass 位的影响
5. **电源域**: 确保目标芯片的电源域已正确上电

## 测试结果示例

```
====================================

开始 JTAG 测试...
  [1] JTAG 复位完成
  [2] 进入 Run-Test/Idle 状态
  [3] 读取 ICEPick IDCODE: 0x0B7B302F
      - 版本号：0x0
      - 器件型号：0xB7B3
      - 制造商 ID: 0x017
      - 制造商：Texas Instruments
  [✓] JTAG 已连接！

  [4] 开始通过 ICEPick 路由到 DAP...
      - CONNECT 指令已发送
      - DCON 寄存器值：0x09
  [✓] ICEPick 已连接！(CONNECTKEY = 1001b)

  [5] 读取 DAP (CPU) IDCODE...
      - DAP IDCODE: 0x4BA00477
      - 版本号：0x4
      - 器件型号：0xBA00
      - 制造商 ID: 0x23B
      - 制造商：ARM CoreSight
  [✓] DAP (CPU) IDCODE 读取成功！

  [6] 测试 DPACC 访问...
      - DPACC Read ACK: 0x2
      - DP IDCODE: 0x4BA00477
  [✓] DPACC 读取成功！

  [7] CTRL/STAT 寄存器状态:
      - ACK: 0x2
      - CTRL/STAT: 0x00000000
      - 系统电源确认: 否
      - 调试电源确认: 否

  [8] 正在上电 DAP...
  [✓] DAP 上电成功！
      - 上电后 CTRL/STAT: 0xA0000000
      - 系统电源确认: 是
      - 调试电源确认: 是
```

## 版本历史

- v1.0 (2025-11-27): 初始版本，实现基本的 DPACC 访问功能

