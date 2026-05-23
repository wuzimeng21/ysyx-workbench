#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <assert.h>
#include <time.h>
#include "syscall.h"

// ==================== 辅助宏 ====================

// 宏连接：将两个 token 连接成一个
#define _concat(x, y) x ## y
#define concat(x, y) _concat(x, y)

// 参数提取宏：从可变参数列表中提取第 n 个参数
#define _args(n, list) concat(_arg, n) list
#define _arg0(a0, ...) a0
#define _arg1(a0, a1, ...) a1
#define _arg2(a0, a1, a2, ...) a2
#define _arg3(a0, a1, a2, a3, ...) a3
#define _arg4(a0, a1, a2, a3, a4, ...) a4
#define _arg5(a0, a1, a2, a3, a4, a5, ...) a5

// ==================== 系统调用参数抽象 ====================

// 从 ISA 特定的宏数组中提取各元素
#define SYSCALL  _args(0, ARGS_ARRAY)  // 自陷指令
#define GPR1 _args(1, ARGS_ARRAY)      // 系统调用号寄存器
#define GPR2 _args(2, ARGS_ARRAY)      // 参数1寄存器
#define GPR3 _args(3, ARGS_ARRAY)      // 参数2寄存器
#define GPR4 _args(4, ARGS_ARRAY)      // 参数3寄存器
#define GPRx _args(5, ARGS_ARRAY)      // 返回值寄存器

// ==================== ISA 特定定义 ====================

#if defined(__ISA_X86__)               
# define ARGS_ARRAY ("int $0x80", "eax", "ebx", "ecx", "edx", "eax")
#elif defined(__ISA_MIPS32__)  
# define ARGS_ARRAY ("syscall", "v0", "a0", "a1", "a2", "v0")
#elif defined(__riscv)                 
#ifdef __riscv_e                        
# define ARGS_ARRAY ("ecall", "a5", "a0", "a1", "a2", "a0")
#else                                   
# define ARGS_ARRAY ("ecall", "a7", "a0", "a1", "a2", "a0")
#endif
#elif defined(__ISA_AM_NATIVE__)  
# define ARGS_ARRAY ("call *0x100000", "rdi", "rsi", "rdx", "rcx", "rax")
#elif defined(__ISA_X86_64__)      
# define ARGS_ARRAY ("int $0x80", "rdi", "rsi", "rdx", "rcx", "rax")
#elif defined(__ISA_LOONGARCH32R__) 
# define ARGS_ARRAY ("syscall 0", "a7", "a0", "a1", "a2", "a0")
#else
#error _syscall_ is not implemented
#endif



// ==================== 核心系统调用接口 ====================

/**
 * @brief 通用系统调用函数
 * @param type 系统调用号
 * @param a0 参数1
 * @param a1 参数2
 * @param a2 参数3
 * @return 系统调用返回值
 * 
 * 这个函数将参数放入指定寄存器，执行自陷指令，
 * 然后从返回值寄存器获取结果。
 */
intptr_t _syscall_(intptr_t type, intptr_t a0, intptr_t a1, intptr_t a2) {
  // 将参数放入对应寄存器
  register intptr_t _gpr1 asm (GPR1) = type;  // 系统调用号
  register intptr_t _gpr2 asm (GPR2) = a0;    // 参数1
  register intptr_t _gpr3 asm (GPR3) = a1;    // 参数2
  register intptr_t _gpr4 asm (GPR4) = a2;    // 参数3
  register intptr_t ret asm (GPRx);            // 返回值
  
  // 执行自陷指令
  asm volatile (SYSCALL : "=r" (ret) : "r"(_gpr1), "r"(_gpr2), "r"(_gpr3), "r"(_gpr4));
  
  return ret;
}



// ==================== Newlib 必需的系统调用 ====================

/**
 * @brief 退出程序
 * @param status 退出状态码
 */
void _exit(int status) {
  _syscall_(SYS_exit, status, 0, 0);
  while (1);  // 防止编译器警告
}

/**
 * @brief 打开文件
 * @param path 文件路径
 * @param flags 打开标志
 * @param mode 权限模式
 * @return 文件描述符
 */
int _open(const char *path, int flags, mode_t mode) {
  // _exit(SYS_open);  // 暂未实现
  __syscall_(SYS_open, path, flags, mode);
  return 0;
}

