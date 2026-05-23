#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>

// 将SDL键盘事件转换为字符
char handle_key(SDL_Event *ev);

#define NR_CMD_NTERM ARRLEN(cmd_table_nterm)

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table_nterm [] = {
  { "help", "help:  Display information about all supported commands", sh_help },
  { "echo", "echo displays text or variables to the terminal output", sh_echo },
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
      printf("%s - %s\n", cmd_table_nterm[i].name, cmd_table_nterm[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD_NTERM; i ++) {
      if (strcmp(arg, cmd_table_nterm[i].name) == 0) {
        printf("%s - %s\n", cmd_table_nterm[i].name, cmd_table_nterm[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

// echo
static int sh_echo(const char *echo) {
  sh_printf("%s", echo);
  return 0;
}

// 命令处理函数
static void sh_handle_cmd(const char *cmd) {
  char * str = cmd;
  for (str; *str != NULL; str ++ ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd_tmp = strtok(str, " ");
    char * pathname = cmd_tmp;
    if (cmd_tmp == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd_tmp + strlen(cmd_tmp) + 1;
    char *envp = args + strlen(args) + 1;
    if (args >= str_end) {
      args = NULL;
    }

    int i;
    for (i = 0; i < NR_CMD_NTERM; i ++) {
      if (strcmp(cmd_tmp, cmd_table_nterm[i].name) == 0) {
        if (cmd_table_nterm[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (strcmp(envp, "PATH=") == 0) {
        envp += "/bin";
    }
    setenv("PATH", "/bin", 0);
    if (i == NR_CMD_NTERM) { 
      execve(pathname, args, envp);
      sh_printf("sh_handle_cmd: exec failed\n");
    }
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



