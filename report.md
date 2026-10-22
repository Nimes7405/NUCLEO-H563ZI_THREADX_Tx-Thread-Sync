# NUCLEO-H563ZI ThreadX Tx_Thread_Sync 项目复盘

## 目标

依据 ST 的
[STM32CubeH5 `Tx_Thread_Sync` 示例](https://github.com/STMicroelectronics/STM32CubeH5/tree/main/Projects/NUCLEO-H563ZI/Applications/ThreadX/Tx_Thread_Sync)，
在本工作区的手写 Makefile 工程体系中复现「两个同优先级 ThreadX 线程通过
同步对象共享 LED 翻转临界区」的用法，并遵守 NUCLEO-H563ZI Secure ThreadX
模板与新项目命名规则。

上游示例的预期行为：

- 两个同优先级（10）、同时片（5 tick）的线程 **ThreadOne / ThreadTwo**
  按时间片轮转运行。
- 线程进入临界区（LED 翻转）前用 `TX_NO_WAIT` 尝试获取同步对象。
- 获取成功：打印 `** ThreadXXX : SyncObject acquired **`，翻转 LED
  （500 ms × 10 次 = 5 s），释放并打印 `** ThreadXXX : SyncObject released **`，
  随后 `tx_thread_sleep(1)`。
- 获取失败：周期性打印 `** ThreadXXX : waiting for SyncObject !! **`。
- ThreadOne 翻转绿色 LED（LED1/PB0），ThreadTwo 翻转红色 LED（LED3/PG4）。
- 消息经 USART3（115200 8-N-1）从 ST-LINK 虚拟串口输出。

## 结果摘要

- 从 `NUCLEO-H563ZI_THREADX-SECURE_TEMPLATE` 复制出
  `NUCLEO-H563ZI_THREADX_Tx-Thread-Sync`，目录名、Makefile `TARGET` 与
  VS Code ELF 路径保持一致。
- 同步对象通过 [`Core/Inc/app_threadx.h`](./Core/Inc/app_threadx.h) 的
  `APP_SYNC_*` 宏抽象，默认 `TX_MUTEX`，可一键切换为二进制 `TX_SEMAPHORE`。
- `make clean && make -j4` 构建成功，产物：

  | 产物 | 大小 | text / data / bss |
  | --- | --- | --- |
  | `NUCLEO-H563ZI_THREADX_Tx-Thread-Sync.elf` | 384644 B | 51452 / 1592 / 5632 |
  | `NUCLEO-H563ZI_THREADX_Tx-Thread-Sync.bin` | 53044 B | — |

- 链接层面验证通过：`tx_application_define`、`ThreadOne_Entry`、
  `ThreadTwo_Entry`、`_tx_initialize_low_level`、ThreadX 的
  `SysTick_Handler`/`PendSV_Handler`、HAL 的 `TIM6_IRQHandler`、
  `HAL_TIM_Base_MspInit`、`HAL_UART_MspInit` 均已链接为真实实现。
- 板上串口已观察到预期输出序列
  （`ThreadOne acquired → ThreadTwo waiting → ThreadOne released →
  ThreadTwo acquired`），证明双线程轮转调度与互斥同步正常。

## 实现内容

### 线程与同步对象

[`Core/Src/app_threadx.c`](./Core/Src/app_threadx.c) 静态分配两个
`TX_THREAD` 与各 512 字节栈（`TX_APP_STACK_SIZE=512`）。两个线程均用：

- 优先级 `TX_APP_THREAD_PRIO=10`
- 抢占阈值 = 自身优先级（`TX_APP_THREAD_PREEMPTION_THRESHOLD`
  默认取 `TX_APP_THREAD_PRIO`）
- 时间片 `TX_APP_THREAD_TIME_SLICE=5`（100 ticks/s 下即 50 ms 一轮）

相同优先级 + 相同阈值 + 非零时间片 → 调度器按时间片轮转，这是本示例
演示「线程共享临界区」的基础。

`tx_application_define()` 先创建两个线程（`TX_AUTO_START`），再创建同步
对象，任一失败即进入 `Error_Handler()`。

### 同步对象可切换（mutex / semaphore）

[`Core/Inc/app_threadx.h`](./Core/Inc/app_threadx.h) 用宏把同步原语抽象
为 `APP_SYNC_TYPE / APP_SYNC_CREATE / APP_SYNC_GET / APP_SYNC_PUT`：

- 默认 `USE_TX_MUTEX`：`tx_mutex_create(..., TX_NO_INHERIT)` +
  `tx_mutex_get(..., TX_NO_WAIT)` + `tx_mutex_put`。
- 定义 `USE_TX_SEMAPHORE` 改为：`tx_semaphore_create(..., 1)` +
  `tx_semaphore_get(..., TX_NO_WAIT)` + `tx_semaphore_ceiling_put(..., 1)`。
- 两者同时定义会触发 `#error`。

### 临界区与忙等延时

`Led_Toggle()` 在持有同步对象期间执行，先写低、翻转 10 次、最后写低。
延时用 `App_Delay()` 基于 `tx_time_get()` 忙等，**不**用
`tx_thread_sleep()`：sleep 会让出 CPU 但同步对象仍被持有，语义混乱且
无法保证 500 ms 节拍；忙等保持「持有锁 → 独占翻转」的演示意图。

### 等待消息限流

获取失败分支用 `iteration % 2000000U == 0U` 限流打印。若不限流，忙等
循环会以极高频次刷 `waiting` 消息，阻塞式 USART 发送会显著拖慢轮转。

### 时基分工

- ThreadX tick：SysTick 100 ticks/s（`tx_initialize_low_level.S` 中
  `SYSTEM_CLOCK=250000000`、`SYSTICK_CYCLES=(SYSTEM_CLOCK/100)-1`）。
- HAL 1 ms tick：TIM6（`stm32h5xx_hal_timebase_tim.c` +
  `HAL_TIM_Base_MspInit` + `TIM6_IRQHandler`）。
- 两者不共用 SysTick。

### printf 重定向

[`Core/Src/syscalls.c`](./Core/Src/syscalls.c) 的 `_write` 把 stdio 输出
经 `HAL_UART_Transmit(&huart3, ..., HAL_MAX_DELAY)` 阻塞发送到 ST-LINK
虚拟串口。因此 Makefile 的 `HAL_SRCS` 在模板基础上增加了
`stm32h5xx_hal_uart.c` 与 `stm32h5xx_hal_uart_ex.c`（模板不含 UART HAL
源）。

## 构建与链接验证

执行 `make clean && make -j4` 成功。用 `arm-none-eabi-size` 与
`arm-none-eabi-nm` 核对：

| 符号 | 类型 | 说明 |
| --- | --- | --- |
| `_tx_initialize_low_level` | T | ThreadX 低层初始化（含 ThreadX 的 SysTick） |
| `SysTick_Handler` / `PendSV_Handler` | T | ThreadX 端口异常入口 |
| `SVC_Handler` | W | Secure 单模式下启动文件保留的弱符号，预期行为 |
| `tx_application_define` | T | 内核定义回调 |
| `ThreadOne_Entry` / `ThreadTwo_Entry` | T | 两个应用线程入口 |
| `TIM6_IRQHandler` | T | HAL 1 ms 时基中断 |
| `HAL_TIM_Base_MspInit` / `HAL_UART_MspInit` | T | 真实 MSP 实现（非弱空函数） |

`SVC_Handler` 为弱符号（W）在 `TX_SINGLE_MODE_SECURE=1` 单安全模式下是
预期行为，不代表配置错误。

## 运行验证

### 串口输出

消息经 ST-LINK 虚拟串口（USART3，115200 8-N-1）输出，Linux 下设备为
`/dev/ttyACM0`。读取命令：

```sh
stty -F /dev/ttyACM0 115200 cs8 -cstopb -parenb
cat /dev/ttyACM0            # Ctrl+C 退出
```

或使用 `screen /dev/ttyACM0 115200`、`picocom -b 115200 /dev/ttyACM0`。
预期输出序列：

```text
** ThreadOne : SyncObject acquired **
** ThreadTwo : waiting for SyncObject !! **
** ThreadOne : SyncObject released **
** ThreadTwo : SyncObject acquired **
```

`waiting` 行是正常现象：ThreadTwo 在 ThreadOne 持有同步对象的 5 s 内以
`TX_NO_WAIT` 轮询失败并周期性打印，ThreadOne 释放后随即获取。

### 分层检查

构建/烧录/串口任一项成功都不单独证明全部链路正常，按
[`NUCLEO-H563ZI_THREADX_TIPS.md`](./NUCLEO-H563ZI_THREADX_TIPS.md) 第 5 节
分层核对：

1. 编译链接通过、关键符号齐全（上文已核对）。
2. 烧录后无首次线程切换 HardFault；`CFSR`/`HFSR` 无 `INVPC`。
3. LED 心跳与串口输出同时出现，证明线程实际执行。
4. 串口出现 `released → 另一线程 acquired` 顺序，证明互斥语义正确。

## 经验总结

1. **同步对象用 `TX_NO_WAIT` 轮询 + `tx_time_get` 忙等**，而不是在临界区
   内 `tx_thread_sleep`。sleep 会让出 CPU 却仍持有锁，既破坏 500 ms 节拍
   也混淆「持有锁」与「让出调度」的语义。
2. **串口打印必须限流**。忙等分支高频打印会经阻塞式 USART 发送拖慢整个
   轮转，用 `iteration % N` 控制输出频率。
3. **printf 重定向需要两处配套**：`syscalls.c` 的 `_write` 指向 USART3，
   以及 Makefile 加入 `hal_uart*.c` + `HAL_UART_MspInit`。模板不含 UART
   HAL 源，新增串口时必须补上。
4. **ThreadX 与 HAL 时基分离**：SysTick 归 ThreadX（100 ticks/s），HAL
   1 ms tick 走 TIM6，互不争用。
5. **同优先级 + 同抢占阈值 + 非零时间片** 才是时间片轮转的前提；三者任一
   改变都会改变线程调度行为。
6. **`SVC_Handler` 在 Secure 单模式下是弱符号属预期**，不要误判为缺符号；
   但 `SysTick_Handler`/`PendSV_Handler` 必须来自 ThreadX 端口，不得在
   `stm32h5xx_it.c` 重复定义。
7. **文档三层分工**：`README.md` 记用法与串口读取，
   `NUCLEO-H563ZI_THREADX_TIPS.md` 记可复用的 ThreadX/TrustZone 约束，
   本 `report.md` 记本项目实现与验证过程；裸机板级通用内容查
   `NUCLEO-H563ZI_TEMPLATE/NUCLEO-H563ZI_TIPS.md`。
