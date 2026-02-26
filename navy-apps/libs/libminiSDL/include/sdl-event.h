#ifndef __SDL_EVENT_H__
#define __SDL_EVENT_H__

#define _KEYS(_) \
  _(ESCAPE) _(F1) _(F2) _(F3) _(F4) _(F5) _(F6) _(F7) _(F8) _(F9) _(F10) _(F11) _(F12) \
  _(GRAVE) _(1) _(2) _(3) _(4) _(5) _(6) _(7) _(8) _(9) _(0) _(MINUS) _(EQUALS) _(BACKSPACE) \
  _(TAB) _(Q) _(W) _(E) _(R) _(T) _(Y) _(U) _(I) _(O) _(P) _(LEFTBRACKET) _(RIGHTBRACKET) _(BACKSLASH) \
  _(CAPSLOCK) _(A) _(S) _(D) _(F) _(G) _(H) _(J) _(K) _(L) _(SEMICOLON) _(APOSTROPHE) _(RETURN) \
  _(LSHIFT) _(Z) _(X) _(C) _(V) _(B) _(N) _(M) _(COMMA) _(PERIOD) _(SLASH) _(RSHIFT) \
  _(LCTRL) _(APPLICATION) _(LALT) _(SPACE) _(RALT) _(RCTRL) \
  _(UP) _(DOWN) _(LEFT) _(RIGHT) _(INSERT) _(DELETE) _(HOME) _(END) _(PAGEUP) _(PAGEDOWN)

#define enumdef(k) SDLK_##k,

enum SDL_Keys {
  SDLK_NONE = 0,      // 无按键
  _KEYS(enumdef)      // 展开所有按键（SDLK_ESCAPE, SDLK_F1, ...）
};

enum SDL_EventType {
  SDL_KEYDOWN,        // 按键按下事件
  SDL_KEYUP,          // 按键释放事件
  SDL_USEREVENT,      // 用户自定义事件
};

#define SDL_EVENTMASK(ev_type) (1u << (ev_type))

enum SDL_EventAction {
  SDL_ADDEVENT,       // 添加事件到队列
  SDL_PEEKEVENT,      // 查看事件但不移除
  SDL_GETEVENT,       // 获取事件并从队列移除
};

typedef struct {
  uint8_t sym;        // 按键符号（对应 SDL_Keys 枚举值）
} SDL_keysym;

typedef struct {
  uint8_t type;               // 事件类型（SDL_KEYDOWN/SDL_KEYUP）
  SDL_keysym keysym;          // 按键符号信息
} SDL_KeyboardEvent;

typedef struct {
  uint8_t type;               // 事件类型（SDL_USEREVENT）
  int code;                   // 用户事件代码
  void *data1;                // 用户数据1
  void *data2;                // 用户数据2
} SDL_UserEvent;

typedef union {
  uint8_t type;                // 事件类型（所有事件共有的第一个字段）
  SDL_KeyboardEvent key;       // 键盘事件
  SDL_UserEvent user;          // 用户事件
} SDL_Event;

int SDL_PushEvent(SDL_Event *ev);
int SDL_PollEvent(SDL_Event *ev);
int SDL_WaitEvent(SDL_Event *ev);
int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask);
uint8_t* SDL_GetKeyState(int *numkeys);

#endif
