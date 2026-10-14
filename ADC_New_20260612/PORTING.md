# 产品测试模块适配说明

适配基准是 `ADC_New_20260612` 的现有驱动、引脚和 Modbus 入口。旧 `ADC` 的产品测试流程移入新工程，保留原产品结果的寄存器偏移。

## 文件与接口

- `Core/Src/wf280.c` / `Core/Inc/wf280.h`：`WF280CheckOtpReg()`，至少 2 个结果寄存器。
- `Core/Src/wf183.c` / `Core/Inc/wf183.h`：`AllTestFor183()`，至少 118 个结果寄存器。
- `Core/Src/wf2200.c` / `Core/Inc/wf2200.h`：`AllTestFor2200()`，至少 77 个结果寄存器；`WF3050FtData()`，至少 12 个。
- `Core/Src/wf100s.c` / `Core/Inc/wf100s.h`：`AllTestFor100S()`，至少 16 个结果寄存器。
- 补入 `I2CHelper.c/.h`、`SoftIIC_PP.c/.h`。辅助驱动使用新版 `I2CInfo *` 接口，不引入旧版同名但不兼容的 I2C 结构体或全局 I2C 驱动。
- 6 个新增 C 文件均已加入 `MDK-ARM/ADC.uvprojx`，头文件统一位于 `Core/Inc`。

## 新板硬件适配

| 功能 | 实际调用/引脚 |
| --- | --- |
| VDD | 新版 `MCP4725_WriteData_Voltage()`；不再访问旧板 `PC6/PWR` |
| 产品 I2C | 新版 `SoftIIC`，`PB4/I2C_SDA`、`PB5/I2C_SCL` |
| 产品 UART | 新版 `SoftUart`，发送 `PB6/UART_TX`、接收 `PB7/UART_RX` |
| OWI | 新版 `ParseOwiCmd()` / `owi.c`，`PA0` |
| ADS1220 | 新版 `GET_ADS1220_Val()` / `ADS1220_WriteRegister()` |
| WF3050 参考传感器 | 推挽 I2C，SDA=`PB7`、SCL=`PB6`；测试结束释放为模拟输入，后续 UART 会重新初始化 |
| VPP | 保留新版 `PB3/VPP_EN` 及 `0x202` 命令 |

**WF100S 的模拟供电请求 5000 mV，其他型号使用 3300 mV。** `CmdHelper.h` 的 `WF100S_TEST_VDD_MV` 用于 WF100S，`PRODUCT_TEST_VDD_MV` 用于 WF183、WF2200 和 WF3050。`MCP4725_Init()` 默认请求 3300 mV，WF280 按主机设置的 VDD 校验，使用时设置 3300 mV。关电请求为 **0 mV**。UART/I2C 的 GPIO 配置继续使用新板接口；5 V 指 WF100S 的模拟供电。

旧版 `{0x201, 0xFF}` 开关命令已经转换为新版 `{0x201, 电压低字节, 电压高字节}`：3300 mV 为 `{1, 0x201, 0xE4, 0x0C}`，5000 mV 为 `{1, 0x201, 0x88, 0x13}`。

产品内部 I2C 向量使用新版格式：`{3, reserved, send_len, reserved, recv_len, reserved, address, payload...}`。`send_len` 包含地址字节，地址使用原来的 8 位总线地址（`0xDA`、`0x70`）。`ProductI2CTransfer()` 使用新版底层驱动，并保留产品结果中“数据从偏移 0 开始，紧随其后为状态”的布局；单独读取不会额外发送地址写事务。对外通用 I2C 命令仍先写后读，`out[0]` 为写状态、`out[1]` 为读状态、数据从 `out[2]` 开始，0 表示成功、1 表示 NACK。

## Modbus 调用

保留新版从站地址 **1**、115200、8N1、保持寄存器起点 **40001**、输入寄存器起点 **30001** 和 120 个结果寄存器。请求仍经 `FreeModbus/Modbus.c` 调用 `Parse()`。

