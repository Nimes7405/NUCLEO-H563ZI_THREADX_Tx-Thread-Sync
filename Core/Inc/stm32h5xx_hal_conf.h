#ifndef STM32H5XX_HAL_CONF_H
#define STM32H5XX_HAL_CONF_H

#define HAL_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED

#define HSE_VALUE 8000000UL
#define HSE_STARTUP_TIMEOUT 100UL
#define HSI_VALUE 64000000UL
#define HSI48_VALUE 48000000UL
#define CSI_VALUE 4000000UL
#define LSI_VALUE 32000UL
#define LSE_VALUE 32768UL
#define LSE_STARTUP_TIMEOUT 5000UL
#define EXTERNAL_CLOCK_VALUE 12288000UL

#define VDD_VALUE 3300UL
#define TICK_INT_PRIORITY 15UL
#define USE_RTOS 0U
#define PREFETCH_ENABLE 0U
#define CACHE_ENABLE 1U
#define USE_HAL_ADC_REGISTER_CALLBACKS 0U
#define USE_HAL_CORTEX_REGISTER_CALLBACKS 0U
#define USE_HAL_COMP_REGISTER_CALLBACKS 0U
#define USE_HAL_CRC_REGISTER_CALLBACKS 0U
#define USE_HAL_DAC_REGISTER_CALLBACKS 0U
#define USE_HAL_DMA_REGISTER_CALLBACKS 0U
#define USE_HAL_EXTI_REGISTER_CALLBACKS 0U
#define USE_HAL_FLASH_REGISTER_CALLBACKS 0U
#define USE_HAL_GPIO_REGISTER_CALLBACKS 0U
#define USE_HAL_I2C_REGISTER_CALLBACKS 0U
#define USE_HAL_ICACHE_REGISTER_CALLBACKS 0U
#define USE_HAL_PCD_REGISTER_CALLBACKS 0U
#define USE_HAL_PWR_REGISTER_CALLBACKS 0U
#define USE_HAL_RCC_REGISTER_CALLBACKS 0U
#define USE_HAL_RTC_REGISTER_CALLBACKS 0U
#define USE_HAL_SPI_REGISTER_CALLBACKS 0U
#define USE_HAL_TIM_REGISTER_CALLBACKS 0U
#define USE_HAL_UART_REGISTER_CALLBACKS 0U
#define USE_HAL_USART_REGISTER_CALLBACKS 0U

#ifdef HAL_RCC_MODULE_ENABLED
#include "stm32h5xx_hal_rcc.h"
#include "stm32h5xx_hal_rcc_ex.h"
#endif
#ifdef HAL_GPIO_MODULE_ENABLED
#include "stm32h5xx_hal_gpio.h"
#include "stm32h5xx_hal_gpio_ex.h"
#endif
#ifdef HAL_DMA_MODULE_ENABLED
#include "stm32h5xx_hal_dma.h"
#endif
#ifdef HAL_CORTEX_MODULE_ENABLED
#include "stm32h5xx_hal_cortex.h"
#endif
#ifdef HAL_FLASH_MODULE_ENABLED
#include "stm32h5xx_hal_flash.h"
#endif
#ifdef HAL_PWR_MODULE_ENABLED
#include "stm32h5xx_hal_pwr.h"
#include "stm32h5xx_hal_pwr_ex.h"
#endif
#ifdef HAL_TIM_MODULE_ENABLED
#include "stm32h5xx_hal_tim.h"
#endif
#ifdef HAL_UART_MODULE_ENABLED
#include "stm32h5xx_hal_uart.h"
#include "stm32h5xx_hal_uart_ex.h"
#endif

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line);
#define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
#else
#define assert_param(expr) ((void)0U)
#endif

#endif
