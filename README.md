# TMS570LC4357 JTAG GPIO 模拟项目

## 原项目信息

- TI HERCULES HDK
- CCS 构建
- HALCoGen 生成
- OpenOCD 调试
- 开发板 Jtag XDS100v2

---

## 📌 JTAG GPIO 模拟功能

本项目新增了使用 N2HET1 GPIO 引脚模拟 JTAG 信号的功能，实现一块 TMS570LC4357 芯片通过 GPIO 控制另一块 TMS570LC4357 芯片的调试接口。

### 主要特性

- ✅ N2HET1 引脚配置为 GPIO 模式
- ✅ 完整的 JTAG 协议实现（IEEE 1149.1）
- ✅ IR/DR寄存器读写功能
- ✅ IDCODE 自动读取和解析
- ✅ 丰富的示例代码
- ✅ 详细的中文文档

## 引脚映射

| 物理引脚 | N2HET1 引脚 | JTAG 信号 | 信号名称 | 方向 |
|---------|-----------|---------|---------|------|
| B3 | N2HET1_22 | TRST | BMU_M_S_TRST_OUT_R | 输出 |
| J4 | N2HET1_23 | TCK  | BMU_M_S_TCK_OUT_R  | 输出 |
| P1 | N2HET1_24 | TDI  | BMU_M_S_TDI_OUT_R  | 输出 |
| A9 | N2HET1_27 | TDO  | BMU_M_S_TDO_IN_R   | 输入 |
| A3 | N2HET1_29 | TMS  | BMU_M_S_TMS_OUT_R  | 输出 |

## 快速开始

### 1️⃣ 硬件连接

```
主控TMS570 (Master)     目标TMS570 (Slave)
B3  (TRST)  ───────>    TRST
J4  (TCK)   ───────>    TCK
P1  (TDI)   ───────>    TDI
A9  (TDO)   <───────    TDO
A3  (TMS)   ───────>    TMS
GND         ───────>    GND  ⚠️ 必须共地
```

### 2️⃣ 代码集成

```c
#include "jtag_gpio.h"

int main(void)
{
    sciInit();              // 初始化串口
    JTAG_GPIO_Init();       // 初始化 JTAG GPIO
    
    JTAG_Reset();           // 复位 JTAG
    JTAG_Goto_Idle();       // 进入空闲状态
    
    uint32 idcode = JTAG_Read_DR(32);  // 读取器件 ID
    printf("IDCODE: 0x%08X\n", idcode);
    
    // ... 更多操作
}
```

### 3️⃣ 编译运行

1. 在 CCS 项目中添加 `HALCoGen/source/jtag_gpio.c`
2. 编译项目（无错误）
3. 下载到目标板
4. 通过串口查看 JTAG 测试结果

## 📁 项目文件

### 核心代码
```
HALCoGen/
├── include/
│   └── jtag_gpio.h              # JTAG GPIO头文件
└── source/
    ├── jtag_gpio.c              # JTAG GPIO实现
    ├── jtag_gpio_example.c      # 示例代码
    └── HL_sys_main.c            # 主程序（已集成测试）
```

### 文档资料
```
📄 JTAG_GPIO_使用说明.md        # 详细使用文档（推荐）
📄 JTAG_GPIO_快速参考.md        # API快速查询
📄 硬件连接图.txt               # 硬件接线指南
📄 项目总结_JTAG_GPIO模拟.md    # 项目完整总结
```

## 📚 文档导航

**建议阅读顺序：**

1. ⚡ **快速入门（5 分钟）**  
   → `JTAG_GPIO_快速参考.md`

2. 🔌 **硬件连接（10 分钟）**  
   → `硬件连接图.txt`

3. 📖 **详细使用（30 分钟）**  
   → `JTAG_GPIO_使用说明.md`

4. 📊 **项目总览**  
   → `项目总结_JTAG_GPIO模拟.md`

## 🚀 常用 API

### 初始化
```c
void JTAG_GPIO_Init(void);  // 必须首先调用
```

### 引脚控制
```c
void JTAG_Set_TRST(uint32 value);
void JTAG_Set_TCK(uint32 value);
void JTAG_Set_TDI(uint32 value);
void JTAG_Set_TMS(uint32 value);
uint32 JTAG_Get_TDO(void);
```

### JTAG 操作
```c
void JTAG_Reset(void);              // JTAG 复位
void JTAG_Goto_Idle(void);          // 进入空闲状态
void JTAG_Clock_Pulse(void);        // 产生时钟脉冲
```

### 寄存器访问
```c
void JTAG_Write_IR(uint32 ir_value, uint32 ir_len);  // 写 IR
void JTAG_Write_DR(uint32 dr_value, uint32 dr_len);  // 写 DR
uint32 JTAG_Read_DR(uint32 dr_len);                   // 读 DR
```

完整 API 请查看 `jtag_gpio.h` 或快速参考文档。

