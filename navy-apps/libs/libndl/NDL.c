#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int evtdev = -1;      // 事件设备文件描述符
static int fbdev = -1;       // 帧缓冲设备文件描述符
static int screen_w = 0, screen_h = 0;  // 屏幕宽度和高度
static int rect_x = 0, rect_y = 0;  

uint32_t NDL_GetTicks() {
  struct timeval tv;
  assert(gettimeofday(&tv, NULL) == 0);
  uint32_t ret = tv.tv_sec * 1000 + tv.tv_usec;
  return ret;
}

// 读出一条事件信息, 将其写入`buf`中, 最长写入`len`字节
// 若读出了有效的事件, 函数返回1, 否则返回0
int NDL_PollEvent(char *buf, int len) {

  int read_ret = read(evtdev, buf, len);
  if(read_ret <= 0) return 0;
  return 1;
}

/**
 * NDL_OpenCanvas - 打开画布（NWM环境下的特殊处理）
 * @param w 画布宽度指针（输入输出参数）
 * @param h 画布高度指针（输入输出参数）
 * 
 * 功能：在NWM环境下，与窗口管理器交互创建指定大小的画布
 * 说明：如果是在NWM环境下运行，会通过固定的文件描述符与NWM通信
 *       非NWM环境下的处理需要自行添加
 */
void NDL_OpenCanvas(int *w, int *h) {
    if (getenv("NWM_APP")) {
        int fbctl = 4;    // 帧缓冲控制设备（用于发送控制命令）
        fbdev = 5;        // 帧缓冲设备（用于实际绘图）
        screen_w = *w; 
        screen_h = *h;
        // ========== 步骤1：向NWM发送画布大小 ==========
        char buf[64];
        int len = sprintf(buf, "%d %d", screen_w, screen_h);
        // 向fbctl写入"宽度 高度"，通知NWM调整窗口大小并创建帧缓冲
        write(fbctl, buf, len);
        // ========== 步骤2：等待NWM确认 ==========
        while (1) {
            // 从事件设备（fd=3）读取NWM发来的消息
            int nread = read(3, buf, sizeof(buf) - 1);
            // 如果没有读到数据，继续等待
            if (nread <= 0) continue;
            // 添加字符串结束符
            buf[nread] = '\0';           
            // 检查是否收到确认消息
            if (strcmp(buf, "mmap ok") == 0) {
                break;
            }
        }
        close(fbctl);
    }
    else {
      int dispinfo_fd = open("/proc/dispinfo", 0);
      int screen_h = *h, screen_w = *w;
      // read w && h from /proc/dispinfo
      char buf[200];
      int read_ret = read(dispinfo_fd, buf, sizeof(buf) / sizeof(buf[0]));
      bool present, has_accel;
      int width, height, vmemsz;
      sscanf(buf, "PRESENT:%d\nHAS_ACCEL:%d\nWIDTH:%d\nHEIGHT:%d\nVMEMS:%d\n\0",
      present, has_accel, width, height, vmemsz);
      if(screen_h == 0 && screen_w == 0) {
        screen_h = height, screen_w = width;
      }
      assert(screen_h <= height);
      assert(screen_w <= width);
      fbdev = open(file_table[FD_FB].name, 0);
      rect_x = (screen_w - *w) / 2;
      rect_y = (screen_h - *h) / 2;
      return ;

    }
}

// 向画布`(x, y)`坐标处绘制`w*h`的矩形图像, 并将该绘制区域同步到屏幕上
// 图像像素按行优先方式存储在`pixels`中, 每个像素用32位整数以`00RRGGBB`的方式描述颜色
void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  int tx = x, ty = y;
  // int pixels_sz = sizeof(pixels) / sizeof(pixels[0]);
  tx += rect_x;
  ty += rect_y;
  for(int i = 0; i < h; i ++) {
    int offset = ((ty + i) + tx) * sizeof(pixels[0]);
    leek(fbdev, offset, SEEK_SET);
    write(fbdev, pixels, w * sizeof(pixels[0]));
    pixels += w;
  }
  
  return ;
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  else {
    evtdev = open("/dev/events", 0, 0);
    assert(evtdev >= 0);

  }
  return 0;
}

void NDL_Quit() {
}
