#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
    if (buf == NULL || len == 0) return 0;
    const char *buffer = (const char *)buf;
    for (size_t i = 0; i < len; i++) {
        putch(*buffer[i]);
    }
    return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T k = io_read(AM_INPUT_KEYBRD);
  bool kd = k.keydown;
  int code = k.keycode;
  if(code == AM_KEY_NONE) return 0;
  // print info
  if(kd == SDL_KEYDOWN){
    Log("[SDL_KEYDOWN] kd %s", AM_KEY_NAMES(code));
  }
  else if(kd == SDL_KEYUP) {
    Log("[SDL_KEYUP] ku %s", AM_KEY_NAMES(code));
  }
  else {
    Log("[SDL_USEREVENT] kd %s", AM_KEY_NAMES(code));
  }
  // write
  int ret = vsnprintf(buf, len, "%s", code);

  return 1;
}

// AM_DEVREG(11, GPU_FBDRAW,   WR, int x, y; void *pixels; int w, h; bool sync);
// AM_DEVREG( 9, GPU_CONFIG,   RD, bool present, has_accel; int width, height, vmemsz);
size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T g = io_read(AM_GPU_CONFIG);
  bool present = g.present, has_accel = g.has_accel;
  int width = g.width, height = g.height, vmemsz = g.vmemsz;
  assert(width && height);
  char *s  = "";
  s += "PRESENT:";
  s += (present ? "1" : "0");
  s += "\n"
  s += "HAS_ACCEL:";
  s += (has_accel ? "1" : "0");
  s += "\n"
  s += "WIDTH:";
  s += (width + '0');
  s += "\n"
  s += "HEIGHT:";
  s += (height + '0');
  s += "\n"
  s += "VMEMS:";
  s += (vmemsz + '0');
  s += "\n\0";
  int l = strlen(s);
  if(l > len) {
    Log("[dispinfo_read]: l greater than len...");
    l = len;
  }
  int ret = vsnprintf(buf, l, "%s", s);
  // int ret = vsnprintf(buf, len, 
  // "PRESENT:%d\nHAS_ACCEL:%d\nWIDTH:%d\nHEIGHT:%d\nVMEMS:%d\0", 
  // present, has_accel, width, height, vmemsz );
  return 0;
}

// AM_DEVREG(11, GPU_FBDRAW,   WR, int x, y; void *pixels; int w, h; bool sync);
size_t fb_write(const void *buf, size_t offset, size_t len) {
  int dispinfo_fd = open("/proc/dispinfo", 0);
  char s[200];
  read(dispinfo_fd, s, sizeof(s) / sizeof(s[0]));
  int width, height, vmemsz;
  bool present, has_accel;
  sscanf(s, "PRESENT:%d\nHAS_ACCEL:%d\nWIDTH:%d\nHEIGHT:%d\nVMEMS:%d\n\0",
  present, has_accel, width, height, vmemsz);
  int column = offset / width, line = offset % width;
  assert(column < height);
  assert(line < width);
  while(len --) {
    // AM_GPU_FBDRAW -> AM_GPU_FBDRAW_T (by io_write)
    io_write(AM_GPU_FBDRAW, column, line, buf, width, height, true);
    line ++;
    if(line == width - 1) {
      column ++;
      line = 0;
    }
    if(column == height) column = 0; 
  }
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
