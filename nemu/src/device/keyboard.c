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

#include <device/map.h>
#include <utils.h>

#define KEYDOWN_MASK 0x8000  // 按键状态标志（最高位：1=按下，0=释放）

#ifndef CONFIG_TARGET_AM
#include <SDL2/SDL.h>

// 定义 NEMU 内部使用的键码枚举,所有按键列表
// Note that this is not the standard
#define NEMU_KEYS(f) \
  f(ESCAPE) f(F1) f(F2) f(F3) f(F4) f(F5) f(F6) f(F7) f(F8) f(F9) f(F10) f(F11) f(F12) \
f(GRAVE) f(1) f(2) f(3) f(4) f(5) f(6) f(7) f(8) f(9) f(0) f(MINUS) f(EQUALS) f(BACKSPACE) \
f(TAB) f(Q) f(W) f(E) f(R) f(T) f(Y) f(U) f(I) f(O) f(P) f(LEFTBRACKET) f(RIGHTBRACKET) f(BACKSLASH) \
f(CAPSLOCK) f(A) f(S) f(D) f(F) f(G) f(H) f(J) f(K) f(L) f(SEMICOLON) f(APOSTROPHE) f(RETURN) \
f(LSHIFT) f(Z) f(X) f(C) f(V) f(B) f(N) f(M) f(COMMA) f(PERIOD) f(SLASH) f(RSHIFT) \
f(LCTRL) f(APPLICATION) f(LALT) f(SPACE) f(RALT) f(RCTRL) \
f(UP) f(DOWN) f(LEFT) f(RIGHT) f(INSERT) f(DELETE) f(HOME) f(END) f(PAGEUP) f(PAGEDOWN)

#define NEMU_KEY_NAME(k) NEMU_KEY_ ## k,

enum {
  NEMU_KEY_NONE = 0,
  MAP(NEMU_KEYS, NEMU_KEY_NAME)
};

#define SDL_KEYMAP(k) keymap[SDL_SCANCODE_ ## k] = NEMU_KEY_ ## k;
static uint32_t keymap[256] = {};

 // 将 SDL 扫描码映射到 NEMU 内部键码
static void init_keymap() {
  MAP(NEMU_KEYS, SDL_KEYMAP)
}

#define KEY_QUEUE_LEN 1024
static int key_queue[KEY_QUEUE_LEN] = {};
static int key_f = 0, key_r = 0; // 队列头和尾

// 入队
static void key_enqueue(uint32_t am_scancode) {
  key_queue[key_r] = am_scancode;
  key_r = (key_r + 1) % KEY_QUEUE_LEN;
  Assert(key_r != key_f, "key queue overflow!");
}

// 出队
static uint32_t key_dequeue() {
  uint32_t key = NEMU_KEY_NONE;
  if (key_f != key_r) {
    key = key_queue[key_f];
    key_f = (key_f + 1) % KEY_QUEUE_LEN;
  }
  return key;
}

void send_key(uint8_t scancode, bool is_keydown) {
  // 只在 NEMU 运行时且键码有效时才处理
  if (nemu_state.state == NEMU_RUNNING && keymap[scancode] != NEMU_KEY_NONE) {
    // 组合键码：高位置 1 表示按下，置 0 表示释放
    uint32_t am_scancode = keymap[scancode] | (is_keydown ? KEYDOWN_MASK : 0);
    key_enqueue(am_scancode);  // 入队
  }
}
#else // !CONFIG_TARGET_AM
#define NEMU_KEY_NONE 0

static uint32_t key_dequeue() {
  // 通过 AM 的 IOE 读取键盘输入
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  // 组合键码：高位置 1 表示按下
  uint32_t am_scancode = ev.keycode | (ev.keydown ? KEYDOWN_MASK : 0);
  return am_scancode;
}
#endif

static uint32_t *i8042_data_port_base = NULL; // 模拟的数据寄存器（4字节）

static void i8042_data_io_handler(uint32_t offset, int len, bool is_write) {
  assert(!is_write);          // 键盘只支持读操作
  assert(offset == 0);         // 只有数据寄存器（偏移 0）被实现
  i8042_data_port_base[0] = key_dequeue();  // 从队列取出一个按键
}

void init_i8042() {
  // 1. 分配 4 字节空间作为键盘数据寄存器
  i8042_data_port_base = (uint32_t *)new_space(4);
  i8042_data_port_base[0] = NEMU_KEY_NONE;  // 初始无按键

#ifdef CONFIG_HAS_PORT_IO
  // 2. 端口 I/O 模式：使用端口映射
  add_pio_map("keyboard", CONFIG_I8042_DATA_PORT, 
              i8042_data_port_base, 4, i8042_data_io_handler);
#else
  // 3. MMIO 模式：使用内存映射
  add_mmio_map("keyboard", CONFIG_I8042_DATA_MMIO, 
               i8042_data_port_base, 4, i8042_data_io_handler);
#endif

  // 4. native 环境下初始化键码映射
  IFNDEF(CONFIG_TARGET_AM, init_keymap());
}