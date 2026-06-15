#include <NDL.h>
#include <SDL.h>
#include <string.h>
#include <assert.h>

static SDL_AudioSpec audio_spec;
static int audio_opened = 0;
static int audio_paused = 1;

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
  if (audio_opened) return -1;

  memcpy(&audio_spec, desired, sizeof(SDL_AudioSpec));
  // Calculate buffer size if not set (samples * bytes_per_sample * channels)
  if (audio_spec.size == 0) {
    int bps = 1;
    if (audio_spec.format == AUDIO_S16SYS) bps = 2;
    audio_spec.size = audio_spec.samples * bps * audio_spec.channels;
  }
  if (obtained) memcpy(obtained, &audio_spec, sizeof(SDL_AudioSpec));

  NDL_OpenAudio(desired->freq, desired->channels, desired->samples);
  audio_opened = 1;
  return 0;
}

void SDL_CloseAudio() {
  if (!audio_opened) return;
  NDL_CloseAudio();
  audio_opened = 0;
}

void SDL_PauseAudio(int pause_on) {
  if (!audio_opened) return;
  audio_paused = pause_on;
}

// Called periodically from SDL_PollEvent / SDL_WaitEvent to feed audio
void sdl_audio_tick() {
  if (!audio_opened || audio_paused) return;
  if (!audio_spec.callback) return;

  int size = audio_spec.size;
  uint8_t *buf = malloc(size);
  if (!buf) return;

  memset(buf, 0, size);
  audio_spec.callback(audio_spec.userdata, buf, size);
  NDL_PlayAudio(buf, size);
  free(buf);
}

void SDL_MixAudio(uint8_t *dst, uint8_t *src, uint32_t len, int volume) {
  // Simple mix: copy with volume scaling
  if (volume == 0) return;
  for (uint32_t i = 0; i < len; i++) {
    int sample = (int)dst[i] + (int)((src[i] * volume) / SDL_MIX_MAXVOLUME);
    if (sample > 255) sample = 255;
    if (sample < 0) sample = 0;
    dst[i] = (uint8_t)sample;
  }
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec,
                            uint8_t **audio_buf, uint32_t *audio_len) {
  // WAV loading not implemented for this platform
  return NULL;
}

void SDL_FreeWAV(uint8_t *audio_buf) {
}

void SDL_LockAudio() {
  // Single-threaded: no lock needed
}

void SDL_UnlockAudio() {
  // Single-threaded: no lock needed
}
