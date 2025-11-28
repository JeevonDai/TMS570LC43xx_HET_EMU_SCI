# ICEPick 旁路位修复说明

## 问题描述

在多 TAP 链环境中（ICEPick + DAP），访问 DPACC 时需要考虑 ICEPick 的旁路位：
- **IR 链**：ICEPick IR (6 位) + DAP IR (4 位) = 10 位
- **DR 链**：ICEPick BYPASS (1 位) + DAP DR (35 位) = 36 位

之前的实现在 `JTAG_DPACC_Read` 和 `JTAG_DPACC_Write` 函数中：
1. ❌ 只写入了 4 位 IR（DAP_IR_DPACC），缺少 ICEPick 的 6 位旁路
2. ❌ 只移位了 35 位 DR，缺少 ICEPick 的 1 位旁路

## 解决方案：方案 1 - 旁路处理外置

将 ICEPick 旁路处理从函数内部移到外部，由调用者负责设置完整的 IR。

### 修改内容

#### 1. `JTAG_DPACC_Write` 函数修改

**变更点：**
- ✅ **删除内部 IR 设置**：移除了 `JTAG_Write_IR_Pause(DAP_IR_DPACC, 4)` 调用
- ✅ **添加 ICEPick 旁路位**：DR 从 35 位改为 36 位
  - 第一次 DR 写入：35 位 DPACC + 1 位 bypass = 36 位
  - 第二次 DR 读取 ACK：35 位 + 1 位 bypass = 36 位
- ✅ **更新注释**：说明需要调用者先设置 IR

**关键代码片段：**
```c
// 旧代码：只移位 32 位数据后就退出
for (i = 0; i < 32; i++) {
    uint32 bit = (data >> i) & 0x01U;
    if (i == 31) {
        tdo = JTAG_Shift_Bit(1, bit);  // 最后一位退出 ❌
    }
    else {
        JTAG_Shift_Bit(0, bit);
    }
}

// 新代码：移位 32 位数据后，再移位 1 位 bypass
for (i = 0; i < 32; i++) {
    uint32 bit = (data >> i) & 0x01U;
    JTAG_Shift_Bit(0, bit);  // 不退出
}
// 移位 1 位 ICEPick BYPASS（填充 0，最后一位退出）
tdo = JTAG_Shift_Bit(1, 0);  // ✅
```

#### 2. `JTAG_DPACC_Read` 函数修改

**变更点：**
- ✅ **删除内部 IR 设置**：移除了 `JTAG_Write_IR_Pause(DAP_IR_DPACC, 4)` 调用
- ✅ **添加 ICEPick 旁路位**：两次 DR 扫描都改为 36 位
  - 第一次发送读请求：35 位 + 1 位 bypass = 36 位
  - 第二次读取数据：35 位 + 1 位 bypass = 36 位
- ✅ **更新注释**：说明需要调用者先设置 IR

**关键代码片段：**
```c
// 第一次 DR 扫描：发送读请求
for (i = 0; i < 35; i++) {
    uint32 bit = (request >> i) & 0x01U;
    if (i >= 3) bit = 0;
    JTAG_Shift_Bit(0, bit);  // 不退出
}
// 移位 1 位 ICEPick BYPASS（最后一位退出）
JTAG_Shift_Bit(1, 0);  // ✅

// 第二次 DR 扫描：读取数据
// 读取 3 位 ACK + 32 位数据
for (i = 0; i < 32; i++) {
    tdo = JTAG_Shift_Bit(0, 0);
    read_data |= (tdo & 0x01U) << i;
}
// 读取 1 位 ICEPick BYPASS（最后一位退出）
JTAG_Shift_Bit(1, 0);  // ✅
```

#### 3. 主程序 `HL_sys_main.c` 修改