/**
 * @brief 写入文件
 * @param fd 文件描述符
 * @param buf 数据缓冲区
 * @param count 字节数
 * @return 实际写入的字节数
 */
int _write(int fd, void *buf, size_t count) {
  // _exit(SYS_write);  // 暂未实现
  return _syscall_(SYS_write, fd, buf, count);
}

/**
 * @brief 调整堆区大小
 * @param increment 增加字节数
 * @return 旧堆顶指针，失败返回 -1
 */
void *_sbrk(intptr_t increment) {
  // return (void *)-1;  // 暂未实现
  extern char _end;
  char * program_break = &_end;
  char * program_break_record = program_break;
  program_break_record += increment;
  if(program_break_record < &_end) {
    return (void *)-1; 
  }
  if(_syscall_(SYS_brk, increment, program_break_record, 0) == 0 ) {
    char * program_break_prefer = program_break;
    program_break = program_break_record;
    return program_break_prefer;
  }
  return (void *)-1; 

}

/**
 * @brief 读取文件
 * @param fd 文件描述符
 * @param buf 缓冲区
 * @param count 要读取的字节数
 * @return 实际读取的字节数
 */
int _read(int fd, void *buf, size_t count) {
  // _exit(SYS_read);
  __syscall_(SYS_read, fd, buf, count);
  return 0;
}

/**
 * @brief 关闭文件
 * @param fd 文件描述符
 * @return 0 成功，-1 失败
 */
int _close(int fd) {
  // _exit(SYS_close);
  __syscall_(SYS_close, fd, 0, 0);
  return 0;
}

/**
 * @brief 移动文件指针
 * @param fd 文件描述符
 * @param offset 偏移量
 * @param whence 起始位置
 * @return 新文件偏移
 */
off_t _lseek(int fd, off_t offset, int whence) {
  // _exit(SYS_lseek);
  __syscall_(SYS_lseek, fd, offset, whence);
  return 0;
}

/**
 * @brief 获取时间
 * @param tv 时间结构体
 * @param tz 时区结构体（已废弃）
 * @return 0 成功，-1 失败
 */
int _gettimeofday(struct timeval *tv, struct timezone *tz) {
  // _exit(SYS_gettimeofday);
  return __syscall_(SYS_gettimeofday, tv, tz, 0);
}

/**
 * @brief 执行新程序
 * @param fname 程序文件名
 * @param argv 参数列表
 * @param envp 环境变量
 * @return 成功不返回，失败返回 -1
 */
// int _execve(const char *fname, char * const argv[], char *const envp[]) {
//   // _exit(SYS_execve);
//   return __syscall_(SYS_execve, fname, argv, envp);
// }
int _execve(const char *fname, char *const argv[], char *const envp[])
{
    intptr_t ret = _syscall_(SYS_execve, (uintptr_t)fname, (uintptr_t)argv,
                             (uintptr_t)envp);
    if (ret < 0)
    {
        errno = -ret;
        return -1;
    }
    return 0;
}

// ==================== 未使用的系统调用 ====================
// 以下函数在 Nanos-lite 中未使用，但为通过链接而定义

int _fstat(int fd, struct stat *buf) {
  return -1;
}

int _stat(const char *fname, struct stat *buf) {
  assert(0);
  return -1;
}

int _kill(int pid, int sig) {
  _exit(-SYS_kill);
  return -1;
}

pid_t _getpid() {
  _exit(-SYS_getpid);
  return 1;
}

pid_t _fork() {
  assert(0);
  return -1;
}

pid_t vfork() {
  assert(0);
  return -1;
}

int _link(const char *d, const char *n) {
  assert(0);
  return -1;
}

int _unlink(const char *n) {
  assert(0);
  return -1;
}

pid_t _wait(int *status) {
  assert(0);
  return -1;
}

clock_t _times(void *buf) {
  assert(0);
  return 0;
}

int pipe(int pipefd[2]) {
  assert(0);
  return -1;
}

int dup(int oldfd) {
  assert(0);
  return -1;
}

int dup2(int oldfd, int newfd) {
  return -1;
}

unsigned int sleep(unsigned int seconds) {
  assert(0);
  return -1;
}

ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) {
  assert(0);
  return -1;
}

int symlink(const char *target, const char *linkpath) {
  assert(0);
  return -1;
}

int ioctl(int fd, unsigned long request, ...) {
  return -1;
}


