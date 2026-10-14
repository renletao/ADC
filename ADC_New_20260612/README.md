# ADC New 20260612 基础采集板固件

## 1. 工程定位

`ADC_New_20260612` 是旧版 `ADC` 的硬件适配/重构版本。它仍以 STM32G031G8Ux、ADS1220、FreeRTOS 和 FreeModbus 为基础，但去掉了旧版的 183/100S/2200/280 产品测试状态机，改为保留通用采集与通信能力，并新增 MCP4725 电压输出、片上 ADC1 和 DMA 配置。

因此它更像“新 PCB 的基础固件骨架”，不能直接视为项目申请中所有品类功能的完成版。

## 2. 软件和硬件组成

| 项目 | 代码中的配置 |
| --- | --- |
| MCU | STM32G031G8Ux，Cortex-M0+ |
| 工具链 | Keil uVision，ARM Compiler 5；工程文件 `MDK-ARM/ADC.uvprojx` |
| HAL/RTOS | STM32 HAL、FreeRTOS、CMSIS-RTOS v1 |
| 主机接口 | USART2 硬件 RS485，115200，8N1，Modbus 从站地址 `1` |
| 外部 ADC | ADS1220，SPI1，`PB0/INT` 等待转换完成 |
| DAC | MCP4725，`PA12/DAC_SDA`、`PA11/DAC_SCL` 软件 I2C |
| 产品 I2C | `PB4/I2C_SDA`、`PB5/I2C_SCL` 软件 I2C |
| 产品 UART | `PB6/UART_TX`、`PB7/UART_RX`，由 `SoftUart.c` 重新配置 GPIO bit-bang |
| 片上 ADC | ADC1 通道 9，输入 `PB1/VDD_FB`，12 位、连续转换、16 倍过采样 |
| DMA | DMA1 Channel 1，ADC1 到内存，循环模式 |
| VPP 控制 | `PB3/VPP_EN` |
| OWI | `PA0` |
| 用户指示 | `PA8/LED` |

引脚来源于 `ADC.ioc`、`Core/Inc/main.h` 和 `Core/Src/gpio.c`。

## 3. 启动和任务关系

入口在 `Core/Src/main.c`：

1. 初始化 HAL、系统时钟、GPIO、DMA、SPI1、USART2、TIM1/2/3 和 ADC1。
2. `MX_IWDG_Init()` 被注释掉，当前启动流程不启用独立看门狗。
3. 启动 FreeRTOS。

`Core/Src/app_freertos.c` 创建两个任务：

- `StartDefaultTask`：每 500 ms 改变 LED 状态；刷新看门狗的代码已注释。
- `ModbusTask`：启动 TIM1/TIM2，从 Flash 恢复 ADS1220 配置，初始化 ADS1220 和 MCP4725，初始化 Modbus RTU 并持续轮询。

Modbus 寄存器回调不再放在 `app_freertos.c`，而是放在 `Middlewares/Third_Party/FreeModbus/Modbus.c`。写保持寄存器后仍会调用 `Parse()`。

## 4. Modbus 数据区

| 区域 | 起始地址 | 长度 | 用途 |
| --- | ---: | ---: | --- |
| Input Register | 30001 | 120 个 16 位寄存器 | 读取 `sendBuf` |
| Holding Register | 40001 | 64 个 16 位寄存器 | 写入 `recvBuf`，触发 `Parse()`；也可读回 |

协议参数在 `app_freertos.c` 中为从站地址 `1`、115200、无校验。地址回调使用完整地址并要求请求地址等于起始地址。

## 5. 当前可用命令

命令常量在 `Core/Inc/CmdHelper.h`，分发在 `Core/Src/CmdHelper.c`。

### 系统命令 `cmd[0] == 1`

| 命令 | 含义 | 当前行为 |
| ---: | --- | --- |
| `0x104` | 获取固件版本 | 返回 `{0x01, 0x00}` |
| `0x105` | 获取 ADS1220 原始值 | 返回 32 位转换值 |
| `0x107` | 设置 ADS1220 配置 | 写入 `cmd[2..5]`，不保存 |
| `0x108` | 读取 ADS1220 配置 | 返回 4 个配置字节 |
| `0x200` | 保存 ADS1220 配置 | 写入用户 Flash |
| `0x201` | 设置 VDD 电压 | 将 `cmd[2]`、`cmd[3]` 合成为 mV，调用 `MCP4725_WriteData_Voltage()` |
| `0x202` | 控制 VPP | `cmd[2]` 非 0 置高 `VPP_EN`，并返回引脚状态 |

