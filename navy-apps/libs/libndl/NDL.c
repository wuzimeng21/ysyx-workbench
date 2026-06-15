#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int rect_x = 0, rect_y = 0;

uint32_t NDL_GetTicks() {
  struct timeval tv;
  assert(gettimeofday(&tv, NULL) == 0);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  int read_ret = read(evtdev, buf, len);
  if (read_ret <= 0) return 0;
  return 1;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w;
    screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    write(fbctl, buf, len);
    while (1) {
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  } else {
    int dispinfo_fd = open("/proc/dispinfo", 0);
    assert(dispinfo_fd >= 0);

    char buf[200];
    int read_ret = read(dispinfo_fd, buf, sizeof(buf) - 1);
    assert(read_ret > 0);
    buf[read_ret] = '\0';

    int present, has_accel;
    int width, height, vmemsz;
    sscanf(buf, "PRESENT:%d\nHAS_ACCEL:%d\nWIDTH:%d\nHEIGHT:%d\nVMEMS:%d\n",
           &present, &has_accel, &width, &height, &vmemsz);

    close(dispinfo_fd);

    if (*h == 0 && *w == 0) {
      *h = height;
      *w = width;
    }
    assert(*h <= height);
    assert(*w <= width);

    fbdev = open("/dev/fb", 0);
    assert(fbdev >= 0);

    screen_w = width;
    screen_h = height;
    rect_x = (screen_w - *w) / 2;
    rect_y = (screen_h - *h) / 2;
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  int tx = x + rect_x;
  int ty = y + rect_y;
  for (int i = 0; i < h; i++) {
    int offset = ((ty + i) * screen_w + tx) * sizeof(uint32_t);
    lseek(fbdev, offset, SEEK_SET);
    write(fbdev, pixels + i * w, w * sizeof(uint32_t));
  }
}

static int audio_fd = -1;

void NDL_OpenAudio(int freq, int channels, int samples) {
  audio_fd = open("/dev/audio", 0, 0);
}

void NDL_CloseAudio() {
  if (audio_fd >= 0) {
    close(audio_fd);
    audio_fd = -1;
  }
}

int NDL_PlayAudio(void *buf, int len) {
  if (audio_fd < 0) return 0;
  return write(audio_fd, buf, len);
}

int NDL_QueryAudio() {
  if (audio_fd < 0) return 0;
  char buf[32];
  int n = read(audio_fd, buf, sizeof(buf));
  if (n <= 0) return 0;
  return atoi(buf);
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  } else {
    evtdev = open("/dev/events", 0, 0);
    assert(evtdev >= 0);
  }
  return 0;
}

void NDL_Quit() {}
