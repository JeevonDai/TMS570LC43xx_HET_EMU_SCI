# JTAG GPIO 快速参考卡

## 引脚映射速查表

| 信号 | 物理引脚 | N2HET1引脚 | 方向 | 默认状态 |
|------|---------|-----------|------|---------|
| TRST | B3      | PIN_22    | 输出 | 高(1)   |
| TCK  | J4      | PIN_23    | 输出 | 低(0)   |
| TDI  | P1      | PIN_24    | 输出 | 低(0)   |
| TDO  | A9      | PIN_27    | 输入 | -       |
| TMS  | A3      | PIN_29    | 输出 | 高(1)   |

## 常用函数速查

### 基础初始化
```c
JTAG_GPIO_Init();    // 初始化GPIO，必须首先调用
```

### JTAG标准操作
```c
JTAG_Reset();        // 复位JTAG（发送5+个TMS=1）
JTAG_Goto_Idle();    // 进入Run-Test/Idle状态
JTAG_Clock_Pulse();  // 产生一个时钟脉冲
```

### 引脚直接控制
```c
JTAG_Set_TRST(1);    // TRST = 高电平
JTAG_Set_TCK(1);     // TCK = 高电平
JTAG_Set_TDI(1);     // TDI = 高电平
JTAG_Set_TMS(1);     // TMS = 高电平
uint32 val = JTAG_Get_TDO();  // 读取TDO
```

### 数据传输
```c
// 写指令寄存器
JTAG_Write_IR(0x05, 5);  // 指令值, IR长度

// 写数据寄存器
JTAG_Write_DR(0x12345678, 32);  // 数据值, DR长度

// 读数据寄存器
uint32 data = JTAG_Read_DR(32);  // 读取32位

// 单bit移位
uint32 tdo = JTAG_Shift_Bit(tms, tdi);  // 返回TDO
```

## 典型使用流程

### 1. 初始化和读取IDCODE
```c
// 初始化
JTAG_GPIO_Init();

// 复位并读取IDCODE
JTAG_Reset();
JTAG_Goto_Idle();
uint32 idcode = JTAG_Read_DR(32);
printf("IDCODE: 0x%08X\n", idcode);
```

### 2. 写入调试寄存器
```c
// 写入IR指令（例如：选择某个DR）
JTAG_Write_IR(DEBUG_CMD, 5);

// 写入DR数据
JTAG_Write_DR(debug_data, 32);
```

### 3. 读取调试寄存器
```c
// 写入IR指令
JTAG_Write_IR(READ_CMD, 5);

// 读取DR数据
uint32 result = JTAG_Read_DR(32);
```

## TMS570LC4357 JTAG信息

### IDCODE格式
```
Bits 31-28: Version      (版本)
Bits 27-12: Part Number  (器件型号)
Bits 11-1:  Mfg ID       (制造商ID, TI=0x017)
Bit  0:     固定为1
```

### 常用IR指令
```c
#define IR_BYPASS    0x1F  // 旁路 (5位全1)
#define IR_IDCODE    0x02  // 读取IDCODE
#define IR_SAMPLE    0x03  // 边界扫描采样
#define IR_EXTEST    0x00  // 外部测试
#define IR_LENGTH    5     // IR长度
```

## JTAG状态机转换

```
               TMS=1
      Reset -----------> Idle
        ^                 |
        | TMS=1           | TMS=1
        |                 v
        |            Select-DR
        |           /          \
      TMS=1      TMS=0        TMS=1
        |         |              |
        |    Capture-DR    Select-IR
        |         |              |
        |    Shift-DR       Capture-IR
        |         |              |
        |    Exit1-DR        Shift-IR
        |         |              |
        |    Update-DR       Exit1-IR
        |         |              |
        |         +----TMS=0-----+
        |                |
        +------TMS=1-----+
```

## 调试检查清单

### 硬件检查
- [ ] 目标芯片供电正常（3.3V）
- [ ] JTAG引脚正确连接
- [ ] 两块芯片共地
- [ ] 无短路或接触不良

### 软件检查
- [ ] 调用 `JTAG_GPIO_Init()` 初始化
- [ ] 引脚功能未被其他模块占用
- [ ] 时序延时是否足够

### 信号验证（使用示波器）
- [ ] TCK时钟波形正常
- [ ] TMS、TDI信号变化正确
- [ ] TDO有响应数据

## 故障诊断速查

| 症状 | 可能原因 | 解决方法 |
|------|---------|---------|
| IDCODE全0 | TDO未连接/目标无电 | 检查连接和供电 |
| IDCODE全1 | TDO悬空 | 检查TDO连接 |
| 数据不稳定 | 时序问题 | 增加延时 |
| 完全无响应 | 初始化失败 | 检查代码初始化 |

## 示例：完整的读写流程

```c
void jtag_example(void)
{
    uint32 idcode, data;
    
    // 步骤1: 初始化
    JTAG_GPIO_Init();
    
    // 步骤2: 复位JTAG
    JTAG_Reset();
    JTAG_Goto_Idle();
    
    // 步骤3: 读取IDCODE验证连接
    idcode = JTAG_Read_DR(32);
    if (idcode == 0 || idcode == 0xFFFFFFFF) {
        // 连接失败
        return;
    }
    
    // 步骤4: 写入IR指令
    JTAG_Write_IR(0x05, 5);
    
    // 步骤5: 写入数据
    JTAG_Write_DR(0x12345678, 32);
    
    // 步骤6: 读取数据
    data = JTAG_Read_DR(32);
    
    // 步骤7: 返回Idle状态
    JTAG_Goto_Idle();
}
```

## 性能参考

| 参数 | 典型值 | 备注 |
|------|--------|------|
| TCK频率 | ~10kHz | 软件延时实现 |
| 单bit时间 | ~100us | 包含延时 |
| IDCODE读取 | ~3.2ms | 32位 |
| 最大DR长度 | 不限 | 受内存限制 |

## 引脚电气特性

| 参数 | 值 |
|------|-----|
| IO电平 | 3.3V |
| 输出驱动 | 标准 |
| 输入阻抗 | 带上拉 |
| 最大频率 | 参考TMS570手册 |

## 注意事项

⚠️ **重要提示：**
1. 必须先调用 `JTAG_GPIO_Init()` 初始化
2. 确保两块芯片共地
3. 不要在JTAG操作期间改变pinmux配置
4. TDO是唯一的输入引脚
5. 当前实现为阻塞式，操作期间会占用CPU

## 相关文件

- `jtag_gpio.h` - 头文件
- `jtag_gpio.c` - 实现文件
- `jtag_gpio_example.c` - 示例代码
- `JTAG_GPIO_使用说明.md` - 详细文档

## 版本信息

**版本：** v1.0  
**日期：** 2025-11-19  
**平台：** TMS570LC4357  
**工具链：** TI CCS/GCC

---

*更多详细信息请参阅《JTAG_GPIO_使用说明.md》*

