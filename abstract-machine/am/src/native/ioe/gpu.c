#include <am.h>
#include <SDL.h>
#include <fenv.h>

//#define MODE_800x600
#define WINDOW_W 800
#define WINDOW_H 600
#ifdef MODE_800x600
const int disp_w = WINDOW_W, disp_h = WINDOW_H;
#else
const int disp_w = 400, disp_h = 300;
#endif

#define FPS   60

#define RMASK 0x00ff0000
#define GMASK 0x0000ff00
#define BMASK 0x000000ff
#define AMASK 0x00000000

static SDL_Window *window = NULL;
static SDL_Surface *surface = NULL;

static Uint32 texture_sync(Uint32 interval, void *param) {
  SDL_BlitScaled(surface, NULL, SDL_GetWindowSurface(window), NULL);
  SDL_UpdateWindowSurface(window);
  return interval;
}

void __am_gpu_init() {
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
  window = SDL_CreateWindow("Native Application",
      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      WINDOW_W, WINDOW_H, SDL_WINDOW_OPENGL);
  surface = SDL_CreateRGBSurface(SDL_SWSURFACE, disp_w, disp_h, 32,
      RMASK, GMASK, BMASK, AMASK);
  SDL_AddTimer(1000 / FPS, texture_sync, NULL);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = disp_w, .height = disp_h,
    .vmemsz = 0
  };
}

void __am_gpu_status(AM_GPU_STATUS_T *stat) {
  stat->ready = true;
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
    // ----- 1. 提取参数 -----
    int x = ctl->x;        // 起始 x 坐标
    int y = ctl->y;        // 起始 y 坐标
    int w = ctl->w;        // 矩形宽度
    int h = ctl->h;        // 矩形高度
    
    // ----- 2. 参数检查 -----
    // 如果宽或高为 0，没有需要绘制的像素，直接返回
    if (w == 0 || h == 0) return;
    
    // ----- 3. 清除浮点异常（SDL 内部可能产生）-----
    // feclearexcept(-1) 清除所有浮点异常标志
    // 防止 SDL 操作引起的浮点异常影响程序
    feclearexcept(-1);
    
    // ----- 4. 创建 SDL 表面 -----
    // SDL_CreateRGBSurfaceFrom() 从内存缓冲区创建 SDL 表面
    // 参数说明：
    //   - ctl->pixels: 像素数据源（应用程序提供的缓冲区）
    //   - w, h: 图像的宽度和高度（像素）
    //   - 32: 每像素位数（32位 RGBA）
    //   - w * sizeof(uint32_t): 每行字节数（行跨度/步幅）
    //   - RMASK, GMASK, BMASK, AMASK: 颜色掩码，定义像素格式
    SDL_Surface *s = SDL_CreateRGBSurfaceFrom(
        ctl->pixels,       // 像素数据源
        w, h,              // 图像尺寸
        32,                // 每像素位数
        w * sizeof(uint32_t), // 每行字节数（行跨度）
        RMASK, GMASK, BMASK, AMASK // 颜色掩码
    );
    
    // ----- 5. 定义目标矩形 -----
    // 指定将图像绘制到屏幕上的哪个位置
    SDL_Rect rect = {
        .x = x,  // 目标 x 坐标
        .y = y   // 目标 y 坐标
    };
    // 注意：.w 和 .h 未设置，SDL_BlitSurface 会使用源表面的宽高
    
    // ----- 6. 执行位块传输（blit）-----
    // 将源表面 s 的像素数据复制到目标表面 surface
    // 参数：
    //   - s: 源表面（应用程序提供的图像）
    //   - NULL: 源矩形（NULL 表示整个表面）
    //   - surface: 目标表面（屏幕的像素缓冲区）
    //   - &rect: 目标矩形位置
    SDL_BlitSurface(s, NULL, surface, &rect);
    
    // ----- 7. 释放源表面 -----
    // 创建的表面使用完后必须释放，防止内存泄漏
    SDL_FreeSurface(s);
    
    // 注意：此实现忽略了 ctl->sync 参数
    // 屏幕的最终更新可能由其他机制触发（如 SDL_Flip 或 SDL_UpdateWindowSurface）
}