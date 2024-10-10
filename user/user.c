#include "kernel/types.h"
#include "user.h"
#include "kernel/fcntl.h"
#include "kernel/syscall.h"

int
sleep(int ticks)
{
  return syscall(SYS_sleep, ticks);
}
