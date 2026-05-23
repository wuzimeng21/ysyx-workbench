#include <NDL.h>
#include <SDL.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  panic("Not implement");
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  // NDL_PollEvent
  char buf[16];
  while(1) {
    if(NDL_PollEvent(buf, sizeof(buf) / sizeof(buf[0]))) {
      if(strcmp(buf, "down") == 0 || strcmp(buf, "j")){
        event->type = SDL_KEYDOWN; 
        event->key->keysym->sym = SDLK_DOWN;
      }
      else if(strcmp(buf, "up") || strcmp(buf, "k")) {
        event->type = SDL_KEYDOWN;
        event->key->keysym->sym = SDLK_UP;
      }
      else if(strcmp(buf, "g")) {
        event->type = SDL_KEYDOWN;
        event->key->keysym->sym = SDLK_G;
      }
      else {
        event->type = SDL_KEYDOWN; 
        event->key->keysym->sym = keyname[(buf[0] + buf[1]) - '0'];
      }
      return 0;
    }
  }
  return 1;
}

// SDL_PollEvent(): 它和SDL_WaitEvent()不同的是, 如果当前没有任何事件, 就会立即返回
int SDL_PollEvent(SDL_Event *ev) {
  
  return 0;
}

// NSlide USAGE:
//   j/down - page down
//   k/up - page up
//   gg - first page
// typedef union{
    // // 所有事件共有的类型字段（必须放在第一位）
    // Uint8 type;  // 事件类型标识符，用于判断具体是哪种事件
    // // ============ 窗口活动事件 ============
    // SDL_ActiveEvent active;  // 窗口获得或失去焦点时触发
    // // ============ 键盘事件 ============
    // SDL_KeyboardEvent key;   // 按键按下/释放事件
    // // ============ 鼠标事件 ============
    // SDL_MouseMotionEvent motion;  // 鼠标移动事件
    // SDL_MouseButtonEvent button;  // 鼠标按钮按下/释放事件
    // // ============ 游戏手柄事件 ============
    // SDL_JoyAxisEvent jaxis;    // 摇杆轴移动事件
    // SDL_JoyBallEvent jball;    // 轨迹球移动事件
    // SDL_JoyHatEvent jhat;      // 方向帽（POV）事件
    // SDL_JoyButtonEvent jbutton; // 手柄按钮事件
    // // ============ 窗口事件 ============
    // SDL_ResizeEvent resize;    // 窗口大小改变事件
    // SDL_ExposeEvent expose;    // 窗口暴露（需要重绘）事件
    // // ============ 系统事件 ============
    // SDL_QuitEvent quit;        // 退出程序事件
    // SDL_UserEvent user;        // 用户自定义事件
    // SDL_SysWMEvent syswm;      // 系统窗口管理器事件
//        } SDL_Event;
int SDL_WaitEvent(SDL_Event *event) {
  // NDL_PollEvent
  char buf[16];
  while(1) {
    if(NDL_PollEvent(buf, sizeof(buf) / sizeof(buf[0]))) {
      if(strcmp(buf, "down") == 0 || strcmp(buf, "j")){
        event->type = SDL_KEYDOWN; 
        event->key->keysym->sym = SDLK_DOWN;
      }
      else if(strcmp(buf, "up") || strcmp(buf, "k")) {
        event->type = SDL_KEYDOWN;
        event->key->keysym->sym = SDLK_UP;
      }
      else if(strcmp(buf, "g")) {
        event->type = SDL_KEYDOWN;
        event->key->keysym->sym = SDLK_G;
      }
      else {
        event->type = SDL_KEYDOWN; 
        event->key->keysym->sym = keyname[(buf[0] + buf[1]) - '0'];
      }
    }
    
  }
  return 0;
}


int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  panic("Not implement");
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  panic("Not implement");
  return NULL;
}
