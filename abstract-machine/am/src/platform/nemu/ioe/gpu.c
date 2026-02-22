#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4) // 同步寄存器

void __am_gpu_init() {
  int i;
  // TODO: 从硬件获取正确的宽高
  // 屏幕大小寄存器:高16位存高度，低16位存宽度
  int w = inl(VGACTL_ADDR) & 0xffff;  // 应该从 VGACTL_ADDR 读取
  int h = (inl(VGACTL_ADDR) >> 16) & 0xffff;  // 应该从 VGACTL_ADDR 读取
  
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (i = 0; i < w * h; i ++) fb[i] = i;  // 填充测试图案
  outl(SYNC_ADDR, 1);  // 触发屏幕更新
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = 0, .height = 0,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
//  AM_DEVREG(11, GPU_FBDRAW,   WR, int x, y; void *pixels; int w, h; bool sync);
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  // if(!ctl->sync || w == 0 || h == 0) return ;
  // 无论 sync 是什么，都要绘制，只是可能不同步m
  if( w == 0 || h == 0) return ;
  uint32_t * pixels = (uint32_t *)ctl->pixels;
  uint32_t *fb_content = (uint32_t *)(uintptr_t)FB_ADDR;
  int sw = inl(VGACTL_ADDR) & 0xffff;  // 应该从 VGACTL_ADDR 读取
  // int sh = (inl(VGACTL_ADDR) >> 16) & 0xffff;  // 应该从 VGACTL_ADDR 读取

  for(int i = 0; i < w; i ++) {
    for(int j = 0; j < h; j ++) {
      fb_content[(y + j) * sw + x + i] = pixels[j * w + i];
    }
  }
  if(ctl->sync) {
    outl(SYNC_ADDR, 1); // 同步寄存器只需要写入非 0 值（通常是 1）来触发屏幕刷新，不应该写入像素数据。
  }
  return ;
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
