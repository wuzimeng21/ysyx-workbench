#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;
// ROUNDUP(x, align) 是一个常用宏，
// 用于将数值 x 向上对齐到 align 的倍数。
static char *hbrk = (void *)ROUNDUP(heap.start, 4);// riscv32

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0 ? -x : x);
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}


// 分配 size 字节的未初始化存储。
// 如果分配成功，返回一个指针，该指针对于任何具有基本对齐的对象类型都具有适当的对齐。
// 如果 size 为零，malloc 的行为是实现定义的。
// 例如，可能会返回一个空指针。或者，可能会返回一个非空指针；但此指针不应被解引用，并且应传递给 free 以避免内存泄漏。

// 成功时，返回指向新分配内存起始的指针。为避免内存泄漏，返回的指针必须用 free() 或 realloc() 解除分配。
// 失败时，返回空指针。
void *malloc(size_t size) {
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
  size  = (size_t)ROUNDUP(size, 4); // riscv32
  char * old  = hbrk;
  hbrk += size;
  if(size == NULL || size == 0) return old;
  // 检查是否在堆范围内
  assert((uintptr_t)heap.start <= (uintptr_t)hbrk && (uintptr_t)hbrk < (uintptr_t)heap.end);
  for (uint32_t *p = (uint32_t *)old; p != (uint32_t *)hbrk; p ++) {
    *p = 0; // clear the allocated space
  }
  // 检查内存限制
  assert((uintptr_t)hbrk - (uintptr_t)heap.start <= setting->mlim);
  return old;
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  panic("Not implemented");
#endif
  return NULL;
}

void free(void *ptr) {
}

#endif