**变更点：**
- ✅ **删除多余的状态转换**：移除了两个 `JTAG_From_Pause_To_Select_DR_Scan()` 调用
  - 第一个在调用 `JTAG_DPACC_Read` 之前（第 230 行）
  - 第二个在调用 `JTAG_DAP_PowerUp` 之前（第 239 行）
- ✅ **保留 IR 设置**：主程序已正确设置包含 ICEPick BYPASS 的 IR

**修改前：**
```c
JTAG_Write_IR_Pause(DAP_IR_DPACC | ICEPICK_IR_BYPASS << DAP_IR_LENGTH,
                    DAP_IR_LENGTH + ICEPICK_IR_LENGTH);

JTAG_From_Pause_To_Select_DR_Scan();  // ❌ 多余

uint32 ctrl_stat = 0;
uint32 ack = JTAG_DPACC_Read(DP_ADDR_CTRL_STAT, &ctrl_stat);
```

**修改后：**
```c
JTAG_Write_IR_Pause(DAP_IR_DPACC | ICEPICK_IR_BYPASS << DAP_IR_LENGTH,
                    DAP_IR_LENGTH + ICEPICK_IR_LENGTH);

// 函数内部会处理状态转换 ✅
uint32 ctrl_stat = 0;
uint32 ack = JTAG_DPACC_Read(DP_ADDR_CTRL_STAT, &ctrl_stat);
```

## JTAG 状态机流程

### 修改后的正确流程

1. **主程序设置 IR（包含 ICEPick BYPASS）**
   ```
   Pause-DR → Select-DR-Scan → Select-IR-Scan → Capture-IR → 
   Shift-IR (10 位) → Exit1-IR → Pause-IR
   ```

2. **JTAG_DPACC_Read/Write 执行 DR 扫描**
   ```
   Pause-IR → Exit2-IR → Update-IR → Select-DR-Scan → Capture-DR → 
   Shift-DR (36 位) → Exit1-DR → Pause-DR
   ```

## 数据格式

### DPACC DR 格式（包含 ICEPick BYPASS）

**写操作（36 位）：**
```
[35]    - ICEPick BYPASS (填充 0)
[34:3]  - 32 位写入数据
[2]     - RnW = 0 (写操作)
[1:0]   - A[3:2] 地址位
```

**读操作 - 第一次扫描（36 位）：**
```
[35]    - ICEPick BYPASS (填充 0)
[34:3]  - 32 位数据占位 (填充 0)
[2]     - RnW = 1 (读操作)
[1:0]   - A[3:2] 地址位
```

**读操作 - 第二次扫描（36 位）：**
```
[35]    - ICEPick BYPASS (忽略)
[34:3]  - 32 位读取的数据
[2:0]   - 3 位 ACK 响应
```

## 关键要点

1. ✅ **IR 链长度**：ICEPick IR (6 位) + DAP IR (4 位) = **10 位**
2. ✅ **DR 链长度**：ICEPick BYPASS (1 位) + DPACC DR (35 位) = **36 位**
3. ✅ **调用约定**：调用 `JTAG_DPACC_Read/Write` 前必须先设置完整的 IR
4. ✅ **状态管理**：函数内部会从 Pause 状态转换到 Select-DR-Scan，无需外部干预
5. ✅ **旁路位处理**：每次 DR 扫描的最后都要移位 ICEPick BYPASS 位

## 测试验证

修改后的代码应该能够：
- ✅ 正确读取 DP IDCODE
- ✅ 正确读取 CTRL/STAT 寄存器
- ✅ 成功上电 DAP（CSYSPWRUPACK 和 CDBGPWRUPACK 置位）
- ✅ 所有 DPACC 操作返回 ACK = 0x2（OK）

## 参考文档

- ARM Debug Interface Architecture Specification ADIv5
- ICEPick-C Functional Specification
- TMS570LC43xx Technical Reference Manual
- JTAG IEEE 1149.1 Standard

## 版本历史

- **v1.0** (2025-11-28): 实现 ICEPick 旁路位支持（方案 1）

