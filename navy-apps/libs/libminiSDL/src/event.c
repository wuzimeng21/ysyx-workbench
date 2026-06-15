#include <NDL.h>
#include <SDL.h>
#include <string.h>

extern void sdl_audio_tick();

#define keyname(k) #k,

static const char *keynames[] = {
  "NONE",
  _KEYS(keyname)
};

#define NR_KEYS 256
static uint8_t key_state[NR_KEYS] = {0};

// Parse NDL event buffer (format: "kd KEYNAME\n" or "ku KEYNAME\n")
// into SDL_Event. Updates key_state array. Returns 1 on success.
static int parse_event(const char *buf, SDL_Event *event) {
  if (buf[0] != 'k' || (buf[1] != 'd' && buf[1] != 'u'))
    return 0;

  event->key.type = (buf[1] == 'd') ? SDL_KEYDOWN : SDL_KEYUP;

  // Extract key name (skip "kd " or "ku ")
  const char *name = buf + 3;

  // Search keynames table
  for (int i = 0; i < sizeof(keynames) / sizeof(keynames[0]); i++) {
    size_t len = strlen(keynames[i]);
    if (strncmp(name, keynames[i], len) == 0 && name[len] == '\n') {
      event->key.keysym.sym = i;
      key_state[i] = (event->key.type == SDL_KEYDOWN) ? 1 : 0;
      return 1;
    }
  }
  return 0;
}

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  char buf[16];
  if (NDL_PollEvent(buf, sizeof(buf))) {
    return parse_event(buf, ev);
  }
  sdl_audio_tick();
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[16];
  while (1) {
    if (NDL_PollEvent(buf, sizeof(buf))) {
      if (parse_event(buf, event))
        return 1;
    }
    sdl_audio_tick();
  }
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  if (numkeys) *numkeys = NR_KEYS;
  return key_state;
}
