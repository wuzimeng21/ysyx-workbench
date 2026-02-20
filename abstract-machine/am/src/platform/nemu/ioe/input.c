#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  // kbd->keydown = 0;
  // kbd->keycode = AM_KEY_NONE;
  // write for PA2
  int k = AM_KEY_NONE;

  // SDL_LockMutex(key_queue_lock);
  // if (key_f != key_r) {
  //   k = key_queue[key_f];
  //   key_f = (key_f + 1) % KEY_QUEUE_LEN;
  // }
  // SDL_UnlockMutex(key_queue_lock);

  k = inl(KBD_ADDR);

  kbd->keydown = (k & KEYDOWN_MASK ? true : false);
  kbd->keycode = k & ~KEYDOWN_MASK;
  
  return ;
}


