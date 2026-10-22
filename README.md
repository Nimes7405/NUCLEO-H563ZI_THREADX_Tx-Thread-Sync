# Tx_Thread_Sync — ThreadX synchronization example (NUCLEO-H563ZI)

This project is the hand-written Makefile port of the STM32CubeH5
[`Tx_Thread_Sync`](https://github.com/STMicroelectronics/STM32CubeH5/tree/main/Projects/NUCLEO-H563ZI/Applications/ThreadX/Tx_Thread_Sync)
application. It demonstrates how two equal-priority ThreadX threads share a
critical section (LED toggling) protected by a synchronization object.

## What it does

`tx_application_define()` creates two threads with the same priority (10) and
the same preemption threshold, using a 5-tick time-slice:

- **ThreadOne** toggles the green LED (`LED1`, PB0) every 500 ms for 5 s.
- **ThreadTwo** toggles the red LED (`LED3`, PG4) every 500 ms for 5 s.

Both threads use `Led_Toggle()` as a critical section and must acquire the
sync object before entering it:

- Try to acquire the sync object with no wait (`TX_NO_WAIT`).
- On success: print `** ThreadXXX : SyncObject acquired **`, toggle the LED
  for 5 s, release the object and print `** ThreadXXX : SyncObject released **`,
  then sleep 10 ms.
- On failure: periodically print `** ThreadXXX : waiting for SyncObject !! **`.

The synchronization object is a `TX_MUTEX` by default. Define
`USE_TX_SEMAPHORE` (and remove `USE_TX_MUTEX`) in
[`Core/Inc/app_threadx.h`](Core/Inc/app_threadx.h) to use a binary
`TX_SEMAPHORE` instead.

Messages are printed on USART3 (115200 8-N-1) via the ST-LINK virtual COM port.

## Project layout

```text
Core/
  Inc/       HAL, ThreadX and application configuration headers
  Src/       main, application threads, MSP, IRQ, HAL TIM6 tick,
             syscalls, CMSIS system setup and ThreadX low-level init
startup/     STM32H563 startup and vector table
openocd/     Board-specific OpenOCD config and STM32H5 target config
svd/         STM32H563 SVD for Cortex-Debug peripheral views
Makefile     HAL/ThreadX source selection, build, flash and clean targets
*.ld         Secure single-image Flash/RAM layout
.vscode/     Build, clean, flash and Cortex-Debug launch configuration
```

## External dependencies

The HAL, CMSIS device headers and ThreadX kernel/port implementation are kept
as external dependencies from the STM32Cube_FW_H5 package. Override `CUBE_FW`
when building on another machine:

```sh
make clean
make CUBE_FW=/path/to/STM32Cube_FW_H5_V1.7.0
make flash CUBE_FW=/path/to/STM32Cube_FW_H5_V1.7.0
```

## Build, flash, and debug

```sh
make clean
make -j4
make flash
```

`make flash` uses the local `openocd/st_nucleo_h5.cfg`. In VS Code, install
Cortex-Debug and use the provided build/clean/flash tasks and the
`Debug STM32H5 ThreadX (OpenOCD)` launch configuration.

## Reading the serial output

Thread messages are printed on USART3 (115200 8-N-1) through the ST-LINK
virtual COM port. On Linux the STLINK-V3 enumerates as `/dev/ttyACM0`; confirm
the device name with:

```sh
ls /dev/ttyACM* /dev/ttyUSB*
```

With no extra packages, set the line parameters and read the device directly:

```sh
stty -F /dev/ttyACM0 115200 cs8 -cstopb -parenb
cat /dev/ttyACM0
```

Press `Ctrl+C` to stop reading. If a terminal program is available, use one of:

```sh
screen /dev/ttyACM0 115200      # quit: Ctrl+A then K
picocom -b 115200 /dev/ttyACM0  # quit: Ctrl+A then Ctrl+Q
minicom -D /dev/ttyACM0 -b 115200  # quit: Ctrl+A then X
```

Flash the firmware first (`make flash`), then open the port before resetting
the board so the initial output isn't missed. Expected output:

```text
** ThreadOne : SyncObject acquired **
** ThreadTwo : waiting for SyncObject !! **
** ThreadOne : SyncObject released **
** ThreadTwo : SyncObject acquired **
```

The `waiting` lines are normal: ThreadTwo polls the sync object with
`TX_NO_WAIT` and prints one line per failed acquisition while ThreadOne holds
it, then acquires it once ThreadOne releases.

## ThreadX / HAL time-base notes

- ThreadX tick is 100 ticks/s on SysTick; HAL uses TIM6 for its 1 ms tick.
  See [`NUCLEO-H563ZI_THREADX_TIPS.md`](NUCLEO-H563ZI_THREADX_TIPS.md) for the
  reusable configuration constraints.
- `THREADX_MODE` defaults to `SECURE` (`TX_SINGLE_MODE_SECURE=1`), matching the
  verified NUCLEO-H563ZI Secure startup.
- Thread control blocks and stacks are statically allocated; no ThreadX byte
  pool or dynamic heap is used.

Successful build/flash does not prove the first ThreadX context switch
completed. Check the LEDs and the UART output, and inspect `CFSR` for `INVPC`
if the board faults on the first switch.
