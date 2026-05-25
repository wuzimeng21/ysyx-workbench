#include <NDL.h>
#include <SDL.h>
#include <string.h>

#define keyname(k) #k,

static const char *keynames[] = {
  "NONE",
  _KEYS(keyname)
};

// Parse NDL event buffer (format: "kd KEYNAME\n" or "ku KEYNAME\n")
// into SDL_Event. Returns 1 on success.
static int parse_event(const char *buf, SDL_Event *event) {
  if (buf[0] != 'k' || (buf[1] != 'd' && buf[1] != 'u'))
    return 0;

  event->key.type = (buf[1] == 'd') ? SDL_KEYDOWN : SDL_KEYUP;

  // Extract key name (skip "kd " or "ku ")
  const char *name = buf + 3;

  // Search keynames table
  for (int i = 0; i < sizeof(keynames) / sizeof(keynames[0]); i++) {
    if (strncmp(name, keynames[i], strlen(keynames[i])) == 0) {
      event->key.keysym.sym = i;
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
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[16];
  while (1) {
    if (NDL_PollEvent(buf, sizeof(buf))) {
      if (parse_event(buf, event))
        return 1;
    }
  }
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return NULL;
}
