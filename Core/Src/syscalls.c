#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "main.h"

int _close(int file)
{
  (void)file;
  errno = EBADF;
  return -1;
}

int _lseek(int file, int ptr, int dir)
{
  (void)file;
  (void)ptr;
  (void)dir;
  errno = EBADF;
  return -1;
}

int _read(int file, char *ptr, int len)
{
  (void)file;
  (void)ptr;
  (void)len;
  errno = EBADF;
  return -1;
}

int _write(int file, const char *ptr, int len)
{
  (void)file;

  /* Retarget stdio output (e.g. printf) to the ST-LINK virtual COM port. */
  if (HAL_UART_Transmit(&huart3, (uint8_t *)ptr, (uint16_t)len, HAL_MAX_DELAY) != HAL_OK)
  {
    errno = EIO;
    return -1;
  }

  return len;
}

int _isatty(int file)
{
  (void)file;
  return 1;
}

int _fstat(int file, struct stat *st)
{
  (void)file;
  if (st == NULL)
  {
    errno = EBADF;
    return -1;
  }

  st->st_mode = S_IFCHR;
  return 0;
}

void _exit(int status)
{
  (void)status;
  while (1)
  {
  }
}

int _getpid(void)
{
  return 1;
}

int _kill(int pid, int sig)
{
  (void)pid;
  (void)sig;
  errno = EINVAL;
  return -1;
}
