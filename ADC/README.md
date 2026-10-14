# ADC 旧版多通道终测固件

## 1. 工程定位

`ADC` 是当前目录中的旧版、功能较完整的终测板固件。它以 STM32G031G8Ux 为主控，通过 RS485/Modbus RTU 接收上位机命令，驱动 ADS1220 外部 ADC、产品侧 UART、软件 I2C 和 OWI，执行 183、100S、2200 等品类的采集流程。

工程还保留了 280 的 OTP 校验和 3050 参考/被测传感器数据采集代码，但这些功能没有全部接入同一套产品测试状态机。

## 2. 软件和硬件组成

| 项目 | 代码中的配置 |
| --- | --- |
| MCU | STM32G031G8Ux，Cortex-M0+ |
| 工具链 | Keil uVision，ARM Compiler 5；工程文件 `MDK-ARM/ADC.uvprojx` |
| HAL/RTOS | STM32 HAL、FreeRTOS、CMSIS-RTOS v1 |
| 主机接口 | USART2 硬件 RS485，115200，8N1 |
| 外部 ADC | ADS1220，SPI1，`PB0/INT` 等待转换完成 |
| 产品 UART | USART1，默认 9600；183 的代码会先探测实际位宽 |
| 产品 I2C | `PA12/SDA`、`PA11/SCL` 软件 I2C；另有一套推挽软件 I2C 给 2200/3050 使用 |
| OWI | `PA0` |
| 被测件供电 | `PC6/PWR` |
| 地址选择 | `PA15/PB3/PB4/PB5/PB8` 五个上拉输入 |
| 用户指示 | `PB1/LED` |

具体引脚和外设来源于 `ADC.ioc`、`Core/Inc/main.h` 和生成的 `gpio.c/usart.c`。

## 3. 启动和任务关系

入口在 `Core/Src/main.c`：

1. `HAL_Init()` 和系统时钟初始化。
2. 初始化 GPIO、SPI1、USART1/2、TIM1/2/3、独立看门狗。
3. 调用 `MX_FREERTOS_Init()`，启动调度器。

FreeRTOS 中有三个任务，定义在 `Core/Src/app_freertos.c`：

- `StartDefaultTask`：每 500 ms 改变 LED 状态，同时刷新 IWDG。
- `ModbusTask`：启动 TIM1，读取 Flash 中保存的 ADS1220 配置，初始化 ADS1220、软件 I2C 和 Modbus RTU，从站地址固定为 `31`，持续调用 `eMBPoll()`。
- `Sync183Task`：启动 TIM2，按 `IsTask_mub` 选择 183、100S 或 2200 测试；收到任务触发后将结果写入 `sendBufBy`。

上位机写入保持寄存器后，Modbus 回调会调用 `Parse()`；`Parse()` 根据 `cmd[0]` 选择系统、UART、I2C、OWI 或产品测试命令。

## 4. Modbus 数据区

实现位于 `Core/Src/app_freertos.c`，协议栈位于 `Middlewares/Third_Party/FreeModbus`。

| 区域 | 起始地址 | 长度 | 用途 |
| --- | ---: | ---: | --- |
| Input Register | 30001 | 120 个 16 位寄存器 | 读取 `sendBuf` 中的结果 |
| Holding Register | 40001 | 64 个 16 位寄存器 | 写入 `recvBuf`，触发 `Parse()`；也可读回 |

寄存器地址在代码中使用“完整地址”（例如 30001、40001），并且回调要求请求地址等于起始地址。每个寄存器以高字节在前、低字节在后的方式传输；测试代码把 32 位原始 ADC 值拆成四个 16 位寄存器槽位时，实际低 8 位先放入低地址槽位，读取端需要按源码约定重新拼接。

## 5. 命令格式

### 5.1 系统命令 `cmd[0] == 1`

`cmd[1]` 是命令字，定义在 `Core/Inc/CmdHelper.h`：

| 命令 | 含义 | 备注 |
| ---: | --- | --- |
| `0x104` | 获取固件版本 | 当前版本 `{0x01, 0x09}` |
| `0x105` | 读取 ADS1220 原始值 | 返回 32 位有符号 ADC 数据 |
| `0x107` | 设置 ADS1220 配置 | `cmd[2..5]` 写入 4 个配置寄存器，不保存 |
| `0x108` | 读取 ADS1220 配置 | 返回 4 个配置字节 |
| `0x200` | 设置并保存 ADS1220 配置 | 写入用户 Flash 页 |
| `0x201` | 打开/关闭被测件供电 | `0xFF` 打开，其他值关闭 |
| `0x400` | 查询产品测试状态 | 返回 `IsTaskDone` |
| `0x401` | 读取批量测试结果 | 拷贝 `sendBufBy` |
| `0x567` | 读取 WF3050 FT 数据 | 执行参考/被测件读取流程 |

