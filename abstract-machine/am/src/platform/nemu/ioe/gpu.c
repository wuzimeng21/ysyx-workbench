#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4) // 同步寄存器

void __am_gpu_init() {
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t size = inl(VGACTL_ADDR);
  int w = size & 0xffff;
  int h = size >> 16;
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = w, .height = h,
    .vmemsz = w * h * sizeof(uint32_t)
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;

  if (w > 0 && h > 0) {
    uint32_t *pixels = (uint32_t *)ctl->pixels;
    uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
    int sw = inl(VGACTL_ADDR) & 0xffff;

    for (int j = 0; j < h; j ++) {
      for (int i = 0; i < w; i ++) {
        fb[(y + j) * sw + x + i] = pixels[j * w + i];
      }
    }
  }

  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
