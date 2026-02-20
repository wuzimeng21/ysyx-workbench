/**
 * @file ioe.c
 * @brief AM的输入输出扩展(IOE)实现
 * 
 * 这个文件实现了AM的IOE（I/O Extension）接口，提供统一的设备访问抽象。
 * 它通过一个跳转表(lut)将抽象寄存器号映射到具体的设备处理函数。
 */

#include <am.h>
#include <klib-macros.h>

// ==================== 外部函数声明 ====================
// 这些函数在各自的设备实现文件中定义

// 定时器设备
void __am_timer_init();                    // 初始化定时器
void __am_timer_rtc(AM_TIMER_RTC_T *);     // 读取RTC时间
void __am_timer_uptime(AM_TIMER_UPTIME_T *); // 读取系统启动时间

// GPU设备
void __am_gpu_init();                       // 初始化GPU
void __am_gpu_config(AM_GPU_CONFIG_T *);    // 获取GPU配置
void __am_gpu_status(AM_GPU_STATUS_T *);    // 获取GPU状态
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *);    // 绘制帧缓冲

// 音频设备
void __am_audio_init();                      // 初始化音频
void __am_audio_config(AM_AUDIO_CONFIG_T *); // 获取音频配置
void __am_audio_ctrl(AM_AUDIO_CTRL_T *);     // 控制音频
void __am_audio_status(AM_AUDIO_STATUS_T *); // 获取音频状态
void __am_audio_play(AM_AUDIO_PLAY_T *);     // 播放音频

// 输入设备
void __am_input_keybrd(AM_INPUT_KEYBRD_T *); // 读取键盘输入

// 磁盘设备
void __am_disk_config(AM_DISK_CONFIG_T *cfg);   // 获取磁盘配置
void __am_disk_status(AM_DISK_STATUS_T *stat);  // 获取磁盘状态
void __am_disk_blkio(AM_DISK_BLKIO_T *io);      // 磁盘块IO操作

// ==================== 配置查询函数 ====================
// 这些函数用于查询设备是否存在

/**
 * @brief 查询定时器配置
 */
static void __am_timer_config(AM_TIMER_CONFIG_T *cfg) { 
  cfg->present = true;     // 定时器存在
  cfg->has_rtc = true;     // 支持RTC
}

/**
 * @brief 查询输入设备配置
 */
static void __am_input_config(AM_INPUT_CONFIG_T *cfg) { 
  cfg->present = true;     // 输入设备存在
}

/**
 * @brief 查询UART配置（串口）
 */
static void __am_uart_config(AM_UART_CONFIG_T *cfg) {   
  cfg->present = false;    // UART不存在
}

/**
 * @brief 查询网络设备配置
 */
static void __am_net_config (AM_NET_CONFIG_T *cfg) {    
  cfg->present = false;    // 网络设备不存在
}

// ==================== 跳转表定义 ====================
/**
 * 抽象寄存器号到处理函数的映射表
 * 
 * 这是一个函数指针数组，索引是AM定义的抽象寄存器号，
 * 值是对应的处理函数（统一为 void (*)(void*) 类型）
 */
typedef void (*handler_t)(void *buf);

static void *lut[128] = {
  // 定时器寄存器
  [AM_TIMER_CONFIG] = __am_timer_config,  // 查询定时器配置
  [AM_TIMER_RTC   ] = __am_timer_rtc,     // 读取RTC时间
  [AM_TIMER_UPTIME] = __am_timer_uptime,  // 读取启动时间

  // 输入设备寄存器
  [AM_INPUT_CONFIG] = __am_input_config,  // 查询输入设备配置
  [AM_INPUT_KEYBRD] = __am_input_keybrd,  // 读取键盘输入

  // GPU设备寄存器
  [AM_GPU_CONFIG  ] = __am_gpu_config,    // 获取GPU配置
  [AM_GPU_FBDRAW  ] = __am_gpu_fbdraw,    // 绘制帧缓冲
  [AM_GPU_STATUS  ] = __am_gpu_status,    // 获取GPU状态

  // UART设备寄存器
  [AM_UART_CONFIG ] = __am_uart_config,   // 查询UART配置

  // 音频设备寄存器
  [AM_AUDIO_CONFIG] = __am_audio_config,  // 获取音频配置
  [AM_AUDIO_CTRL  ] = __am_audio_ctrl,    // 控制音频
  [AM_AUDIO_STATUS] = __am_audio_status,  // 获取音频状态
  [AM_AUDIO_PLAY  ] = __am_audio_play,    // 播放音频

  // 磁盘设备寄存器
  [AM_DISK_CONFIG ] = __am_disk_config,   // 获取磁盘配置
  [AM_DISK_STATUS ] = __am_disk_status,   // 获取磁盘状态
  [AM_DISK_BLKIO  ] = __am_disk_blkio,    // 磁盘块IO操作

  // 网络设备寄存器
  [AM_NET_CONFIG  ] = __am_net_config,    // 查询网络配置
};

// ==================== 默认处理函数 ====================
/**
 * @brief 访问不存在寄存器的默认处理
 * 
 * 如果应用程序访问了未实现的寄存器，触发panic
 */
static void fail(void *buf) { 
  panic("access nonexist register"); 
}

// ==================== IOE初始化 ====================
/**
 * @brief 初始化IOE子系统
 * 
 * 1. 将跳转表中未初始化的条目设置为fail函数
 * 2. 初始化各个设备
 * 
 * @return true 初始化成功
 */
bool ioe_init() {
  // 将所有空指针设置为fail，避免访问未定义的寄存器
  for (int i = 0; i < LENGTH(lut); i++)
    if (!lut[i]) lut[i] = fail;
  
  // 初始化各个设备
  __am_gpu_init();    // 初始化GPU
  __am_timer_init();  // 初始化定时器
  __am_audio_init();  // 初始化音频
  
  return true;
}

void ioe_read (int reg, void *buf) { ((handler_t)lut[reg])(buf); }
void ioe_write(int reg, void *buf) { ((handler_t)lut[reg])(buf); }
