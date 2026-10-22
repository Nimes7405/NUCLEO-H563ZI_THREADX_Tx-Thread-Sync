#include "main.h"
#include "stm32h5xx_it.h"

extern TIM_HandleTypeDef htim6;

void HardFault_Handler(void)
{
  while (1) {}
}

void TIM6_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim6);
}
