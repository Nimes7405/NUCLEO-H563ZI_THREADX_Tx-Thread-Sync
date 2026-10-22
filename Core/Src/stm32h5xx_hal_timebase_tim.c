#include "stm32h5xx_hal.h"
#include "stm32h5xx_hal_tim.h"

TIM_HandleTypeDef htim6;

HAL_StatusTypeDef HAL_InitTick(uint32_t tick_priority)
{
  RCC_ClkInitTypeDef clock_config = {0};
  uint32_t flash_latency = 0U;
  uint32_t apb1_prescaler;
  uint32_t timer_clock;
  uint32_t prescaler;
  HAL_StatusTypeDef status;

  __HAL_RCC_TIM6_CLK_ENABLE();
  HAL_RCC_GetClockConfig(&clock_config, &flash_latency);
  apb1_prescaler = clock_config.APB1CLKDivider;
  timer_clock = HAL_RCC_GetPCLK1Freq();
  if (apb1_prescaler != RCC_HCLK_DIV1)
  {
    timer_clock *= 2U;
  }

  prescaler = (timer_clock / 100000U) - 1U;
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = prescaler;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 99U;
  htim6.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  status = HAL_TIM_Base_Init(&htim6);
  if (status != HAL_OK)
  {
    return status;
  }

  status = HAL_TIM_Base_Start_IT(&htim6);
  if (status != HAL_OK)
  {
    return status;
  }

  HAL_NVIC_SetPriority(TIM6_IRQn, tick_priority, 0U);
  HAL_NVIC_EnableIRQ(TIM6_IRQn);
  uwTickPrio = tick_priority;
  return HAL_OK;
}

void HAL_SuspendTick(void)
{
  __HAL_TIM_DISABLE_IT(&htim6, TIM_IT_UPDATE);
}

void HAL_ResumeTick(void)
{
  __HAL_TIM_ENABLE_IT(&htim6, TIM_IT_UPDATE);
}
