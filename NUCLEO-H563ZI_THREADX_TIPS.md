# ThreadX + TrustZone 模板配置经验

> **准备开始新项目？** 先查看[新项目复制与命名流程](./New_project_naming.md)，
> 按流程复制模板并同步目录名、Makefile 目标名和调试配置。

本文从 USBX HID 移植项目的复盘中提取仅适用于本模板的 RTOS、ThreadX
和 TrustZone 配置要求。USBX HID、USB-PD/CAD、UCPD 附着检测和 USB 线材排查
属于具体应用，不是本模板的基础配置，因此不在本文展开。

> **工具链、250MHz 时钟树、GPIO/MSP、中断、OpenOCD/GDB 调试和裸机 Makefile
> 搭建等板级通用内容请查《NUCLEO-H563ZI 裸机模板 TIPS》**
> （`NUCLEO-H563ZI_TEMPLATE/NUCLEO-H563ZI_TIPS.md`）。本文只写 ThreadX 特有、
> 裸机 TIPS 没有覆盖的部分，两份配合使用。

## 1. 先确认 TrustZone 执行状态，再选 ThreadX 模式

- TrustZone 已启用（例如 OpenOCD 显示 `TZEN`）不等于应用正在 Non-Secure
  状态。应确认 CPU 复位后的实际安全状态，以及启动代码是否执行了
  Secure-to-Non-Secure 跳转。
- 本模板针对已验证的 NUCLEO-H563ZI Secure 启动状态，默认使用
  `THREADX_MODE=SECURE`，对应 `TX_SINGLE_MODE_SECURE=1`。
- Makefile 将安全模式宏用于 ThreadX C 内核和 Cortex-M33 GNU 汇编端口。
  不应只对应用代码设置此宏。
- Secure/Non-Secure 模式不匹配可能导致 ThreadX 第一次上下文切换时出现
  无效异常返回（`CFSR.INVPC`）并进入 HardFault。编译、链接和烧录校验成功
  均不能证明安全状态配置正确。
- `THREADX_MODE=NON_SECURE` 仅切换 ThreadX 编译模式；它不会生成 Secure
  启动程序、SAU/IDAU 配置、Non-Secure 向量表或存储器/外设安全属性。
  只有工程另行实现并验证完整 Secure-to-Non-Secure 启动链后，才可使用。

## 2. ThreadX 端口与启动源必须完整且版本匹配

本模板引用同一 STM32Cube_FW_H5 包内的 ThreadX common 和 Cortex-M33 GNU
port；ThreadX 低层初始化汇编位于本工程 `Core/Src/tx_initialize_low_level.S`。
移植或更换 Cube 固件包时，保持内核、端口和低层初始化来自兼容版本。

ThreadX 端口不只是几个 C 文件。构建必须包含上下文保存/恢复、调度、线程
栈构造、系统返回、计时中断和低层初始化所需的汇编文件。遗漏时可能链接
失败，也可能使调度或首次上下文切换异常。

一个能跑的 ThreadX 工程，Makefile 必须包含以下三类源：

- **内核 C 源码**：`$(THREADX_DIR)/common/src/*.c` 全部（mutex/semaphore/
  queue/event_flags/block_pool/byte_pool/timer/trace 等），本模板直接
  `wildcard` 引入。
- **Cortex-M33 GNU 端口汇编**（`$(THREADX_DIR)/ports/cortex_m33/gnu/src/`）：
  `tx_misra.S`、`tx_thread_context_restore.S`、`tx_thread_context_save.S`、
  `tx_thread_interrupt_control.S`、`tx_thread_interrupt_disable.S`、
  `tx_thread_interrupt_restore.S`、`tx_thread_schedule.S`、
  `tx_thread_stack_build.S`、`tx_thread_system_return.S`、
  `tx_timer_interrupt.S`。
- **本工程低层初始化**：`Core/Src/tx_initialize_low_level.S`（已按 250MHz
  定制）。不要再加 Cube 端口目录里自带的 `tx_initialize_low_level.S`，
  它的时钟常量与本工程主频不一致。

端口目录里的 `tx_thread_secure_stack_*.{c,S}` 用于 TrustZone 双模式，
`TX_SINGLE_MODE_SECURE=1` 下**不要**加入构建。

端口汇编拥有 ThreadX 的关键异常处理函数：低层初始化提供 ThreadX 的
`SysTick_Handler`，Cortex-M33 端口提供 `PendSV_Handler` 和
`SVC_Handler`。不要再在 `stm32h5xx_it.c` 或应用中定义同名处理函数。

## 3. ThreadX 和 HAL 使用不同硬件时基

- 本模板的 ThreadX tick 为 **100 ticks/s**，由 ThreadX 端口使用 SysTick。
- HAL 的 **1 ms tick** 通过 TIM6 驱动。HAL 的时间基文件、TIM6 MSP 时钟使能、
  NVIC 配置和 `TIM6_IRQHandler` 必须一起加入构建。
