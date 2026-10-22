# 新建项目：复制与命名流程

开始开发前，先从模板复制出一个独立项目目录，并统一设置目录名和生成物
名称。不要直接在模板目录中开发，也不要只改文件夹名称而遗漏 Makefile
和调试配置。

## 1. 复制模板并确定新项目名

在 `H563ZI_Applications` 下复制整个模板目录。用项目自己的简短名称替换
`<PROJECT>`，目录名建议保持易读并避免空格：

```sh
cd /path/to/H563ZI_Applications
cp -a NUCLEO-H563ZI_THREADX-SECURE_TEMPLATE NUCLEO-H563ZI_<PROJECT>
cd NUCLEO-H563ZI_<PROJECT>
```

例如，新建 `NUCLEO-H563ZI_SENSOR-NODE`：

```sh
cp -a NUCLEO-H563ZI_THREADX-SECURE_TEMPLATE NUCLEO-H563ZI_SENSOR-NODE
cd NUCLEO-H563ZI_SENSOR-NODE
```

复制完整目录可以保留隐藏的 `.vscode/`、`.gitignore` 和 OpenOCD 配置。
确认当前工作目录已经是新项目，不是模板原目录。

## 2. 同步项目名与构建/调试配置

建议新目录名与构建目标同名。在新项目的 `Makefile` 中修改：

```make
TARGET = NUCLEO-H563ZI_SENSOR-NODE
```

然后在 `.vscode/launch.json` 中同步修改 ELF 文件名：

```json
"executable": "${workspaceFolder}/NUCLEO-H563ZI_SENSOR-NODE.elf"
```

`TARGET` 会决定生成的 `.elf`、`.bin` 和 `.map` 文件名；如果只改目录名而
未同步这两处，Makefile 仍会使用模板的产物名称，VS Code 调试配置也可能
指向错误的 ELF。

## 3. 更新项目说明与应用代码

在新项目中更新 `README.md`（从模板复制后可自行新建），至少记录项目用途、
构建/烧录命令和外部依赖路径。然后再按应用需求调整：

- `Core/Src/main.c`：时钟、板级初始化和 `tx_kernel_enter()` 之前的设置。
- `Core/Src/app_threadx.c`：线程、优先级、栈和 ThreadX 对象。
- `Makefile`：HAL 驱动源、应用源文件和新增中间件的源文件/头文件路径。
- MSP、中断和 HAL 时基：每个新增外设都要配齐所需时钟、引脚、NVIC、IRQ
  处理及 HAL 驱动。
- 链接脚本与 RAM 使用：新增静态线程栈、缓冲区或内存池后确认空间足够；
  若使用动态内存，显式配置并核对内存边界。

保留 HAL/CMSIS/ThreadX 从 STM32Cube 固件包外部引用的做法即可；除非项目有
明确的依赖封装需求，不必把整套库复制进新工程。

## 4. 核对目标板与 TrustZone 配置

不要因为项目目录名已经包含 `SECURE`，就默认所有目标都以 Secure 状态运行。
烧录前确认目标板实际 Secure/Non-Secure 执行状态、启动链、安全属性和 ThreadX
单模式定义一致。模板默认使用 `THREADX_MODE=SECURE`；改成 Non-Secure 还需要
单独实现和验证 Secure-to-Non-Secure 启动、向量表及内存/外设安全属性，单改
编译宏并不能建立这些配置。

如果更换芯片/板卡，还要重新核对时钟源与主频、链接地址和 RAM 大小、启动文件、
OpenOCD 配置、SVD 以及 LED/按键引脚。

## 5. 清理、构建并验证

首次构建前清理从模板带来的对象/依赖文件和旧 ELF，然后构建新目标：

```sh
make clean
make -j4
```

检查输出文件名与新项目目标一致。板上验证前应确认 ThreadX 首次上下文切换
正常、没有 HardFault，再继续验证具体外设和应用功能。成功构建或烧录校验本身
不等于运行验证完成。

## 开始开发前检查表

- [ ] 已复制整个模板到独立的新目录。
- [ ] 新项目目录名已确定，且当前终端位于新目录。
- [ ] Makefile 的 `TARGET` 与项目命名一致。
- [ ] `.vscode/launch.json` 的 ELF 路径与 `TARGET` 一致。
- [ ] README 已描述新项目用途、依赖和运行方法。
- [ ] 已核对目标板、TrustZone 执行状态和 ThreadX 模式。
- [ ] 已运行 `make clean && make -j4`，构建结果使用新项目名。