## 💡 使用示例

### 示例 1：读取器件 ID
```c
JTAG_GPIO_Init();
JTAG_Reset();
JTAG_Goto_Idle();
uint32 idcode = JTAG_Read_DR(32);

// 解析 IDCODE
uint32 version = (idcode >> 28) & 0x0F;
uint32 part_num = (idcode >> 12) & 0xFFFF;
uint32 mfg_id = (idcode >> 1) & 0x7FF;

printf("Version: 0x%X\n", version);
printf("Part: 0x%04X\n", part_num);
printf("Manufacturer: 0x%03X\n", mfg_id);
```

### 示例 2：写入调试寄存器
```c
// 写入指令寄存器
JTAG_Write_IR(0x05, 5);

// 写入数据寄存器
JTAG_Write_DR(0x12345678, 32);
```

### 示例 3：读取调试数据
```c
// 选择要读取的寄存器
JTAG_Write_IR(0x03, 5);

// 读取数据
uint32 data = JTAG_Read_DR(32);
printf("Data: 0x%08X\n", data);
```

更多示例请参考 `jtag_gpio_example.c`。

## ⚙️ 技术规格

| 参数 | 值 | 说明 |
|------|-----|------|
| 支持芯片 | TMS570LC4357 | 可扩展到其他 TMS570 系列 |
| JTAG 时钟 | ~10 kHz | 软件延时实现 |
| IR 长度 | 5 位 | TMS570 标准 |
| DR 长度 | 可变 | 取决于具体寄存器 |
| 电压等级 | 3.3V | 符合 TMS570 规范 |
| 协议标准 | IEEE 1149.1 | 标准 JTAG |

## ⚠️ 注意事项

1. **硬件要求**
   - ⚠️ 两块芯片必须共地（GND 连接）
   - ⚠️ 使用相同电源电压（3.3V）
   - ⚠️ 目标芯片必须先上电

2. **软件要求**
   - ⚠️ 必须首先调用 `JTAG_GPIO_Init()`
   - ⚠️ 确保 HALCoGen 配置正确
   - ⚠️ 引脚不与其他功能冲突

3. **信号要求**
   - ⚠️ 信号线不宜过长（建议<30cm）
   - ⚠️ 避免与强干扰源靠近
   - ⚠️ 确保连接可靠

## 🔧 故障排除

### 问题 1：无法读取 IDCODE
**症状：** IDCODE 读取为 0x00000000

**解决方法：**
- [ ] 检查硬件连接是否正确
- [ ] 确认目标芯片供电正常
- [ ] 验证 GND 是否可靠连接
- [ ] 检查 TDO 引脚连接

### 问题 2：IDCODE 读取为全 1
**症状：** IDCODE 读取为 0xFFFFFFFF

**解决方法：**
- [ ] 检查 TDO 引脚是否连接
- [ ] 确认目标芯片是否在工作状态
- [ ] 验证时钟信号是否正常

### 问题 3：编译错误
**症状：** 找不到头文件或函数未定义

**解决方法：**
- [ ] 确认 `jtag_gpio.c` 已添加到项目
- [ ] 检查包含路径设置
- [ ] 验证 HALCoGen 配置

更多问题请参考详细文档的故障排除章节。

## 📊 性能指标

| 操作 | 时间 | 备注 |
|------|------|------|
| 单 bit 传输 | ~100 μs | 含延时 |
| IDCODE 读取 | ~3.2 ms | 32 位 |
| IR 写入 (5 位) | ~0.5 ms | - |
| DR 写入 (32 位) | ~3.2 ms | - |

## 🛠️ 开发环境

- **IDE：** Code Composer Studio (CCS)
- **HALCoGen：** v04.07.01
- **编译器：** TI ARM Compiler
- **目标器件：** TMS570LC4357
- **调试器：** XDS100v2

## 📝 版本历史

### v1.0 (2025-11-19)
- ✨ 初始版本发布
- ✨ 实现基本 JTAG GPIO 功能
- ✨ 支持 IR/DR 读写
- ✨ 提供完整文档和示例
- ✨ 集成到主程序

## 📄 许可证

本项目遵循 TI HALCoGen 生成代码的许可协议。

## 🔗 参考资料

- [TMS570LC43xx Technical Reference Manual](https://www.ti.com/)
- [IEEE 1149.1 JTAG 标准](https://standards.ieee.org/)
- [TI E2E 论坛](https://e2e.ti.com/)
- [ARM Debug Interface v5 规范](https://developer.arm.com/)

## 💬 技术支持

如有问题，请：
1. 查阅项目文档
2. 检查故障排除章节
3. 访问 TI E2E 论坛

---

**项目状态：** ✅ 完成并可用  
**最后更新：** 2025-11-19  
**版本：** v1.0  
**作者：** 根据用户需求定制开发

---

*祝您使用愉快！如有问题请参考详细文档。*
