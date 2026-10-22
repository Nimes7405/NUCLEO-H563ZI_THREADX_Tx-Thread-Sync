#ifndef MAIN_H
#define MAIN_H

#include "stm32h5xx_hal.h"
#include <stdio.h>

void Error_Handler(void);

/* NUCLEO-H563ZI LEDs:
   - LED1 (green): PB0
   - LED3 (red):   PG4 */
#define LED1_Pin        GPIO_PIN_0
#define LED1_GPIO_Port  GPIOB
#define LED3_Pin        GPIO_PIN_4
#define LED3_GPIO_Port  GPIOG

extern UART_HandleTypeDef huart3;

#endif
