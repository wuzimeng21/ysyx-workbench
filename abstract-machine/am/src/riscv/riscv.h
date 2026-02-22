/**
 * @file riscv.h
 * @brief RISC-V 架构相关定义和内联函数
 * 
 * 这个头文件提供了 RISC-V 架构特有的：
 * 1. MMIO 访问函数（in/out 系列）
 * 2. 页表项标志位
 * 3. 特权模式定义
 * 4. mstatus 寄存器标志位
 */

#ifndef RISCV_H__
#define RISCV_H__

#include <stdint.h>

// ==================== MMIO 访问函数 ====================

/**
 * @brief 从指定地址读取一个字节
 * @param addr 物理地址
 * @return 读取的 8 位数据
 * 
 * volatile 关键字防止编译器优化，确保每次都从内存读取
 * 用于访问内存映射的设备寄存器
 */
static inline uint8_t inb(uintptr_t addr) { 
    return *(volatile uint8_t *)addr; 
}

/**
 * @brief 从指定地址读取两个字节
 * @param addr 物理地址（建议 2 字节对齐）
 * @return 读取的 16 位数据
 */
static inline uint16_t inw(uintptr_t addr) { 
    return *(volatile uint16_t *)addr; 
}

/**
 * @brief 从指定地址读取四个字节
 * @param addr 物理地址（建议 4 字节对齐）
 * @return 读取的 32 位数据
 */
static inline uint32_t inl(uintptr_t addr) { 
    return *(volatile uint32_t *)addr; 
}

/**
 * @brief 向指定地址写入一个字节
 * @param addr 物理地址
 * @param data 要写入的 8 位数据
 */
static inline void outb(uintptr_t addr, uint8_t data) { 
    *(volatile uint8_t *)addr = data; 
}

/**
 * @brief 向指定地址写入两个字节
 * @param addr 物理地址（建议 2 字节对齐）
 * @param data 要写入的 16 位数据
 */
static inline void outw(uintptr_t addr, uint16_t data) { 
    *(volatile uint16_t *)addr = data; 
}

/**
 * @brief 向指定地址写入四个字节
 * @param addr 物理地址（建议 4 字节对齐）
 * @param data 要写入的 32 位数据
 */
static inline void outl(uintptr_t addr, uint32_t data) { 
    *(volatile uint32_t *)addr = data; 
}

// ==================== 页表项标志位 ====================
// RISC-V 页表项（PTE）的标志位定义
// 参考 RISC-V 特权架构规范

#define PTE_V 0x01  // Valid    - 页表项有效
#define PTE_R 0x02  // Read     - 可读
#define PTE_W 0x04  // Write    - 可写
#define PTE_X 0x08  // Execute  - 可执行
#define PTE_U 0x10  // User     - 用户态可访问
#define PTE_A 0x40  // Accessed - 已被访问（由硬件设置）
#define PTE_D 0x80  // Dirty    - 已被修改（由硬件设置）

// ==================== 特权模式 ====================
// RISC-V 支持三种特权模式
// U-Mode: User（用户模式）
// S-Mode: Supervisor（监管模式）
// M-Mode: Machine（机器模式）

enum { 
    MODE_U,     // 用户模式 (0)
    MODE_S,     // 监管模式 (1)
    MODE_M = 3  // 机器模式 (3)
};

// ==================== mstatus 寄存器标志位 ====================
// mstatus（Machine Status Register）的关键标志位

// MPP 
#define MSTATUS_MPP_MASK  (0x3 << 11)
// #define MSTATUS_MPP  (1 << 12)

// MIE
#define MSTATUS_MIE  (1 << 3)

// MPIE
#define MSTATUS_MPIE  (1 << 7)

// MXR (Make eXecutable Readable)
// 允许执行权限的页也被读取
#define MSTATUS_MXR  (1 << 19)

// SUM (Supervisor User Memory access)
// 允许监管模式访问用户页
#define MSTATUS_SUM  (1 << 18)

// 根据 RISC-V 的 XLEN 设置 SXL 和 UXL
// SXL: Supervisor XLEN（监管模式寄存器位宽）
// UXL: User XLEN（用户模式寄存器位宽）
#if __riscv_xlen == 64
// 64 位系统：SXL 和 UXL 设为 64 位
#define MSTATUS_SXL  (2ull << 34)  // SXL = 2 (64-bit)
#define MSTATUS_UXL  (2ull << 32)  // UXL = 2 (64-bit)
#else
// 32 位系统：没有 SXL/UXL 位
#define MSTATUS_SXL  0
#define MSTATUS_UXL  0
#endif

#endif // RISCV_H__