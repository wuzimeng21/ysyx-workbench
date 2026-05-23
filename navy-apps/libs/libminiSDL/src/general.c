#include <NDL.h>

int SDL_Init(uint32_t flags) {
  return NDL_Init(flags);
}

void SDL_Quit() {
  NDL_Quit();
}

char *SDL_GetError() {
  panic("Not implement");
  return "Navy does not support SDL_GetError()";
}

int SDL_SetError(const char* fmt, ...) {
  panic("Not implement");
  return -1;
}

int SDL_ShowCursor(int toggle) {
  panic("Not implement");
  return 0;
}

void SDL_WM_SetCaption(const char *title, const char *icon) {
  panic("Not implement");
}
