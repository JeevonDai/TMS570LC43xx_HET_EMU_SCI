# JTAG GPIO 模拟使用说明

## 概述

本项目实现了使用TMS570LC4357的N2HET1引脚模拟JTAG信号的功能，可以用于调试和控制另一块TMS570LC4357芯片。

## 引脚映射

| 物理引脚 | N2HET1引脚 | JTAG信号 | 方向 | 功能描述 |
|---------|-----------|---------|------|---------|
| B3 | N2HET1_22 | TRST | 输出 | 测试复位（低电平有效）|
| J4 | N2HET1_23 | TCK | 输出 | 测试时钟 |
| P1 | N2HET1_24 | TDI | 输出 | 测试数据输入 |
| A9 | N2HET1_27 | TDO | 输入 | 测试数据输出 |
| A3 | N2HET1_29 | TMS | 输出 | 测试模式选择 |

## 硬件连接

将主控芯片（发送JTAG信号）的引脚连接到目标芯片（接收JTAG信号）：

```
主控TMS570 (Master)          目标TMS570 (Slave)
===================          ==================
B3  (TRST_OUT)  ---------->  TRST (调试接口)
J4  (TCK_OUT)   ---------->  TCK  (调试接口)
P1  (TDI_OUT)   ---------->  TDI  (调试接口)
A9  (TDO_IN)    <----------  TDO  (调试接口)
A3  (TMS_OUT)   ---------->  TMS  (调试接口)
GND             ---------->  GND
```

**注意：** 请确保两块芯片共地。

## 文件说明

### 核心文件
- `HALCoGen/include/jtag_gpio.h` - JTAG GPIO头文件，包含所有函数声明和定义
- `HALCoGen/source/jtag_gpio.c` - JTAG GPIO实现文件，包含底层驱动
- `HALCoGen/source/jtag_gpio_example.c` - 使用示例代码

## 使用步骤

### 1. 初始化

在main函数中调用初始化函数：

```c
#include "jtag_gpio.h"

int main(void)
{
    // 系统初始化
    sciInit();
    
    // JTAG GPIO初始化
    JTAG_GPIO_Init();
    
    // ... 其他代码
}
```

### 2. 基本操作

#### 2.1 复位目标芯片JTAG

```c
JTAG_Reset();       // 复位JTAG状态机
JTAG_Goto_Idle();   // 进入空闲状态
```

#### 2.2 读取器件ID

```c
uint32 idcode;
JTAG_Reset();
JTAG_Goto_Idle();
idcode = JTAG_Read_DR(32);  // 读取32位IDCODE
printf("Device IDCODE: 0x%08X\n", idcode);
```

#### 2.3 写入指令寄存器(IR)

```c
uint32 instruction = 0x05;  // 示例指令
uint32 ir_length = 5;       // TMS570的IR长度为5位
JTAG_Write_IR(instruction, ir_length);
```

#### 2.4 写入数据寄存器(DR)

```c
uint32 data = 0x12345678;
uint32 dr_length = 32;
JTAG_Write_DR(data, dr_length);
```

#### 2.5 读取数据寄存器(DR)

```c
uint32 data;
data = JTAG_Read_DR(32);  // 读取32位数据
```

### 3. 高级操作

#### 3.1 单bit操作

```c
uint32 tdo;
tdo = JTAG_Shift_Bit(1, 0);  // TMS=1, TDI=0, 返回TDO值
```

#### 3.2 多bit移位

```c
uint32 tdi_data = 0xAA;
uint32 tdo_data;
JTAG_Shift_Data(0, tdi_data, 8, &tdo_data);  // 移位8位
```

#### 3.3 直接引脚控制

```c
JTAG_Set_TRST(0);    // TRST置低
JTAG_Set_TCK(1);     // TCK置高
JTAG_Set_TDI(1);     // TDI置高
JTAG_Set_TMS(0);     // TMS置低
uint32 tdo = JTAG_Get_TDO();  // 读取TDO
```

## API参考

### 初始化函数

```c
void JTAG_GPIO_Init(void);
```
配置N2HET1引脚为GPIO模式并初始化默认状态。

### 控制函数

```c
void JTAG_Set_TRST(uint32 value);  // 设置TRST电平 (0/1)
void JTAG_Set_TCK(uint32 value);   // 设置TCK电平 (0/1)
void JTAG_Set_TDI(uint32 value);   // 设置TDI电平 (0/1)
void JTAG_Set_TMS(uint32 value);   // 设置TMS电平 (0/1)
uint32 JTAG_Get_TDO(void);         // 读取TDO电平
```

