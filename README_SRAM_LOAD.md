# TMS570LC43xx JTAG SRAM 加载器说明文档

## 概述

本文档说明如何通过 JTAG 接口将 bin 文件写入 TMS570LC43xx 的 SRAM 并启动执行。

参考文档：[TI SPNA230 - Hercules JTAG Scan Architecture](https://www.ti.com/lit/an/spna230/spna230.pdf) 第 2.3.4 节 "Example: Steps to Write to System Memory Using APB-AP Interface"

## 内存映射

根据 `Debug_SRAM/TMS570LC43xx_SRAM.map` 文件：

| 区域 | 起始地址 | 长度 | 用途 |
|------|----------|------|------|
| VECTORS | 0x08000000 | 0x20 | 中断向量表 |
| CODE | 0x08000020 | 0x3FFE0 | 程序代码 |
| STACKS | 0x08040000 | 0x1500 | 堆栈区 |
| RAM | 0x08041500 | 0x3EB00 | 数据区 |

- **bin 文件写入地址**: `0x08000000` (SRAM 起始地址)
- **程序入口点**: `0x080087dc` (符号 `_c_int00`)

## 完整流程

### 阶段 1: JTAG 初始化与 ICEPick 连接

```
[1] JTAG 复位 → [2] 进入 Idle → [3] 读取 ICEPick IDCODE
                                           ↓
[4] ICEPick CONNECT → [5] 路由 SDTAP0 (DAP) 到扫描链
```

### 阶段 2: DAP 初始化

```
[6] 读取 DAP IDCODE → [7] DAP 上电 (CTRL/STAT)
                                ↓
[8] 激活 APB-AP (SELECT = 0x01000000)
                                ↓
[9] 发送 HALT 请求，CPU 进入调试模式
```

### 阶段 3: AHB-AP 写入 SRAM

```
[10] 激活 AHB-AP (SELECT = 0x00000000)
                    ↓
[11] 配置 AHB-AP.CSW = 0x43000002 (32-bit 访问)
                    ↓
[12] 循环: 设置 TAR → 写入 DRW (每次 4 字节)
                    ↓
[13] 完成 bin 文件写入
```

### 阶段 4: 设置 PC 并启动执行

参考代码 `HALCoGen/source/jtag_gpio.c` 中的 `JTAG_Set_PC_And_Run()` 函数：

```c
uint32 JTAG_Set_PC_And_Run(uint32 entry_addr)  // entry_addr = 0x080087dc
{
    /* 1. 再次激活 APB-AP */
    JTAG_DPACC_Write(DP_ADDR_SELECT, 0x01000000);

    /* 2. 将入口地址写入 DTRRX (0x80001080) */
    JTAG_APB_AP_Write(DBG_DTRRX_ADDR, entry_addr);

    /* 3. 通过 ITR (0x80001084) 执行: MRC p14, 0, R0, c0, c5, 0 
     *    将 DTRRX 的值读入 R0 */
    JTAG_APB_AP_Write(DBG_ITR_ADDR, ARM_INSTR_MRC_DTRRX_R0);

    /* 4. 通过 ITR 执行: BX R0 
     *    跳转到 R0 指向的地址（即 0x080087dc）*/
    JTAG_APB_AP_Write(DBG_ITR_ADDR, ARM_INSTR_BX_R0);

    /* 5. 发送 RESTART 请求 (DRCR = 0x80001090)，恢复 CPU 执行 */
    JTAG_APB_AP_Write(DBG_DRCR_ADDR, DRCR_RESTART);

    return 0;
}
```

## APB-AP 调试寄存器详解

根据 TI SPNA230 文档 2.3.4 节，通过 APB-AP 接口设置 PC 的步骤：

### 调试寄存器地址

| 寄存器 | 地址 | 说明 |
|--------|------|------|
| DTRRX | 0x80001080 | 调试数据传输接收寄存器（外部写入） |
| ITR | 0x80001084 | 指令传输寄存器 |
| DSCR | 0x80001088 | 调试状态和控制寄存器 |
| DTRTX | 0x8000108C | 调试数据传输发送寄存器（外部读取） |
| DRCR | 0x80001090 | 调试运行控制寄存器 |

### ARM 指令编码

| 指令 | 编码 | 说明 |
|------|------|------|
| MRC p14, 0, R0, c0, c5, 0 | 0xEE100E15 | 从 DTRRX 读取到 R0 |
| BX R0 | 0xE12FFF10 | 跳转到 R0 指向的地址 |

### DRCR 位定义

| 位 | 名称 | 说明 |
|----|------|------|
| [0] | HALT | 请求 CPU 停止 |
| [1] | RESTART | 请求 CPU 重启 |
| [2] | CLR_EXCEPTIONS | 清除粘滞异常 |

## 数据流图

```
┌─────────────────────────────────────────────────────────────────────────┐
│                           主控 MCU (TMS570LC4357)                        │
│                                                                          │
│  ┌──────────────┐      ┌──────────────┐      ┌────────────────────────┐ │
│  │   FLASH      │      │   JTAG GPIO  │      │      SCI 串口          │ │
│  │ (bin 文件)   │ ───> │   模拟器     │      │   (调试输出)           │ │
│  │ 0x00200000   │      │   N2HET1     │      │                        │ │
│  └──────────────┘      └──────┬───────┘      └────────────────────────┘ │
└─────────────────────────────────┼────────────────────────────────────────┘
                                  │ JTAG (TCK/TMS/TDI/TDO/TRST)
                                  ↓
┌─────────────────────────────────────────────────────────────────────────┐
│                           目标 MCU (TMS570LC4357)                        │
│                                                                          │
│  ┌──────────────────────────────────────────────────────────────────┐   │
│  │                         ICEPick TAP                               │   │
│  │                    (JTAG 路由控制器)                              │   │
│  └──────────────────────────────┬───────────────────────────────────┘   │
│                                 │ SDTAP0                                 │
│                                 ↓                                        │
│  ┌──────────────────────────────────────────────────────────────────┐   │
│  │                         ARM DAP                                    │   │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐   │   │
│  │  │   JTAG-DP   │  │   AHB-AP    │  │        APB-AP           │   │   │
│  │  │             │  │             │  │                         │   │   │
│  │  │ CTRL/STAT   │  │ 访问 SRAM   │  │ 访问调试寄存器          │   │   │
│  │  │ SELECT      │  │ 0x08000000  │  │ DTRRX/ITR/DSCR/DRCR     │   │   │
│  │  │ RDBUFF      │  │             │  │ 0x80001080-0x80001090   │   │   │
│  │  └─────────────┘  └──────┬──────┘  └───────────┬─────────────┘   │   │
│  └──────────────────────────┼─────────────────────┼─────────────────┘   │
│                             │                     │                      │
│                             ↓                     ↓                      │
│  ┌──────────────────────────────────────────────────────────────────┐   │
│  │                      ARM Cortex-R4F CPU                           │   │
│  │                                                                    │   │
│  │    PC = 0x080087dc (_c_int00)                                     │   │
│  │                                                                    │   │
│  │    ┌────────────────────────────────────────────────────────┐     │   │
│  │    │                     SRAM                                │     │   │
│  │    │  0x08000000 ┌─────────────────┐                        │     │   │
│  │    │             │   VECTORS (32B) │                        │     │   │
│  │    │  0x08000020 ├─────────────────┤                        │     │   │
│  │    │             │                 │                        │     │   │
│  │    │             │   CODE          │                        │     │   │
│  │    │             │                 │                        │     │   │
│  │    │  0x080087dc │ → _c_int00      │ ← PC 入口点            │     │   │
│  │    │             │                 │                        │     │   │
│  │    │  0x08040000 ├─────────────────┤                        │     │   │
│  │    │             │   STACKS        │                        │     │   │
│  │    │  0x08041500 ├─────────────────┤                        │     │   │
│  │    │             │   RAM           │                        │     │   │
│  │    │  0x08080000 └─────────────────┘                        │     │   │
│  │    └────────────────────────────────────────────────────────┘     │   │
│  └───────────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────┘
```

## PC 设置详细步骤（对应 TI SPNA230 2.3.4 节）

### 步骤 1: 激活 APB-AP

```
写入 JTAG-DP.SELECT = 0x01000000
├── APSEL[31:24] = 0x01  → 选择 APB-AP
└── APBANKSEL[7:4] = 0   → 选择 bank 0
```

### 步骤 2: 将入口地址写入 DTRRX

```
通过 APB-AP 写入 DTRRX (0x80001080):
├── 设置 APB-AP.TAR = 0x80001080
└── 写入 APB-AP.DRW = 0x080087dc (入口地址)
```

### 步骤 3: 执行 MRC 指令读取 DTRRX 到 R0

```
通过 APB-AP 写入 ITR (0x80001084):
├── 设置 APB-AP.TAR = 0x80001084
└── 写入 APB-AP.DRW = 0xEE100E15 (MRC p14, 0, R0, c0, c5, 0)

执行效果: R0 = 0x080087dc
```

### 步骤 4: 执行 BX R0 跳转到入口地址

```
通过 APB-AP 写入 ITR (0x80001084):
├── 设置 APB-AP.TAR = 0x80001084
└── 写入 APB-AP.DRW = 0xE12FFF10 (BX R0)

执行效果: PC 准备跳转到 R0 = 0x080087dc
```

### 步骤 5: 发送 RESTART 命令恢复 CPU 执行

```
通过 APB-AP 写入 DRCR (0x80001090):
├── 设置 APB-AP.TAR = 0x80001090
└── 写入 APB-AP.DRW = 0x00000002 (RESTART 位)

执行效果: CPU 退出调试模式，从 PC=0x080087dc 开始执行
```

## 配置参数

在 `HL_sys_main.c` 中配置以下参数：

```c
#define SRAM_BIN_FLASH_ADDR 0x00200000U /* bin 文件在 FLASH BANK1 的起始地址 */
#define SRAM_BIN_SIZE 38000U            /* bin 文件大小（字节）*/
#define TARGET_SRAM_BASE 0x08000000U    /* 目标芯片 SRAM 起始地址 */
```

## 入口地址说明

入口地址 `0x080087dc` 来自 `.map` 文件：

```
ENTRY POINT SYMBOL: "_c_int00"  address: 080087dc
```

`_c_int00` 是 TI C 运行时库的入口点，负责：
1. 初始化 BSS 段
2. 初始化全局变量
3. 调用 `main()` 函数

> **注意**: 不要直接跳转到 `0x08000000`（向量表起始），因为向量表中是跳转指令，而非直接的代码入口。正确做法是跳转到 `_c_int00`。

## 硬件连接

N2HET1 引脚映射：

| N2HET1 引脚 | 功能 | 方向 |
|-------------|------|------|
| N2HET1_22 (B3) | TRST | 输出 |
| N2HET1_23 (J4) | TCK | 输出 |
| N2HET1_24 (P1) | TDI | 输出 |
| N2HET1_27 (A9) | TDO | 输入 |
| N2HET1_29 (A3) | TMS | 输出 |

## 错误码说明

`JTAG_Set_PC_And_Run()` 返回值：

| 返回值 | 说明 |
|--------|------|
| 0 | 成功 |
| 1 | 再次激活 APB-AP 失败 |
| 2 | 写入 DTRRX 失败 |
| 3 | 写入 ITR (MRC) 失败 |
| 4 | 写入 ITR (BX) 失败 |
| 5 | 发送 RESTART 失败 |

## 参考资料

1. [TI SPNA230 - Hercules JTAG Scan Architecture](https://www.ti.com/lit/an/spna230/spna230.pdf)
2. ARM CoreSight Technical Reference Manual
3. ARM Debug Interface v5 Architecture Specification
4. TMS570LC43x Technical Reference Manual
