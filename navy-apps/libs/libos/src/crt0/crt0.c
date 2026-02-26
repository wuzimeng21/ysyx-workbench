/**
 * @file crt0.c
 * @brief C 运行时初始化代码（C RunTime Zero）
 * 
 * 这个文件是用户程序的入口点，在 start.S 中的 _start 函数之后执行。
 * 它负责设置环境变量，然后调用用户程序的 main 函数。
 */

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

// 用户程序的 main 函数声明
// argc: 命令行参数个数
// argv: 命令行参数数组
// envp: 环境变量数组
int main(int argc, char *argv[], char *envp[]);
extern char **environ;

void call_main(uintptr_t *args) {
  // 创建一个空的环境变量数组,只包含 NULL 终止符,表示没有环境变量
  char *empty[] = {NULL};
  // 设置全局环境变量指针,这样 getenv 等函数就能找到环境变量
  environ = empty;
  
  // 调用用户程序的 main 函数
  //   argc = 0 (没有命令行参数)
  //   argv = empty (空参数列表)
  //   envp = empty (空环境变量)
  // main 函数返回后，用返回值调用 exit
  exit(main(0, empty, empty));
  assert(0);
}


