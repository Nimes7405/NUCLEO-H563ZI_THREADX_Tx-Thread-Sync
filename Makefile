TARGET = NUCLEO-H563ZI_THREADX_Tx-Thread-Sync

TOOLCHAIN_PATH ?= /opt/arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi/bin
PREFIX = arm-none-eabi-
CC = $(TOOLCHAIN_PATH)/$(PREFIX)gcc
OBJCOPY = $(TOOLCHAIN_PATH)/$(PREFIX)objcopy
SIZE = $(TOOLCHAIN_PATH)/$(PREFIX)size
OPENOCD ?= /usr/local/bin/openocd

CUBE_FW ?= /mnt/study/mcu_fw/stm32CubeH5/STM32Cube_FW_H5_V1.7.0
HAL_DIR = $(CUBE_FW)/Drivers/STM32H5xx_HAL_Driver
CMSIS_DIR = $(CUBE_FW)/Drivers/CMSIS
THREADX_DIR = $(CUBE_FW)/Middlewares/ST/threadx
THREADX_PORT_DIR = $(THREADX_DIR)/ports/cortex_m33/gnu/src
DEVICE_DIR = $(CMSIS_DIR)/Device/ST/STM32H5xx

THREADX_MODE ?= SECURE
ifeq ($(THREADX_MODE),SECURE)
SECURITY_DEFINE = -DTX_SINGLE_MODE_SECURE=1
else ifeq ($(THREADX_MODE),NON_SECURE)
SECURITY_DEFINE = -DTX_SINGLE_MODE_NON_SECURE=1
else
$(error THREADX_MODE must be SECURE or NON_SECURE)
endif

MCU = cortex-m33
FPU = -mfpu=fpv5-sp-d16 -mfloat-abi=hard
CFLAGS = -mcpu=$(MCU) $(FPU) -mthumb -Wall -Wextra -g -O0 -std=c99
CFLAGS += -ffunction-sections -fdata-sections -MMD -MP
CFLAGS += -DSTM32H563xx -DHSE_VALUE=8000000UL -DUSE_HAL_DRIVER
CFLAGS += -DTX_INCLUDE_USER_DEFINE_FILE
CFLAGS += $(SECURITY_DEFINE)
ASFLAGS = $(CFLAGS) -x assembler-with-cpp

BUILD_DIR = build
LDSCRIPT = STM32H563ZITX_FLASH.ld
OPENOCD_CFG = openocd/st_nucleo_h5.cfg

INC_PATHS = \
	-ICore/Inc \
	-I$(HAL_DIR)/Inc \
	-I$(HAL_DIR)/Inc/Legacy \
	-I$(DEVICE_DIR)/Include \
	-I$(CMSIS_DIR)/Include \
	-I$(THREADX_DIR)/common/inc \
	-I$(THREADX_DIR)/ports/cortex_m33/gnu/inc

HAL_SRCS = \
	$(HAL_DIR)/Src/stm32h5xx_hal.c \
	$(HAL_DIR)/Src/stm32h5xx_hal_cortex.c \
	$(HAL_DIR)/Src/stm32h5xx_hal_dma.c \
	$(HAL_DIR)/Src/stm32h5xx_hal_dma_ex.c \
	$(HAL_DIR)/Src/stm32h5xx_hal_flash.c \
	$(HAL_DIR)/Src/stm32h5xx_hal_flash_ex.c \
	$(HAL_DIR)/Src/stm32h5xx_hal_gpio.c \
	$(HAL_DIR)/Src/stm32h5xx_hal_pwr.c \
	$(HAL_DIR)/Src/stm32h5xx_hal_pwr_ex.c \
	$(HAL_DIR)/Src/stm32h5xx_hal_rcc.c \
	$(HAL_DIR)/Src/stm32h5xx_hal_rcc_ex.c \
	$(HAL_DIR)/Src/stm32h5xx_hal_tim.c \
	$(HAL_DIR)/Src/stm32h5xx_hal_tim_ex.c \
	$(HAL_DIR)/Src/stm32h5xx_hal_uart.c \
	$(HAL_DIR)/Src/stm32h5xx_hal_uart_ex.c

THREADX_SRCS = $(wildcard $(THREADX_DIR)/common/src/*.c)
THREADX_ASM_SRCS = \
	Core/Src/tx_initialize_low_level.S \
	$(THREADX_PORT_DIR)/tx_misra.S \
	$(THREADX_PORT_DIR)/tx_thread_context_restore.S \
	$(THREADX_PORT_DIR)/tx_thread_context_save.S \
	$(THREADX_PORT_DIR)/tx_thread_interrupt_control.S \
	$(THREADX_PORT_DIR)/tx_thread_interrupt_disable.S \
	$(THREADX_PORT_DIR)/tx_thread_interrupt_restore.S \
	$(THREADX_PORT_DIR)/tx_thread_schedule.S \
	$(THREADX_PORT_DIR)/tx_thread_stack_build.S \
	$(THREADX_PORT_DIR)/tx_thread_system_return.S \
	$(THREADX_PORT_DIR)/tx_timer_interrupt.S

APP_SRCS = \
	Core/Src/main.c \
	Core/Src/app_threadx.c \
	Core/Src/syscalls.c \
	Core/Src/stm32h5xx_hal_msp.c \
	Core/Src/stm32h5xx_hal_timebase_tim.c \
	Core/Src/stm32h5xx_it.c \
	Core/Src/system_stm32h5xx.c

ASM_SRCS = startup/startup_stm32h563xx.s $(THREADX_ASM_SRCS)
OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(HAL_SRCS) $(APP_SRCS) $(THREADX_SRCS))
OBJS += $(patsubst %.S,$(BUILD_DIR)/%.o,$(THREADX_ASM_SRCS))
OBJS += $(BUILD_DIR)/startup/startup_stm32h563xx.o
DEPS = $(OBJS:.o=.d)

all: $(TARGET).elf $(TARGET).bin

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC_PATHS) -c $< -o $@

$(BUILD_DIR)/%.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) $(INC_PATHS) -c $< -o $@

$(BUILD_DIR)/startup/%.o: startup/%.s
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) $(INC_PATHS) -c $< -o $@

$(TARGET).elf: $(OBJS) $(LDSCRIPT)
	$(CC) $(CFLAGS) $(INC_PATHS) -T$(LDSCRIPT) -Wl,--gc-sections,-Map=$(TARGET).map \
		$(OBJS) -lc -lm -lnosys --specs=nosys.specs -o $@
	$(SIZE) $@

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET).elf $(TARGET).bin $(TARGET).map

flash: $(TARGET).elf
	$(OPENOCD) -s . -f $(OPENOCD_CFG) -c "program $< verify reset exit"

-include $(DEPS)

.PHONY: all clean flash
