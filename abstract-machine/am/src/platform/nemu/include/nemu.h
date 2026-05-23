/**
 * @file nemu.h
 * @brief NEMU 通用定义和硬件抽象层
 * 
 * 这个头文件是 NEMU 项目的核心，定义了：
 * 1. 架构相关的陷入宏
 * 2. 设备地址映射
 * 3. 物理内存布局
 */

#ifndef NEMU_H__
#define NEMU_H__

#include <klib-macros.h>

// ==================== 架构相关头文件 ====================
// ISA_H 由编译选项定义，例如：
// -DISA_H="\"x86/x86.h\"" 
// -DISA_H="\"mips/mips32.h\""
// -DISA_H="\"riscv/riscv.h\""
#include ISA_H

// ==================== 客户程序陷入宏 ====================

/**
 * @brief 让客户程序陷入 NEMU 的调试状态
 * @param code 退出码/陷阱码
 * 
 * 不同的 ISA 使用不同的指令实现陷入：
 * - x86: int3 指令，触发调试异常
 * - mips32: sdbbp 指令，断点异常
 * - riscv: ebreak 指令，环境断点
 * 
 * 使用示例：
 *   nemu_trap(0);  // 正常退出
 *   nemu_trap(1);  // 异常退出
 */
#if defined(__ISA_X86__)
# define nemu_trap(code) asm volatile ("int3" : :"a"(code))

#elif defined(__ISA_MIPS32__)
# define nemu_trap(code) asm volatile ("move $v0, %0; sdbbp" : :"r"(code))

#elif defined(__ISA_RISCV32__) || defined(__ISA_RISCV64__)
# define nemu_trap(code) asm volatile("mv a0, %0; ebreak" : :"r"(code))

#else
# error unsupported ISA __ISA__
#endif

// ==================== 设备基址定义 ====================

/**
 * 设备基址根据架构不同：
 * - x86-nemu: 使用独立的端口I/O空间，基址为0
 * - 其他架构: 使用内存映射I/O，基址为0xa0000000
 */
#if defined(__ARCH_X86_NEMU)
# define DEVICE_BASE 0x0
#else
# define DEVICE_BASE 0xa0000000
#endif

/**
 * MMIO 基址固定为 0xa0000000
 * 这个区域被映射到设备寄存器
 */
#define MMIO_BASE 0xa0000000

// ==================== 设备地址宏 ====================

// 小设备（寄存器较少）基于 DEVICE_BASE
#define SERIAL_PORT     (DEVICE_BASE + 0x00003f8)  // 串口 (COM1)
#define KBD_ADDR        (DEVICE_BASE + 0x0000060)  // 键盘控制器 (i8042)
#define RTC_ADDR        (DEVICE_BASE + 0x0000048)  // 实时时钟
#define VGACTL_ADDR     (DEVICE_BASE + 0x0000100)  // VGA 控制器
#define AUDIO_ADDR      (DEVICE_BASE + 0x0000200)  // 音频控制器
#define DISK_ADDR       (DEVICE_BASE + 0x0000300)  // 磁盘控制器

// 大设备（需要连续地址空间）基于 MMIO_BASE
#define FB_ADDR         (MMIO_BASE   + 0x1000000)  // 帧缓冲 (16MB偏移)
#define AUDIO_SBUF_ADDR (MMIO_BASE   + 0x1200000)  // 音频缓冲区 (18MB偏移)

// ==================== 物理内存布局 ====================

/**
 * 物理内存起始地址，由链接脚本定义
 * 通常在 ld 文件中定义为：
 *   _pmem_start = .;
 */
extern char _pmem_start;

/**
 * 物理内存大小：128 MB
 * 这是 NEMU 可用的最大物理内存
 */
#define PMEM_SIZE (128 * 1024 * 1024)

/**
 * 物理内存结束地址
 * 计算方法：起始地址 + 大小
 */
#define PMEM_END  ((uintptr_t)&_pmem_start + PMEM_SIZE)

/**
 * NEMU 物理地址空间布局（用于 DiffTest 等）
 * 定义了一个 RANGE 宏列表，描述所有可用的物理地址区间：
 * 
 * 1. 物理内存区间 [&_pmem_start, PMEM_END)
 * 2. 帧缓冲区间 [FB_ADDR, FB_ADDR + 2MB)
 * 3. MMIO 设备区间 [MMIO_BASE, MMIO_BASE + 4KB)
 *    包含串口、RTC、键盘、VGA控制器等
 */
#define NEMU_PADDR_SPACE                                                       \
    RANGE(&_pmem_start, PMEM_END),                                             \
    RANGE(FB_ADDR, FB_ADDR + 0x200000),                                        \
    RANGE(AUDIO_SBUF_ADDR, AUDIO_SBUF_ADDR + 0x10000),                         \
    RANGE(MMIO_BASE, MMIO_BASE + 0x1000) /* serial, rtc, screen, keyboard */
    
// ==================== 内存管理相关 ====================

/**
 * 页表项类型（Page Table Entry）
 * 用于虚拟内存管理
 */
typedef uintptr_t PTE;

/**
 * 页大小：4 KB
 * 标准 RISC-V 页大小
 */
#define PGSIZE    4096

#endif // NEMU_H__