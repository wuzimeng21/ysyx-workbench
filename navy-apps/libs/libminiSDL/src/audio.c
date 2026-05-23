#include <NDL.h>
#include <SDL.h>

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
  panic("Not implement");
  return 0;
}

void SDL_CloseAudio() {
  panic("Not implement");
}

void SDL_PauseAudio(int pause_on) {
  panic("Not implement");
}

void SDL_MixAudio(uint8_t *dst, uint8_t *src, uint32_t len, int volume) {
  panic("Not implement");
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, uint8_t **audio_buf, uint32_t *audio_len) {
  panic("Not implement");
  return NULL;
}

void SDL_FreeWAV(uint8_t *audio_buf) {
  panic("Not implement");
}

void SDL_LockAudio() {
  panic("Not implement");
}

void SDL_UnlockAudio() {
  panic("Not implement");
}