| 保持寄存器请求内容 | 行为 |
| --- | --- |
| `{8}` | 请求 WF183 测试 |
| `{9}` | 请求 WF100S 测试 |
| `{10}` | 请求 WF2200 测试 |
| `{1, 0x400}` | `out[0]`：1=排队/执行中，0=空闲/已完成 |
| `{1, 0x401}` | 已完成时返回独立保存的 120 寄存器产品结果 |
| `{11}` | 同步执行 WF280 OTP 校验；返回 `{1, 0}` 成功，`{0xFF, 出错位置(1..32)}` 失败，包括通信无响应 |
| `{1, 0x567}` | 同步执行 WF3050 对比采集；两组各 6 寄存器：状态 + 5 字节数据 |

8/9/10 在独立 `ProductTestTask` 执行，提交成功返回 `out[0]=1`。主机随后查询 `{1, 0x400}`，直到状态为 0，再发送 `{1, 0x401}` 并读取输入寄存器。执行期间仅允许状态和结果查询；重入测试或通用外设命令返回 `0xF1`，未完成的结果查询也返回 `0xF1`。请求长度、接收长度或输出容量不匹配返回 `0xF2`。

WF280 沿用原接口，由主机先设置所需 VDD，再执行 OTP 校验；该接口不会自动改变 VDD。WF3050 和另外三种完整测试会在结束时请求 0 mV。

测试任务在新版 TIM1/TIM2、ADS1220、MCP4725 和 Modbus 初始化完成后才执行。产品任务和 Modbus 任务栈各为 256 个 32 位字；FreeRTOS 堆增加为 4096 字节。新增忙状态避免 Modbus 和产品任务并发操作相同驱动。启动时检查任务创建结果。

WF100S 测试完成后恢复原 ADS1220 配置寄存器 0。另修复了新版 UART TX/RX 参数顺序、无响应时波特率除零、位宽测量截断和采样计数溢出，以及 I2C 读 NACK 返回成功的问题。ADS1220 Flash 恢复按结构体实际长度读取；保存使用对齐的两个 64 位字，避免原来的 20 字节读取/16 个双字写入越过结构体。

## 验证与范围

- 使用 STM32CubeIDE 随附的 ARM GCC 13.3.1 对 Keil 列表中的 71 个 C 编译单元进行编译检查（Keil 汇编形式的 FreeRTOS port.c 单独处理），新增和修改的业务/通信文件无编译警告。
- 完整 GNU 链接验证使用与项目内核匹配的 FreeRTOS V10.3.1 GNU Cortex-M0 port、项目附带的 GNU 启动文件和临时链接脚本；将 assert 映射为嵌入式停机处理。Flash 为 64 KB、RAM 为 8 KB，包含 1 KB 主栈和 512 字节运行库堆预留，链接通过。GNU 验证文件仅放在临时目录，不替换原 Keil 启动文件或 RTOS port。
- 运行 `python3 tests/test_port.py` 可复现主机回归测试，使用 AddressSanitizer/UndefinedBehaviorSanitizer 检查真实产品代码的命令、结果布局、异步入口、边界、供电结束、配置恢复、OTP 不一致/NACK；同时验证真实 I2C/推挽 I2C/UART 驱动的无响应路径。硬件外设在该测试中模拟。
- 没有重新生成原 `MDK-ARM/ADC/ADC.hex`；它仍是历史固件。交付前应在 Keil 对 `ADC.uvprojx` 执行 Rebuild，再烧录新 HEX。
- 未连接实板。实际 VDD 输出、参考传感器推挽时序、UART 波特率，以及 WF183 沿用的 120 Ω 电流换算/20 mA 阈值，需按新板原理图和实测确认。片上 ADC1/DMA 的 VDD 反馈闭环保持新版现有状态，不在本次产品接口移植中新增。