### 通用通信命令

- `cmd[0] == 2`：软件 UART。格式为发送长度、发送超时、接收长度、接收超时和 payload；默认先以 9600 的位宽工作并测量实际位宽。
- `cmd[0] == 3`：软件 I2C。先发送 `cmd[6...]`，再把 `cmd[6]` 当作读地址读取指定长度；结果从 `out[2]` 开始。
- `cmd[0] == 5`：OWI。读取两个周期，返回周期计数和 3 字节数据。

头文件仍声明了 `ParseProductCmd()`，但新版实际使用的是 `ParseOwiCmd()`；这属于遗留声明，不代表新版已经有旧版的产品测试流程。

## 6. 新增模块的实际状态

### MCP4725

`MCP4725_Init()` 初始化软件 I2C，并立即请求输出 `5000 mV`。`MCP4725_WriteData_Voltage()` 用固定 `VREF_3V3 = 3300` 和源码中的 `/2` 公式计算 12 位码值，再发送 3 个字节。代码没有对输入电压做范围限制，也没有校验反馈分压、参考电压和目标输出之间的关系，实际输出需要结合硬件电源和芯片配置确认。

### ADC1/DMA

`Core/Src/adc.c` 已生成 ADC1 通道 9、16 倍过采样和 DMA 循环配置，`Core/Src/dma.c` 已打开 DMA 中断。但是当前源码没有找到 `HAL_ADC_Start_DMA()`、采样缓冲区、DMA 完成回调或把 ADC1 数值放入 `sendBuf` 的代码。`main.c` 中的 `T_AD/P_AD` 等变量也只完成声明，没有参与命令处理。因此“VDD_FB 采样链路”目前是配置完成，业务闭环未完成。

## 7. 与旧版的关系

| 能力 | 旧版 `ADC` | 新版 `ADC_New_20260612` |
| --- | --- | --- |
| ADS1220 SPI 采集 | 有 | 有，读取函数改名为 `GET_ADS1220_Val` |
| FreeModbus RTU | 有，地址 31 | 有，地址 1；回调移到 `Modbus.c` |
| 183/100S/2200 测试 | 有状态机 | 已移除 |
| 280 OTP 校验 | 有 | 已移除 |
| 产品 UART/I2C/OWI 通用命令 | 有 | 有，接口重新映射到新 PCB |
| MCP4725 电压输出 | 无 | 新增 |
| ADC1/DMA VDD 反馈 | 无 | 只完成外设配置，尚未接入业务 |
| 看门狗 | 启用并在默认任务刷新 | 初始化和刷新均被注释 |

## 8. 建议的阅读顺序

1. `Core/Src/main.c`：启动顺序和新增的 ADC/DMA 初始化。
2. `Core/Src/app_freertos.c`：两个任务和 Modbus 参数。
3. `Middlewares/Third_Party/FreeModbus/Modbus.c`：寄存器映射和命令入口。
4. `Core/Src/CmdHelper.c`：系统、UART、I2C、OWI 命令。
5. `Core/Src/ADS1220.c`、`MCP4725.c`、`SoftIIC.c`、`SoftUart.c`、`owi.c`：底层设备驱动。

## 9. 构建和风险

工程包含已有的 `MDK-ARM/ADC/ADC.hex`。`MDK-ARM/ADC/ADC.build_log.htm` 记录的历史构建为 0 error、0 warning，但它对应的是历史快照；本环境没有 Keil，未重新编译当前源码。

在继续开发前，建议优先确认：

- 启动 ADC1 DMA 并定义采样缓冲、换算公式、结果寄存器和异常处理。
- `MCP4725` 的输出范围、I2C 地址和 3.3/5 V 供电关系，尤其是默认 5000 mV 请求。
- 产品 UART/I2C GPIO 在 `gpio.c` 初始状态与 `SoftUart/SoftIIC` 动态重配置之间的切换时序。
- 重新接入项目申请要求的 183、280、162A 测试状态机和短路保护策略。
