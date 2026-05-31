#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

// 将SDL键盘事件转换为字符
char handle_key(SDL_Event *ev);

#define ARRLEN(arr) (sizeof(arr) / sizeof(arr[0]))
#define NR_CMD_NTERM ARRLEN(cmd_table_nterm)

static int sh_help(char *args);
static int sh_echo(char *echo);
static int sh_exit(char *args);
static int sh_execve(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table_nterm [] = {
  { "help",   "help:   Display information about all supported commands", sh_help },
  { "echo",   "echo:   Display text or variables to the terminal", sh_echo },
  { "execve", "execve: Execute an external program (usage: execve <prog> [args...])", sh_execve },
  { "exit",   "exit:   Exit the terminal and return to menu", sh_exit },
};

// shell专用的打印函数
static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

// 显示命令提示符
static void sh_prompt() {
  sh_printf("sh> ");
}

static int sh_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    for (i = 0; i < NR_CMD_NTERM; i ++) {
      sh_printf("%s - %s\n", cmd_table_nterm[i].name, cmd_table_nterm[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD_NTERM; i ++) {
      if (strcmp(arg, cmd_table_nterm[i].name) == 0) {
        sh_printf("%s - %s\n", cmd_table_nterm[i].name, cmd_table_nterm[i].description);
        return 0;
      }
    }
    sh_printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

// echo
static int sh_echo(char *echo) {
  sh_printf("%s\n", echo);
  return 0;
}

// exit
static int sh_exit(char *args) {
  _exit(0);
  return 0;
}

// execve: 显式启动外部程序
static int sh_execve(char *args) {
  if (args == NULL || *args == '\0') {
    sh_printf("Usage: execve <program> [args...]\n");
    return -1;
  }

  // 解析程序名
  char *arg_saveptr;
  char *prog = strtok_r(args, " \t\n", &arg_saveptr);
  if (prog == NULL) {
    sh_printf("Usage: execve <program> [args...]\n");
    return -1;
  }

  // 用静态缓冲区避免 execve 加载 ELF 时栈上数据被覆盖
  static char fullpath[256];
  if (prog[0] == '/') {
    snprintf(fullpath, sizeof(fullpath), "%s", prog);
  } else {
    snprintf(fullpath, sizeof(fullpath), "/bin/%s", prog);
  }

  // 构建 argv 数组
  static char *argv[16];
  int argc = 0;
  argv[argc++] = fullpath;  // argv[0] = 完整路径

  char *arg = strtok_r(NULL, " \t\n", &arg_saveptr);
  while (arg != NULL && argc < 15) {
    argv[argc++] = arg;
    arg = strtok_r(NULL, " \t\n", &arg_saveptr);
  }
  argv[argc] = NULL;

  static const char *envp[] = {"PATH=/bin", NULL};

  execve(fullpath, argv, (char *const *)envp);
  sh_printf("execve: %s: failed to execute\n", fullpath);
  return -1;
}

// 命令处理函数
static void sh_handle_cmd(const char *cmd) {
  char *str = (char *)cmd;
  char *saveptr;
  char *cmd_tmp = strtok_r(str, " \t\n", &saveptr);
  if (cmd_tmp == NULL) return;

  char *args = saveptr;  // 剩余部分作为参数

  // 先尝试内置命令
  int i;
  for (i = 0; i < NR_CMD_NTERM; i ++) {
    if (strcmp(cmd_tmp, cmd_table_nterm[i].name) == 0) {
      if (cmd_table_nterm[i].handler(args) < 0) { return; }
      break;
    }
  }

  // 如果不是内置命令，尝试作为外部程序执行
  if (i == NR_CMD_NTERM) {
    // 构建完整路径: /bin/ + 命令名
    char fullpath[256];
    if (cmd_tmp[0] == '/') {
      snprintf(fullpath, sizeof(fullpath), "%s", cmd_tmp);
    } else {
      snprintf(fullpath, sizeof(fullpath), "/bin/%s", cmd_tmp);
    }

    // 构建 argv 数组: [cmdname, arg1, arg2, ..., NULL]
    char *argv[16];
    int argc = 0;
    argv[argc++] = cmd_tmp;  // argv[0] = 命令名

    if (args && *args) {
      char *arg_saveptr;
      char *arg = strtok_r(args, " \t\n", &arg_saveptr);
      while (arg != NULL && argc < 15) {
        argv[argc++] = arg;
        arg = strtok_r(NULL, " \t\n", &arg_saveptr);
      }
    }
    argv[argc] = NULL;

    // 构建 envp 数组: ["PATH=/bin", NULL]
    const char *envp[] = {"PATH=/bin", NULL};

    execve(fullpath, argv, (char *const *)envp);
    sh_printf("sh: %s: command not found\n", cmd_tmp);
  }
}

// shell主循环
void builtin_sh_run() {
  sh_banner();
  sh_prompt();

  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          // sh_echo(res);
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}



