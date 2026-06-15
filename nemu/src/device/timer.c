/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/
/**
 * @file timer.c
 * @brief 实时时钟(RTC)设备模拟
 * 
 * 实现一个简单的实时时钟，提供：
 * - 两个32位寄存器，组合成64位微秒级时间戳
 * - 读操作时返回当前时间
 * - 可选的定时器中断功能（native环境）
 */

#include <device/map.h>
#include <device/alarm.h>
#include <utils.h>

// ==================== 全局变量 ====================
static uint32_t *rtc_port_base = NULL;  // RTC寄存器基址（8字节）

// ==================== I/O处理函数 ====================
/**
 * @brief RTC设备的I/O读写回调
 * 
 * 寄存器布局：
 *   offset 0: 时间戳低32位 (只读)
 *   offset 4: 时间戳高32位 (只读)
 * 
 * @param offset  寄存器内偏移（0或4）
 * @param len     访问长度（应为4）
 * @param is_write 是否是写操作（RTC只读）
 */
static void rtc_io_handler(uint32_t offset, int len, bool is_write) {
  // 1. 参数检查
  assert(offset == 0 || offset == 4);  // 只允许访问这两个偏移
  assert(len == 4);                     // 必须按4字节访问
  
  // 2. 读操作且访问高32位寄存器时，更新时间戳
  if (!is_write && offset == 4) {
    // 获取当前时间（微秒级）
    uint64_t us = get_time();
    
    // 将64位时间戳拆分成两个32位写入寄存器
    rtc_port_base[0] = (uint32_t)us;      // 低32位
    rtc_port_base[1] = us >> 32;          // 高32位
  }
  
  // 注意：写操作和读低32位都不做任何事
  // 写操作被忽略（RTC是只读设备）
  // 读低32位直接返回上次更新的值
}

// ==================== 定时器中断处理 ====================
/**
 * @brief 定时器中断处理函数
 *
 * 被alarm机制周期性调用，触发设备中断
 */
static void timer_intr() {
  // 只在NEMU运行时触发中断
  if (nemu_state.state == NEMU_RUNNING) {
    extern void dev_raise_intr();  // 声明外部函数：触发中断
    dev_raise_intr();              // 向CPU发送中断
  }
}

// ==================== 设备初始化 ====================
/**
 * @brief 初始化RTC设备
 * 
 * 1. 分配8字节寄存器空间
 * 2. 注册I/O映射（端口I/O或MMIO）
 * 3. native环境下注册定时器中断处理
 */
void init_timer() {
  // ----- 1. 分配寄存器空间 -----
  rtc_port_base = (uint32_t *)new_space(8);
  
  // ----- 2. 注册I/O映射 -----
#ifdef CONFIG_HAS_PORT_IO
  // 端口I/O模式（x86）
  add_pio_map("rtc", CONFIG_RTC_PORT, rtc_port_base, 8, rtc_io_handler);
#else
  // MMIO模式（RISC-V/mips32）
  add_mmio_map("rtc", CONFIG_RTC_MMIO, rtc_port_base, 8, rtc_io_handler);
#endif

  // ----- 3. 注册定时器中断 -----
  add_alarm_handle(timer_intr);
}