### 5.2 通用产品 UART `cmd[0] == 2`

格式为 `type, 0, sendLen, sendTimeout, recvLen, recvTimeout, payload...`。超时单位按代码是毫秒；返回数据前两个槽位表示测得的 UART 位宽换算值，后续槽位是接收数据。

### 5.3 通用 I2C `cmd[0] == 3`

代码注释约定格式为 `type, channel, read/write, busAddr, startReg, dataLen, data...`，其中 `0x11` 表示写、`0x22` 表示读。2200 专用流程使用 7 位地址 `0x6D`（读写字节分别为 `0xDA` 等）。

### 5.4 OWI `cmd[0] == 5`

`ParseProductCmd()` 连续读取两次 OWI，返回时序周期以及 3 个字节。`cmd[0] == 8/9/10` 分别触发 183、100S、2200 批量测试，`cmd[0] == 11` 触发 280 OTP 对比。

## 6. 各产品流程

- **183 (`wf183.c`)**：打开供电，先用 ADS1220 计算电流并以 `20 mA` 为短路判断阈值；随后读取 UART 的 ID、版本、温度/气压标定系数和数据，读取 I2C 温度/气压标定数据，读取 OWI 压力数据，再读取工作电流，最后关闭供电。结果按固定槽位写入 120 寄存器缓冲。
- **100S (`wf100s.c`)**：切换 ADS1220 的 MUX/增益配置，依次采集四组原始值，代码注释标注为电流/电压类测量；流程结束后关闭供电。
- **2200 (`wf2200.c`)**：通过 I2C 地址 `0xDA` 复位器件，循环读取 3 个器件的寄存器、OTP、温度/压力数据，同时采集待机和工作电流。`WF3050FtData()` 还使用另一套推挽 I2C 读取参考件和被测件。
- **280 (`wf280.c`)**：逐项读取 `0x70` 器件的两段 OTP，并比较镜像值，返回成功或首个不一致索引。它没有被 `Sync183Task` 的 8/9/10 分支调用。

ADS1220 的四个配置字节由 `flash.c` 写入 STM32 用户 Flash，启动时在 `ModbusTask` 中恢复。

## 7. 建议的阅读顺序

1. `Core/Src/main.c`：看启动、时钟和外设初始化。
2. `Core/Src/app_freertos.c`：看任务、Modbus 地址和回调。
3. `Core/Src/CmdHelper.c`：看命令分发和协议数据布局。
4. `Core/Src/wf183.c`、`wf100s.c`、`wf2200.c`、`wf280.c`：看品类测试步骤。
5. `Core/Src/ADS1220.c`、`I2CHelper.c`、`SoftIIC_*.c`、`owi.c`、`SoftUart.c`：看底层时序。

## 8. 当前代码需要特别确认的地方

- 代码能生成已有的 `MDK-ARM/ADC/ADC.hex`，但本目录没有硬件测试记录；电流换算、结果槽位和每种传感器返回字节的业务含义仍需结合硬件说明确认。
- `WF280CheckOtpReg()` 只有 OTP 校验，项目申请中的 280 供电模式、温度、气压和工作/待机电流并未在本工程形成完整流程。
- 项目申请中的 162A 流程没有对应源码。
- `Parse()` 中 `case 6` 没有 `break`，会继续执行 `case 7`；如果两种 I2C 初始化不是有意叠加，应补充确认。
- `IsTaskDone`、`IsTask_mub` 与结果缓冲由不同任务/Modbus 回调共享，当前没有显式互斥或临界区，硬件高并发场景需要验证。

## 9. 构建

在安装 STM32G0 Keil Device Pack 和 ARM Compiler 5 后，打开 `MDK-ARM/ADC.uvprojx` 构建。`MDK-ARM/ADC/ADC.build_log.htm` 记录的历史构建为 0 error、0 warning；当前说明基于源码静态梳理，未在本环境重新调用 Keil 构建。
