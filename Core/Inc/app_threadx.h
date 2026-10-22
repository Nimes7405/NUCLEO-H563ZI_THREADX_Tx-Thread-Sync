#ifndef __APP_THREADX_H
#define __APP_THREADX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "tx_api.h"
#include "main.h"

/* Thread configuration ------------------------------------------------------*/
#define TX_APP_STACK_SIZE                       512
#define TX_APP_THREAD_PRIO                      10

#ifndef TX_APP_THREAD_PREEMPTION_THRESHOLD
#define TX_APP_THREAD_PREEMPTION_THRESHOLD      TX_APP_THREAD_PRIO
#endif

#ifndef TX_APP_THREAD_TIME_SLICE
#define TX_APP_THREAD_TIME_SLICE                5
#endif

#ifndef TX_APP_THREAD_AUTO_START
#define TX_APP_THREAD_AUTO_START                TX_AUTO_START
#endif

/* Synchronization object selection ------------------------------------------*/
/* Use a TX_MUTEX by default. Define USE_TX_SEMAPHORE instead of USE_TX_MUTEX
   to protect the critical section with a binary TX_SEMAPHORE.                */
#define USE_TX_MUTEX
/* #define USE_TX_SEMAPHORE */

#if defined(USE_TX_MUTEX) && defined(USE_TX_SEMAPHORE)
#error "Only one synchronization API must be enabled"
#endif

#ifdef USE_TX_MUTEX
#define APP_SYNC_TYPE                        TX_MUTEX
#define TX_SYNC_ERROR                        TX_MUTEX_ERROR
#define APP_SYNC_GET                         tx_mutex_get
#define APP_SYNC_PUT(a)                      tx_mutex_put(a)
#define APP_SYNC_CREATE(a)                   tx_mutex_create((a), "App Mutex", TX_NO_INHERIT)
#else
#define APP_SYNC_TYPE                        TX_SEMAPHORE
#define TX_SYNC_ERROR                        TX_SEMAPHORE_ERROR
#define APP_SYNC_GET                         tx_semaphore_get
#define APP_SYNC_PUT(a)                      tx_semaphore_ceiling_put((a), 1)
#define APP_SYNC_CREATE(a)                   tx_semaphore_create((a), "App Binary Semaphore", 1)
#endif

/* Exported functions --------------------------------------------------------*/
void ThreadOne_Entry(ULONG thread_input);
void ThreadTwo_Entry(ULONG thread_input);

#ifdef __cplusplus
}
#endif
#endif /* __APP_THREADX_H */
