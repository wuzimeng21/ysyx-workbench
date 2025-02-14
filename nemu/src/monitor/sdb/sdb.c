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

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/paddr.h>
#include <memory/vaddr.h>
#include <memory/host.h>
#include <../../../include/debug.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_si(char *args){
	int step = 1;
	if(args == NULL){
		;
	}
	else {
		// read the value of the N (si [N])
		sscanf(args, "%d", &step);
	}
	// move N steps
  printf("cmd_si N is %d\n", step);
	 cpu_exec(step);
	 return 0;
}

static int cmd_info(char *args){
	// print regs or watchpoints
	if(!strcmp(args, "r")) {
	  isa_reg_display();
	}
	else 
	{
		watchpoint_info();
	}
	return 0;
}

static int cmd_x(char *args) {
	char *N = strtok(args, " ");
	char * expr_16 = strtok(NULL, " ");
	// use Assert()???
	Assert(N != NULL && expr_16 != NULL, "less args: N->%s, EXPR->%s\n", N, expr_16);
	//assert(N != NULL && expr_16 != NULL);
	int l = 0;
	vaddr_t addr;
	sscanf(N, "%d", &l);
	sscanf(expr_16, "%x", &addr);
	//int expr_10 = strtol(expr_16, NULL, 16);
	printf("cmd_x N is %s, expr is %s\n", N, expr_16);
	printf("cmd_x l is %d, addr is %x or %d\n", l, addr, addr);
	for(int i = 0; i < l ; i ++) 
	{
    printf("cmd_x addr: %x\t", addr);
		word_t tmp = vaddr_read(addr, 4);
		addr += 4;
		printf("cmd_x read result: %x\n", tmp);
	}	
	return 0;
}

static int cmd_test()
{
    printf("\n-----ENTER CMD_TEST----\n");
    test_expr();
    printf("\n-----FINISH----\n");
    return 0;
}

static int cmd_p(char *args)
{
    bool su = true;
    /// char *e;
    // char *c1 = strtok(args, " ");
    word_t res_expr = expr(args, &su);

    if (su == false)
    {
        printf("expr err..\n");
    }
    else
    {
        printf("expr ans is %d.\n", res_expr);
    }

    return 0;
}

static int cmd_w(char *args)
{
    assert(args != NULL);
    bool su_tmp = true;
    word_t num_to_wp = expr(args, &su_tmp);
    assert(su_tmp != false);
    watchpoint_w(args, num_to_wp);
    watchpoint_info();
    return 0;
}

static int cmd_d(char *args)
{
    int num_wp = -1;
    sscanf(args, "%d", &num_wp);
    assert(num_wp >= 0);

    watchpoint_d(num_wp);
    return 0;
}



static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "help:  Display information about all supported commands", cmd_help },
  { "c", "c: Continue the execution of the program", cmd_c },
  { "q", "q: Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  {"si", "si [N]: Let the program execute N instructions step by step and then pause execution. If N is not provided, the default value is 1", cmd_si},
  {"info", "info SUNCMD(r || w): To print the register status and watchpoint information", cmd_info},
  {"x", "x N EXPR: To evaluate the expression EXPR, use the result as the starting memory address, and output N consecutive 4-byte values in hexadecimal format", cmd_x},
  {"test", "test: Test the function which is used to calulate the expression", cmd_test},
  {"p", "p EXPR: Calulate the expression EXPR", cmd_p},
  {"w", "w EXPR: When the value of the expression `EXPR` changes, pause the program execution", cmd_w},
  {"d", "d N: Delete the watchpoint with index N", cmd_d},

};


#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
