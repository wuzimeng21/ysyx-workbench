#include <am.h>
#include <SDL.h>

#define KEYDOWN_MASK 0x8000

#define KEY_QUEUE_LEN 1024
static int key_queue[KEY_QUEUE_LEN] = {};
static int key_f = 0, key_r = 0;
static SDL_mutex *key_queue_lock = NULL;

#define XX(k) [SDL_SCANCODE_##k] = AM_KEY_##k,
static int keymap[256] = {
  AM_KEYS(XX)
};

static int event_thread(void *args) {
  SDL_Event event;
  while (1) {
    SDL_WaitEvent(&event);
    switch (event.type) {
      case SDL_QUIT: halt(0);
      case SDL_KEYDOWN:
      case SDL_KEYUP: {
        SDL_Keysym k = event.key.keysym;
        int keydown = event.key.type == SDL_KEYDOWN;
        int scancode = k.scancode;
        if (keymap[scancode] != 0) {
          int am_code = keymap[scancode] | (keydown ? KEYDOWN_MASK : 0);
          SDL_LockMutex(key_queue_lock);
          key_queue[key_r] = am_code;
          key_r = (key_r + 1) % KEY_QUEUE_LEN;
          SDL_UnlockMutex(key_queue_lock);
          void __am_send_kbd_intr();
          __am_send_kbd_intr();
        }
        break;
      }
    }
  }
}

void __am_input_init() {
  key_queue_lock = SDL_CreateMutex();
  SDL_CreateThread(event_thread, "event thread", NULL);
}

void __am_input_config(AM_INPUT_CONFIG_T *cfg) {
  cfg->present = true;
}

/**
 * @file input.c
 * @brief AM 键盘输入实现
 * 
 * 这个函数从键盘队列中读取按键数据，并返回给应用程序。
 * 键盘队列由 NEMU 的 send_key() 函数填充。
 */

/**
 * @brief 读取键盘输入
 * @param kbd 指向 AM_INPUT_KEYBRD_T 结构体的指针，用于返回按键信息
 * 
 * 函数流程：
 * 1. 从环形队列中安全地取出一个按键事件
 * 2. 解析事件中的按键状态和键码
 * 3. 填充到 kbd 结构体中
 * 
 * 队列数据结构：
 *   key_queue[] - 环形队列存储按键事件
 *   key_f - 队首索引（下一个要读取的位置）
 *   key_r - 队尾索引（下一个要写入的位置）
 *   key_queue_lock - 保护队列的互斥锁
 * 
 *   KEYDOWN_MASK = 0x8000 表示最高位为按下状态
 */
void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
    // ----- 1. 初始化返回值 -----
    // 默认无按键事件
    int k = AM_KEY_NONE;

    // ----- 2. 从队列安全读取数据 -----
    // 加锁防止多线程竞争（SDL事件线程和AM读取线程）
    SDL_LockMutex(key_queue_lock);
    
    // 检查队列是否非空 (key_f != key_r 表示队列有数据)
    if (key_f != key_r) {
        // 从队首取出一个按键事件
        k = key_queue[key_f];
        
        // 移动队首指针（环形队列）
        // 当到达数组末尾时，回到开头
        key_f = (key_f + 1) % KEY_QUEUE_LEN;
    }
    
    // 解锁
    SDL_UnlockMutex(key_queue_lock);

    // ----- 3. 解析按键数据 -----
    // 按键状态：检查最高位
    // KEYDOWN_MASK = 0x8000 (二进制 1000 0000 0000 0000)
    // 如果该位为1，表示按键按下；为0表示释放
    kbd->keydown = (k & KEYDOWN_MASK) ? true : false;
    
    // 键码：去掉最高位，保留低16位
    // ~KEYDOWN_MASK = 0x7FFF (二进制 0111 1111 1111 1111)
    kbd->keycode = k & ~KEYDOWN_MASK;
}