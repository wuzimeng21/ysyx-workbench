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
    putch(buffer[i]);
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T k = io_read(AM_INPUT_KEYBRD);
  if (k.keycode == AM_KEY_NONE) return 0;
  return snprintf(buf, len, "%s %s\n", k.keydown ? "kd" : "ku", keyname[k.keycode]);
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T g = io_read(AM_GPU_CONFIG);
  return snprintf(buf, len,
    "PRESENT:%d\nHAS_ACCEL:%d\nWIDTH:%d\nHEIGHT:%d\nVMEMS:%d\n",
    g.present, g.has_accel, g.width, g.height, g.vmemsz);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T g = io_read(AM_GPU_CONFIG);
  int w = g.width;
  int pixel_offset = offset / sizeof(uint32_t);
  int x = pixel_offset % w;
  int y = pixel_offset / w;
  io_write(AM_GPU_FBDRAW, x, y, (void *)buf, len / sizeof(uint32_t), 1, false);
  io_write(AM_GPU_FBDRAW, 0, 0, NULL, 0, 0, true);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
