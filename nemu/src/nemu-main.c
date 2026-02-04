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

#include <common.h>
// add
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include "monitor/sdb/sdb.h"
#include <limits.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  /* Start engine. */
  engine_start();
  // 2026.2.4 PA2: forbiding disturn the test of am-kernels/tests/cpu-tests/tests in batch mode
  // FILE *fp = fopen("/home/wzm/Desktop/ysyx-workbench/nemu/tools/gen-expr/build/input", "r");
  // assert(fp != NULL);
  // char line[65530 + 128];
  // int success = 0, failure = 0;
  // while (fgets(line, sizeof(line), fp)) {
  //     unsigned int answer;
  //     char expression[128];
  //     unsigned int answer_calculate;
  //     bool su = false;
  //     if (sscanf(line, "%u %[^\n]", &answer, expression) == 2) {
  //       answer_calculate = expr(expression, &su); // word_t
  //       printf("(nemu_main) random test: expression: %s\n", expression);
  //       if(answer_calculate == INT_MAX) {
  //         ;
  //       }
  //       else {
  //         printf("answer: %u\n", answer);
  //         printf("answer_calculate: %u\n", answer_calculate);
  //         if(answer == answer_calculate) success++;
  //         else {
  //           failure ++;
  //           assert(0);
  //         }
  //         printf("----------------------\n");
  //       }

  //     } else {
  //         fprintf(stderr, "Read fail: %s", line);
  //     }
  // }

  // fclose(fp);
  // printf("success number: %d failure number: %d\n", success, failure);

  return is_exit_status_bad();
}