- 不要让 HAL 和 ThreadX 同时配置或接管 SysTick，否则会导致 tick 计数、
  延时或调度异常。
- ThreadX 低层初始化中的 `SYSTEM_CLOCK`/`SYSTICK_CYCLES`、`tx_user.h` 中的
  `TX_TIMER_TICKS_PER_SECOND`、主时钟配置和应用中的 `tx_thread_sleep()`
  换算必须一致。若改变 CPU 主频或 ThreadX tick 频率，应一并更新并验证。
  换算公式为 `SYSTICK_CYCLES = SYSTEM_CLOCK / TX_TIMER_TICKS_PER_SECOND - 1`；
  本模板即 `250000000 / 100 - 1 = 2499999`（`SYSTEM_CLOCK=250000000` 在
  `tx_initialize_low_level.S`，`TX_TIMER_TICKS_PER_SECOND=100` 在 `tx_user.h`）。

## 4. 内存策略要与应用和链接布局一致

- 本模板的示例线程控制块和栈是静态分配的；没有创建动态 ThreadX byte pool
  或启用动态 ThreadX heap 的要求。
- 若应用改为动态分配，需显式规划 ThreadX 可用内存并核对链接脚本、RAM
  区域、安全属性和启动时传入的未使用内存边界。
- 添加静态线程、队列、栈或其他中间件对象时，确认链接脚本中的 RAM 空间
  足够；不要因为已有静态对象就假设需要启用动态 heap。
- 在 `tx_application_define()` 或线程入口中创建/使用 ThreadX 对象。HAL、
  GPIO 等板级初始化应在进入 `tx_kernel_enter()` 前完成，避免在内核定义
  回调中依赖尚未启动的调度行为。

## 5. 变更配置后干净构建，并分层验证运行

更改 `THREADX_MODE`、ThreadX 用户配置、端口宏、低层初始化时钟常量或汇编
端口后，执行干净构建，避免旧对象文件保留先前的编译定义：

```sh
make clean
make -j4
```

建议按以下顺序验证：

1. 编译与链接通过，检查输出中包含 ThreadX 低层初始化、SysTick、PendSV、
   `tx_application_define`、应用入口和所需 HAL MSP/IRQ 符号。
2. 烧录后先确认没有在第一次线程切换处 HardFault；LED 心跳可作为示例应用
   线程已运行的外部迹象。
3. 调试时检查 `CFSR`/`HFSR` 和异常栈帧。若有 `INVPC`，优先核对实际 Secure/
   Non-Secure 执行状态、ThreadX 模式宏和对应的端口/启动链。
4. 检查 ThreadX tick 和线程状态。tick 增长只证明内核计时路径在运行，不
   单独证明目标应用线程已就绪或正在执行。
5. 若后续集成 USB、通信或其他外设，再沿各自的初始化和事件链检查；不要
   把外设未启动直接归因于 ThreadX 调度。

运行时观测（可替代/补充断点调试）：

- **串口**：示例线程用 `printf` 经 ST-LINK 虚拟串口（USART3，115200 8-N-1）
  输出 `** ThreadXXX : ... **`，是最直接的运行证据。读取命令：

  ```sh
  stty -F /dev/ttyACM0 115200 cs8 -cstopb -parenb
  cat /dev/ttyACM0            # Ctrl+C 退出
  ```

- **OpenOCD 读内核 tick**：先 `arm-none-eabi-nm <TARGET>.elf | grep _tx_timer_tick`
  查地址，再暂停目标读两次，递增说明 SysTick 计时路径在跑：

  ```sh
  openocd -s . -f openocd/st_nucleo_h5.cfg \
    -c "init" -c "halt" \
    -c "echo [format 0x%08x [mrw 0x2000XXXX]]"
  ```

  注意：tick 递增只证明内核计时在跑；要确认目标线程已就绪/在执行，还需
  结合串口输出或线程状态（`tx_thread_state`）一起看。

## 6. 本模板的配置位置

| 配置内容 | 模板位置 |
| --- | --- |
| ThreadX 安全模式选择、内核/端口源列表 | `Makefile` |
| ThreadX tick 频率和内核配置 | `Core/Inc/tx_user.h` |
| ThreadX SysTick 及低层上下文初始化 | `Core/Src/tx_initialize_low_level.S` |
| ThreadX 线程与静态栈示例 | `Core/Src/app_threadx.c` |
| HAL TIM6 时基 | `Core/Src/stm32h5xx_hal_timebase_tim.c` |
| TIM6 MSP 与 IRQ | `Core/Src/stm32h5xx_hal_msp.c`、`Core/Src/stm32h5xx_it.c` |
| 主时钟、GPIO 初始化及内核入口 | `Core/Src/main.c` |
| Flash/RAM 布局 | `STM32H563ZITX_FLASH.ld` |

本指南记录的是模板的通用 ThreadX/TrustZone 约束；项目级原始移植过程和
USBX 专项经验仍保留在 USBX HID 工程的 `report.md` 与
`NUCLEO-H563ZI_TIPS.md` 中。
