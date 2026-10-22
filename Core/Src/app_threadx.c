#include "app_threadx.h"

/* Statically allocated ThreadX objects. The two threads share the same
   priority and time-slice, so the scheduler runs them in round-robin order. */
static TX_THREAD ThreadOne;
static TX_THREAD ThreadTwo;
static UCHAR thread_one_stack[TX_APP_STACK_SIZE] __attribute__((aligned(8)));
static UCHAR thread_two_stack[TX_APP_STACK_SIZE] __attribute__((aligned(8)));

/* Synchronization object protecting the Led_Toggle critical section. */
static APP_SYNC_TYPE SyncObject;

static VOID Led_Toggle(uint16_t led_pin, GPIO_TypeDef *led_port, UINT iter);
static VOID App_Delay(ULONG delay);

void tx_application_define(VOID *first_unused_memory)
{
  (void)first_unused_memory;

  if (tx_thread_create(&ThreadOne,
                       "Thread One",
                       ThreadOne_Entry,
                       0,
                       thread_one_stack,
                       sizeof(thread_one_stack),
                       TX_APP_THREAD_PRIO,
                       TX_APP_THREAD_PREEMPTION_THRESHOLD,
                       TX_APP_THREAD_TIME_SLICE,
                       TX_APP_THREAD_AUTO_START) != TX_SUCCESS)
  {
    Error_Handler();
  }

  if (tx_thread_create(&ThreadTwo,
                       "Thread Two",
                       ThreadTwo_Entry,
                       0,
                       thread_two_stack,
                       sizeof(thread_two_stack),
                       TX_APP_THREAD_PRIO,
                       TX_APP_THREAD_PREEMPTION_THRESHOLD,
                       TX_APP_THREAD_TIME_SLICE,
                       TX_APP_THREAD_AUTO_START) != TX_SUCCESS)
  {
    Error_Handler();
  }

  if (APP_SYNC_CREATE(&SyncObject) != TX_SUCCESS)
  {
    Error_Handler();
  }
}

void ThreadOne_Entry(ULONG thread_input)
{
  UNUSED(thread_input);
  ULONG iteration = 0;

  for (;;)
  {
    /* Try to acquire the sync object without waiting. */
    if (APP_SYNC_GET(&SyncObject, TX_NO_WAIT) == TX_SUCCESS)
    {
      printf("** ThreadOne : SyncObject acquired **\n");

      /* Sync object acquired: toggle the green LED every 500 ms for 5 s. */
      Led_Toggle(LED1_Pin, LED1_GPIO_Port, 10);

      APP_SYNC_PUT(&SyncObject);
      printf("** ThreadOne : SyncObject released **\n");

      tx_thread_sleep(1);
    }
    else
    {
      if ((iteration % 2000000U) == 0U)
      {
        printf("** ThreadOne : waiting for SyncObject !! **\n");
      }
    }
    iteration++;
  }
}

void ThreadTwo_Entry(ULONG thread_input)
{
  UNUSED(thread_input);
  ULONG iteration = 0;

  for (;;)
  {
    if (APP_SYNC_GET(&SyncObject, TX_NO_WAIT) == TX_SUCCESS)
    {
      printf("** ThreadTwo : SyncObject acquired **\n");

      /* Sync object acquired: toggle the red LED every 500 ms for 5 s. */
      Led_Toggle(LED3_Pin, LED3_GPIO_Port, 10);

      APP_SYNC_PUT(&SyncObject);
      printf("** ThreadTwo : SyncObject released **\n");

      tx_thread_sleep(1);
    }
    else
    {
      if ((iteration % 2000000U) == 0U)
      {
        printf("** ThreadTwo : waiting for SyncObject !! **\n");
      }
    }
    iteration++;
  }
}

static VOID Led_Toggle(uint16_t led_pin, GPIO_TypeDef *led_port, UINT iter)
{
  UINT i;

  HAL_GPIO_WritePin(led_port, led_pin, GPIO_PIN_RESET);

  for (i = 0; i < iter; i++)
  {
    HAL_GPIO_TogglePin(led_port, led_pin);
    /* Toggle the LED every 500 ms (50 ticks at 100 ticks/s). */
    App_Delay(50);
  }

  HAL_GPIO_WritePin(led_port, led_pin, GPIO_PIN_RESET);
}

static VOID App_Delay(ULONG delay)
{
  ULONG initial_time = tx_time_get();

  while ((tx_time_get() - initial_time) < delay)
  {
  }
}
