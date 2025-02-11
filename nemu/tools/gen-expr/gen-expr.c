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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static int buf_idx = 0;
static char buf[65530] = {};
static char code_buf[65530 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"} ";

static int choose(int num) {
  // 0 ~ num - 1
  int idx = rand() % num;
  // printf("random test: choose number is: %d\n", idx);
  return idx;
}

static void gen(char c) {
  assert(buf_idx < 65530);  // buf overflow
  buf[buf_idx ++] = c;
  return ;
}

static void gen_num() {
  assert(buf_idx < 65530);  // buf overflow
  int max_num = 50; // maximum number
  int num = rand() % (max_num + 1) + 1;
  int len = 0;
  int num_tmp = num;
  while(num_tmp) {
    len ++;
    num_tmp /= 10;
  }
  int tmp = 1;
  for(int i = 1; i < len; i ++) {
    tmp *= 10;
  }
  while(num) {
    buf[buf_idx ++] = (num / tmp) + '0';
    num = num % tmp;
    tmp /= 10;
  }
  return ;
}

static void gen_rand_op() {
  assert(buf_idx < 65530);  // buf overflow
  char ops[4] = {'+', '-', '*', '/',}; //'!=', '<=', '>=', '<', '>', '&&', '||'};
  int idx = rand() % 4;
  buf[buf_idx ++] = ops[idx];
  return ;
}

static void gen_rand_expr() {
  // printf("idx %d\n", buf_idx);
  assert(buf_idx < 65530);  // buf overflow
  switch (choose(3)) {
    case 0: gen_num(); break;
    case 1: gen('('); gen_rand_expr(); gen(')'); break;
    default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    buf_idx = 0; // reset
    gen_rand_expr();
    buf[buf_idx] = '\0';

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    // fp = fopen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);
    // fclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
