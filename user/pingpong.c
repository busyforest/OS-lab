#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int
main(void)
{
  int f2c[2]; // 父到子的管道
  int c2f[2]; // 子到父的管道
  char buf[1]; // 用于传输的缓冲区
  int ppid = getpid();
  int pid;

  // 创建两个管道
  if (pipe(f2c) < 0 || pipe(c2f) < 0) {
    fprintf(2, "pipe failed\n");
    exit(1);
  }

  if ((pid = fork()) < 0) {
    fprintf(2, "fork failed\n");
    exit(1);
  }

  if (pid == 0) {  // 子进程
    close(f2c[1]); // 关闭父到子的写端

    // 从父进程读取数据
    if (read(f2c[0], buf, 1) != 1) {
      fprintf(2, "child: read error\n");
      exit(1);
    }
    printf("%d: received ping from pid %d\n", getpid(), ppid);

    // 向父进程发送数据
    if (write(c2f[1], "p", 1) != 1) {
      fprintf(2, "child: write error\n");
      exit(1);
    }

    close(f2c[0]); // 关闭父到子的读端
    close(c2f[1]); // 关闭子到父的写端

  } else {  // 父进程
    close(f2c[0]); // 关闭父到子的读端
    close(c2f[1]); // 关闭子到父的写端

    // 向子进程发送数据
    if (write(f2c[1], "p", 1) != 1) {
      fprintf(2, "parent: write error\n");
      exit(1);
    }

    // 从子进程读取数据
    if (read(c2f[0], buf, 1) != 1) {
      fprintf(2, "parent: read error\n");
      exit(1);
    }
    printf("%d: received pong from pid %d\n", getpid(), pid);

    close(f2c[1]); // 关闭父到子的写端
    close(c2f[0]); // 关闭子到父的读端

    // 等待子进程结束
    wait(0);
  }

  exit(0);
}