### 时序函数

```c
void JTAG_Clock_Pulse(void);       // 产生一个TCK时钟脉冲
void JTAG_Reset(void);             // JTAG复位序列
void JTAG_Goto_Idle(void);         // 进入Run-Test/Idle状态
```

### 数据传输函数

```c
uint32 JTAG_Shift_Bit(uint32 tms, uint32 tdi);  // 移位一位
void JTAG_Shift_Data(uint32 tms, uint32 tdi_data, 
                     uint32 bit_count, uint32* tdo_data);  // 移位多位
void JTAG_Write_IR(uint32 ir_value, uint32 ir_len);  // 写IR
void JTAG_Write_DR(uint32 dr_value, uint32 dr_len);  // 写DR
uint32 JTAG_Read_DR(uint32 dr_len);                   // 读DR
```

## TMS570LC4357 JTAG信息

### IDCODE
- **长度：** 32位
- **格式：**
  - Bits [31:28] - Version (版本号)
  - Bits [27:12] - Part Number (器件型号)
  - Bits [11:1]  - Manufacturer ID (制造商ID，TI = 0x017)
  - Bit [0]      - 固定为1

### 指令寄存器(IR)
- **长度：** 5位
- **常用指令：**
  - `BYPASS` (0x1F) - 旁路
  - `IDCODE` (0x02) - 读取器件ID
  - `SAMPLE/PRELOAD` (0x03) - 边界扫描采样
  - `EXTEST` (0x00) - 外部测试

## 注意事项

1. **时钟频率：** 当前实现使用软件延时，JTAG时钟频率较低。如果需要更高速度，可以调整延时参数。

2. **电平匹配：** 确保两块芯片的IO电平兼容（TMS570LC4357使用3.3V电平）。

3. **上拉电阻：** 代码已经配置了内部上拉电阻，通常不需要外部上拉。

4. **TRST信号：** TRST是可选信号，如果目标芯片没有TRST引脚，保持高电平即可。

5. **状态机：** 了解JTAG状态机转换对于调试很重要，参考IEEE 1149.1标准。

6. **IR长度：** 不同器件的IR长度可能不同，TMS570系列为5位。

7. **多器件链：** 当前实现假设链上只有一个器件，多器件链需要额外处理。

## 调试建议

1. **示波器验证：** 使用示波器检查TCK、TMS、TDI信号的时序是否正确。

2. **IDCODE验证：** 首先尝试读取IDCODE，这是最简单的验证方式。

3. **边界扫描：** 使用边界扫描测试引脚连接。

4. **逐步调试：** 从简单的引脚操作开始，逐步测试复杂功能。

## 示例代码

完整的示例代码请参考 `jtag_gpio_example.c` 文件，包含：
- 基本初始化和读取IDCODE
- IR/DR读写操作
- 边界扫描示例
- 低层引脚控制
- TMS570特定的调试操作

## 编译配置

### CCS项目设置

1. 将以下文件添加到项目：
   - `HALCoGen/source/jtag_gpio.c`
   - `HALCoGen/source/jtag_gpio_example.c` (如果需要示例)

2. 确保包含路径正确：
   - 项目属性 -> C/C++ Build -> Settings -> Include Options
   - 添加：`${PROJECT_ROOT}/HALCoGen/include`

3. 编译并下载到目标板

## 故障排除

### 问题1：无法读取IDCODE
- 检查硬件连接
- 确认目标芯片供电正常
- 验证引脚配置正确

### 问题2：读取的数据全为0xFF或0x00
- 检查TDO引脚连接
- 验证时钟信号是否正常
- 检查目标芯片是否在JTAG模式

### 问题3：编译错误
- 确认所有头文件路径正确
- 检查HALCoGen配置是否使能了N2HET1模块

## 参考资料

1. TMS570LC43xx Technical Reference Manual (TRM)
2. IEEE 1149.1 JTAG标准
3. TMS570LC4357 BSDL文件
4. ARM Debug Interface v5 Architecture Specification

## 版本历史

- v1.0 (2025-11-19) - 初始版本
  - 实现基本JTAG GPIO功能
  - 支持IR/DR读写
  - 提供完整示例代码

## 作者

根据用户需求开发，用于TMS570LC4357芯片间JTAG通信。

## 许可

遵循TI HALCoGen生成代码的许可协议。

