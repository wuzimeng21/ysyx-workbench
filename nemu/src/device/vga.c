/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/
// VGA设备
// ├── 控制寄存器 (vgactl_port_base)
// │   ├── 偏移0: 屏幕尺寸（高16位高度，低16位宽度）
// │   └── 偏移4: 同步寄存器（TODO）
// │
// ├── 显存 (vmem)
// │   ├── 大小：width × height × 4 bytes
// │   ├── 格式：ARGB8888
// │   └── 映射地址：CONFIG_FB_ADDR
// │
// └── 显示实现
//     ├── native: SDL窗口
//     └── AM: 通过IOE输出



/**
 * @file vga.c
 * @brief VGA显示设备模拟
 * 
 * 实现一个简单的VGA控制器，包含：
 * - 显存(vmem)：通过MMIO映射，供CPU写入像素数据
 * - 控制寄存器(vgactl)：存储屏幕尺寸信息
 * - 屏幕更新：通过SDL或AM的IOE显示
 */

#include <common.h>
#include <device/map.h>

// ==================== 屏幕尺寸定义 ====================
// 根据配置选择分辨率：800x600 或 400x300
#define SCREEN_W (MUXDEF(CONFIG_VGA_SIZE_800x600, 800, 400))
#define SCREEN_H (MUXDEF(CONFIG_VGA_SIZE_800x600, 600, 300))

/**
 * @brief 获取屏幕宽度
 * 
 * AM环境：通过IOE读取实际配置
 * native环境：使用编译时常量
 */
static uint32_t screen_width() {
  return MUXDEF(CONFIG_TARGET_AM, 
                io_read(AM_GPU_CONFIG).width,   // AM: 从GPU配置读取
                SCREEN_W);                       // native: 使用宏定义
}

/**
 * @brief 获取屏幕高度
 */
static uint32_t screen_height() {
  return MUXDEF(CONFIG_TARGET_AM,
                io_read(AM_GPU_CONFIG).height,
                SCREEN_H);
}

/**
 * @brief 计算显存总大小（字节）
 * 
 * 显存大小 = 宽度 × 高度 × 每像素4字节（ARGB8888格式）
 */
static uint32_t screen_size() {
  return screen_width() * screen_height() * sizeof(uint32_t);
}

// ==================== 全局变量 ====================
static void *vmem = NULL;              // 显存空间
static uint32_t *vgactl_port_base = NULL; // 控制寄存器基址

// ==================== 屏幕显示相关 ====================
#ifdef CONFIG_VGA_SHOW_SCREEN  // 只有开启显示才编译这部分

#ifndef CONFIG_TARGET_AM  // native环境：使用SDL
#include <SDL2/SDL.h>

static SDL_Renderer *renderer = NULL;  // SDL渲染器
static SDL_Texture *texture = NULL;    // SDL纹理

/**
 * @brief 初始化SDL窗口和渲染器
 */
static void init_screen() {
  SDL_Window *window = NULL;
  char title[128];
  sprintf(title, "%s-NEMU", str(__GUEST_ISA__));
  
  // 初始化SDL
  SDL_Init(SDL_INIT_VIDEO);
  
  // 创建窗口和渲染器（400x300分辨率下放大2倍显示）
  SDL_CreateWindowAndRenderer(
      SCREEN_W * (MUXDEF(CONFIG_VGA_SIZE_400x300, 2, 1)),
      SCREEN_H * (MUXDEF(CONFIG_VGA_SIZE_400x300, 2, 1)),
      0, &window, &renderer);
  
  SDL_SetWindowTitle(window, title);
  
  // 创建纹理（ARGB8888格式）
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
      SDL_TEXTUREACCESS_STATIC, SCREEN_W, SCREEN_H);
  
  SDL_RenderPresent(renderer);
}

/**
 * @brief 更新屏幕显示
 * 
 * 将显存内容更新到SDL纹理并渲染
 */
static inline void update_screen() {
  // 更新纹理数据
  SDL_UpdateTexture(texture, NULL, vmem, SCREEN_W * sizeof(uint32_t));
  
  // 清空渲染器并复制纹理
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  
  // 显示到窗口
  SDL_RenderPresent(renderer);
}

#else  // AM环境：通过IOE输出
/**
 * @brief AM环境不需要初始化
 */
static void init_screen() {}

/**
 * @brief AM环境通过IOE更新屏幕
 * 
 * 调用AM的GPU驱动，将显存内容绘制到屏幕上
 */
static inline void update_screen() {
  io_write(AM_GPU_FBDRAW, 
           0, 0,                    // 起始坐标(0,0)
           vmem,                     // 像素数据
           screen_width(),           // 宽度
           screen_height(),          // 高度
           true);                    // 立即显示
}
#endif  // CONFIG_TARGET_AM

#endif  // CONFIG_VGA_SHOW_SCREEN

// ==================== VGA更新接口 ====================
/**
 * @brief VGA屏幕更新函数
 * 
 * TODO: 当同步寄存器非零时调用update_screen()，然后将同步寄存器清零
 * 
 * 这个函数会被device_update()定期调用
 */
void vga_update_screen() {
  if (vgactl_port_base && vgactl_port_base[1]) {
    vgactl_port_base[1] = 0;
    IFDEF(CONFIG_VGA_SHOW_SCREEN, update_screen());
  }
}

// ==================== VGA初始化 ====================
/**
 * @brief 初始化VGA设备
 * 
 * 1. 创建控制寄存器空间（8字节）
 * 2. 创建显存空间
 * 3. 注册MMIO/端口映射
 * 4. 初始化显示（如果开启）
 */
void init_vga() {
  // ----- 1. 分配控制寄存器空间 -----
  vgactl_port_base = (uint32_t *)new_space(8);
  
  // 控制寄存器格式：高16位存高度，低16位存宽度
  vgactl_port_base[0] = (screen_height() << 16) | screen_width();

  // ----- 2. 注册控制寄存器映射 -----
#ifdef CONFIG_HAS_PORT_IO
  // 端口I/O模式（x86）
  add_pio_map("vgactl", CONFIG_VGA_CTL_PORT, vgactl_port_base, 8, NULL);
#else
  // MMIO模式（RISC-V/mips32）
  add_mmio_map("vgactl", CONFIG_VGA_CTL_MMIO, vgactl_port_base, 8, NULL);
#endif

  // ----- 3. 分配显存空间 -----
  vmem = new_space(screen_size());

  // ----- 4. 注册显存映射（总是MMIO）-----
  add_mmio_map("vmem", CONFIG_FB_ADDR, vmem, screen_size(), NULL);

  // ----- 5. 初始化显示（如果开启）-----
  IFDEF(CONFIG_VGA_SHOW_SCREEN, init_screen());
  
  // ----- 6. 清空显存（如果开启显示）-----
  IFDEF(CONFIG_VGA_SHOW_SCREEN, memset(vmem, 0, screen_size()));
}



