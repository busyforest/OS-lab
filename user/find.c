#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

// 函数声明
void find(const char *path, const char *name);

// 去除文件名前的路径，只保留文件名部分
char *
fmtname(char *path) {
  static char buf[DIRSIZ+1];
  char *p;

  // 找到路径中最后的 '/' 字符
  for(p = path + strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // 拷贝最后的文件名部分
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  buf[strlen(p)] = 0;
  return buf;
}

// 递归查找目录下的文件和目录
void
find(const char *path, const char *name) {
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  // 打开指定路径
  if((fd = open(path, O_RDONLY)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  // 获取文件的状态信息
  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  // **增加当前目录名的匹配检查**
  if(strcmp(fmtname((char *)path), name) == 0) {
    printf("%s\n", path);  // 输出匹配的路径
  }

  // 如果当前路径是目录，递归查找
  switch(st.type){
  case T_FILE:
    // 如果是文件并且名称匹配，已经输出，跳过
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)){
      printf("find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';

    // 遍历目录中的每个文件或子目录
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;

      // 忽略 "." 和 ".."
      if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;

      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;

      // 递归查找子目录
      if(stat(buf, &st) < 0){
        printf("find: cannot stat %s\n", buf);
        continue;
      }

      find(buf, name);  // 递归调用
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[]) {
  if(argc != 3){
    fprintf(2, "Usage: find <path> <name>\n");
    exit(1);
  }
  
  find(argv[1], argv[2]);
  
  exit(0);
}