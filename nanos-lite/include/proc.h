#ifndef __PROC_H__
#define __PROC_H__

#include <common.h>
#include <memory.h>

#define STACK_SIZE (8 * PGSIZE)

typedef union {
  uint8_t stack[STACK_SIZE] PG_ALIGN;
  struct {
    Context *cp;      // 上下文指针，保存寄存器状态
    AddrSpace as;      // 地址空间，管理进程内存映射
    // we do not free memory, so use `max_brk' to determine when to call _map()
    uintptr_t max_brk; // 最大堆地址，用于内存分配
  };
} PCB;

extern PCB *current; // 指向当前正在运行的进程的 PCB。

#endif
