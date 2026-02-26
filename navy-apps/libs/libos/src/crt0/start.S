/**
 * @file start.S
 * @brief C 运行时入口点（_start）
 * 
 * 这个文件是用户程序执行的第一条指令的入口。
 * 它负责设置初始环境，然后跳转到 C 代码的 call_main 函数。
 */

.globl _start
_start:

#if defined(__ISA_AM_NATIVE__)
  // ==================== x86-64 (native) 架构 ====================
  
  // 清零帧指针（标记栈底）
  movq $0, %rbp
  
  // x86-64 ABI 要求：在函数入口处，栈指针 (rsp+8) 必须是 16 字节对齐
  // 这是因为函数返回地址占 8 字节，所以 rsp 本身应该是 8 字节对齐
  // 将 rsp 与 0xfffffffffffffff0 按位与，实现 16 字节对齐
  andq $0xfffffffffffffff0, %rsp
  
  // 将第一个参数（rax 中的值）传递给 rdi
  // 根据 x86-64 调用约定，第一个参数放在 rdi 中
  movq %rax, %rdi
  
  // 调用 C 函数 call_main
  call call_main

#elif defined(__ISA_X86_64__)
  // ==================== x86-64 架构 ====================
  
  // 清零帧指针
  movq $0, %rbp
  
  // 将栈指针设置为 rax 的值
  movq %rax, %rsp
  
  // 将第一个参数传递给 rdi
  movq %rax, %rdi
  
  // 调用 C 函数 call_main
  call call_main

#elif defined(__ISA_X86__)
  // ==================== x86 (32位) 架构 ====================
  
  // 清零帧指针
  movl $0, %ebp
  
  // 调用 C 函数 call_main
  call call_main

#elif defined(__ISA_MIPS32__)
  // ==================== MIPS32 架构 ====================
  
  // 清零帧指针
  move $fp, $zero
  
  // 跳转到 C 函数 call_main
  jal call_main

#elif defined(__riscv)
  // ==================== RISC-V 架构 ====================
  
  // 清零 s0 寄存器（相当于帧指针）
  mv s0, zero
  
  // 跳转到 C 函数 call_main
  jal call_main

#elif defined(__ISA_LOONGARCH32R__)
  // ==================== LoongArch 32 架构 ====================
  
  // 清零帧指针
  move $fp, $zero
  
  // 跳转到 C 函数 call_main
  bl call_main

#else
#error Unsupported ISA
#endif