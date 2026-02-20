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
 * @file alarm.c
 * @brief 虚拟定时器（Alarm）机制实现
 * 
 * 使用 Linux 的 setitimer 和信号机制，实现周期性回调功能。
 * 设备可以注册自己的回调函数，定时器到期时会调用所有注册的函数。
 */

#include <common.h>
#include <device/alarm.h>
#include <sys/time.h>
#include <signal.h>

// ==================== 全局定义 ====================

#define MAX_HANDLER 8  // 最多支持 8 个回调函数

// 存储所有注册的回调函数指针
static alarm_handler_t handler[MAX_HANDLER] = {};
static int idx = 0;  // 当前已注册的回调数量

// ==================== 回调注册接口 ====================

/**
 * @brief 注册定时器回调函数
 * 
 * 设备调用此函数，将自己的更新函数注册到定时器。
 * 定时器每次到期时，会依次调用所有注册的函数。
 * 
 * @param h 要注册的回调函数指针（无参数、无返回值）
 */
void add_alarm_handle(alarm_handler_t h) {
  // 检查是否超过最大数量
  assert(idx < MAX_HANDLER);
  
  // 存入数组，并更新计数
  handler[idx ++] = h;
}

// ==================== 信号处理函数 ====================

/**
 * @brief SIGVTALRM 信号的处理函数
 * 
 * 定时器到期时，内核发送 SIGVTALRM 信号，此函数被调用。
 * 它会遍历并执行所有已注册的回调函数。
 * 
 * @param signum 信号编号（此处为 SIGVTALRM，未使用）
 */
static void alarm_sig_handler(int signum) {
  int i;
  // 依次调用所有注册的回调函数
  for (i = 0; i < idx; i ++) {
    handler[i]();  // 执行回调
  }
}

// ==================== 定时器初始化 ====================

/**
 * @brief 初始化虚拟定时器
 * 
 * 1. 设置 SIGVTALRM 信号的处理函数
 * 2. 配置周期性定时器（频率由 TIMER_HZ 决定）
 * 
 * 定时器开始运行后，会以固定频率发送 SIGVTALRM 信号，
 * 触发 alarm_sig_handler() 执行所有回调。
 */
void init_alarm() {
  // ----- 1. 设置信号处理函数 -----
  struct sigaction s;
  memset(&s, 0, sizeof(s));
  s.sa_handler = alarm_sig_handler;  // 指定信号处理函数
  
  // 注册 SIGVTALRM 信号的处理
  int ret = sigaction(SIGVTALRM, &s, NULL);
  Assert(ret == 0, "Can not set signal handler");

  // ----- 2. 配置周期性定时器 -----
  struct itimerval it = {};
  
  // 首次到期时间：立即开始，周期为 1000000 / TIMER_HZ 微秒
  it.it_value.tv_sec = 0;
  it.it_value.tv_usec = 1000000 / TIMER_HZ;
  
  // 后续间隔：与首次相同（周期性）
  it.it_interval = it.it_value;
  
  // 设置虚拟定时器（ITIMER_VIRTUAL：只在进程用户态运行时计时）
  ret = setitimer(ITIMER_VIRTUAL, &it, NULL);
  Assert(ret == 0, "Can not set timer");
}