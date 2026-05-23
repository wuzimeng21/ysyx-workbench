#include <NDL.h>
#include <sdl-timer.h>
#include <stdio.h>

SDL_TimerID SDL_AddTimer(uint32_t interval, SDL_NewTimerCallback callback, void *param) {
  panic("Not implement");    
  return NULL;
}

int SDL_RemoveTimer(SDL_TimerID id) {
  panic("Not implement");
  return 1;
}


// uint32_t NDL_GetTicks() {
//   struct timeval tv;
//   assert(gettimeofday(&tv, NULL) == 0);
//   uint32_t ret = tv.tv_sec * 1000 + tv.tv_usec;
//   return ret;
// }

// SDL_GetTicks(): 它和NDL_GetTicks()的功能类似, 但有一个额外的小要求, 具体请RTFM
uint32_t SDL_GetTicks() {
  uint32_t ret = NDL_GetTicks(); // <= 49.7 days
  return ret;
}

void SDL_Delay(uint32_t ms) {
  panic("Not implement");
}
