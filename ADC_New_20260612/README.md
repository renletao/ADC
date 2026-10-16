# ADC New 20260612 固件

本工程以 STM32G031G8Ux、ADS1220、FreeRTOS 和 FreeModbus 为基础，使用新版 PCB 的 MCP4725 VDD 输出、软件 UART/I2C、OWI、ADC1 和 DMA 配置。现已移入旧 `ADC` 的 WF183、WF2200、WF100S 测试、WF162A 双供电测试和 WF280 OS 测试，并按本工程的驱动接口适配。

完整移植接口、请求示例、结果布局和验证范围见 [PORTING.md](PORTING.md)。

## 硬件与工具链

| 项目 | 配置 |
| --- | --- |
| MCU | STM32G031G8Ux，Cortex-M0+，64 KB Flash / 8 KB RAM |
| 主工程 | Keil uVision / ARM Compiler 5，`MDK-ARM/ADC.uvprojx` |
| 主机接口 | USART2 RS485，115200、8N1、Modbus 从站地址 1 |
| ADS1220 | SPI1，PB0/INT；读取接口 `GET_ADS1220_Val()` |
| MCP4725 | PA12/DAC_SDA、PA11/DAC_SCL；电压参数单位 mV |
| 产品 I2C | PB4/I2C_SDA、PB5/I2C_SCL，软件开漏 I2C |
| 产品 UART | PB6/UART_TX、PB7/UART_RX，软件 UART |
| 参考传感器 I2C | PB7/SDA、PB6/SCL，补入的推挽 I2C 驱动 |
| VPP / OWI / LED | PB3 / PA0 / PA8 |
| 片上 ADC1 | PB1/VDD_FB，通道 9，12 位、16 倍过采样；DMA1 Channel 1 |

## 启动与任务

`main.c` 初始化 GPIO、DMA、SPI1、USART2、TIM1/2/3、ADC1 并启动 RTOS。新版独立看门狗仍未启用。

- `StartDefaultTask`：LED 指示。
- `ModbusTask`：启动 TIM1/TIM2，恢复 ADS1220 配置，初始化 ADS1220、MCP4725 和 Modbus，然后轮询。
- `ProductTestTask`：等待上述驱动准备完成，执行请求的 WF183 / WF100S / WF2200 / WF162A 测试，保存结果并更新忙状态。

产品测试期间仅允许查询状态和结果，其他外设命令返回忙状态，避免两个任务同时操作相同 GPIO、定时器和 ADC 驱动。

## 主机命令

保持寄存器起点为 40001，长度 64；输入寄存器起点为 30001，长度 120。写入后由 `Middlewares/Third_Party/FreeModbus/Modbus.c` 调用 `Parse()`，输入寄存器返回命令结果。地址处理沿用新版实现，从各区域起点开始读写。

系统命令的 `cmd[0]` 为 1：

| `cmd[1]` | 功能 |
| --- | --- |
| `0x104` | 版本 `{0x01, 0x00}` |
| `0x105` | ADS1220 原始值，4 个低字节在前的字节寄存器 |
| `0x107` / `0x108` / `0x200` | 设置 / 读取 / 保存 ADS1220 配置 |
| `0x201` | 设置 VDD，`cmd[2]` 为 mV 低字节、`cmd[3]` 为高字节 |
| `0x202` | 控制 VPP，`cmd[2]` 非零置高 PB3 |
| `0x400` | 产品任务状态：1=排队/执行中，0=空闲/已完成 |
| `0x401` | 读取已完成的产品测试结果；执行中返回 `0xF1` |
| `0x567` | WF3050 参考与被测传感器对比采集 |

其他 `cmd[0]`：2=软件 UART、3=软件 I2C、5=OWI、8=请求 WF183 测试、9=请求 WF100S 测试、10=请求 WF2200 测试、11=WF280 OTP 校验、12=请求 WF162A 测试。8/9/10/12 异步执行；11 和 0x567 同步执行。系统命令 `0x207/0x208/0x209/0x210` 提供 WF280 的 M1 电流、3.3 V/2.0 V 系数及温压原始数据。

软件 UART/I2C 沿用新版命令向量和数据偏移；I2C 同时兼容旧 WF 测试软件的 `0x11` 写、`0x22` 读帧，避免旧帧被误报为 `0xF2` 长度错误。串口收到首帧无响应时会重新初始化 GPIO 并自动重试一次。协议细节见移植说明。长度错误返回 `0xF2`，测试期间的冲突命令返回 `0xF1`。新版 VDD/VPP 命令含义与旧板不同，旧版 VDD 开关请求不能直接用于新板。

软件 I2C 已修复 ACK/STOP 时序及 SDA 交接，开漏总线支持有限时钟拉伸等待和卡住恢复。驱动验证方法、外部上拉要求和推挽参考总线的使用范围见 [SOFT_I2C.md](SOFT_I2C.md)。

## 硬件验证与构建

WF100S 模拟供电请求 5000 mV，WF162A 依次执行 3300 mV 和 5000 mV 两轮采集，WF280 的 `0x208/0x209` 使用 3300 mV、`0x207/0x210` 使用 2000 mV；其他型号使用 3300 mV。MCP4725 初始化保持断电（0 mV），每个产品流程显式设置供电。产品供电分别配置在 `CmdHelper.h` 的 `WF100S_TEST_VDD_MV` 和 `PRODUCT_TEST_VDD_MV`，供电关闭请求为 0 mV。实际输出仍需结合 DAC 参考电压和电源电路实测。WF183 的电流换算保留原 120 Ω / 20 mA 判定，需结合新板原理图确认。

ADC1/DMA 的配置保持新版状态：尚未接入采样缓冲、`HAL_ADC_Start_DMA()` 或 VDD 反馈闭环。

已进行 ARM GCC 编译、临时 GNU 完整链接和主机回归测试。可通过 `python3 tests/test_port.py` 重跑模拟硬件的回归测试。尚未连接实板，也未在 Keil 重建工程。原 `MDK-ARM/ADC/ADC.hex` 是历史固件；烧录前请对 `ADC.uvprojx` 执行 Rebuild 并使用新生成的 HEX。
