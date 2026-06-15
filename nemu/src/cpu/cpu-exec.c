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

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>
#include "../monitor/sdb/sdb.h"
// PA2: itrace/iringbuf/ftrace/mtrace
#include <utils.h>

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 10

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

void device_update();


static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
    /* 1. 条件指令踪迹记录（写入日志文件） */
    /* 
     * CONFIG_ITRACE_COND: 是否启用条件踪迹记录
     * ITRACE_COND: 具体的条件表达式（如指令计数范围、PC地址范围等）
     * log_write(): 将踪迹写入日志文件（默认：build/nemu-log.txt）
     */
#ifdef CONFIG_ITRACE_COND
    if (ITRACE_COND) { 
        log_write("%s\n", _this->logbuf); 
    }
#endif

// #ifdef CONFIG_MTRACE
//     log_write("MTRACE enable...\n");
// #else
//     log_write("MTRACE disable...\n");
// #endif

// #ifdef CONFIG_ITRACE
//     log_write("ITRACE enable...\n");
// #else
//     log_write("ITRACE disable...\n");
// #endif

    /* 2. 单步调试控制台输出 */
    /* 
     * g_print_step: 当执行少量指令（如si命令）时为true
     * CONFIG_ITRACE: 是否编译itrace功能
     * puts(): 直接输出到控制台，便于实时调试
     */
    if (g_print_step) { 
        IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); 
    }

    // PA2 add for iringbuf
    IFDEF(CONFIG_ITRACE, insert_iringbuf(_this->logbuf) ); 


    /* 3. 差分测试验证 */
    /* 
     * CONFIG_DIFFTEST: 是否启用差分测试
     * difftest_step(): 与参考模拟器（如QEMU）比较执行结果
     * 如果结果不一致，会停止仿真并报告错误
     */
    IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));

    /* 4. 观察点（Watchpoint）检查 */
    /* 
     * CONFIG_WATCHPOINT: 是否启用观察点功能
     * check_all_wp(): 检查所有观察点的值是否发生变化
     * 如果观察点值变化，停止仿真以便用户调试
     */
#ifdef CONFIG_WATCHPOINT
    bool flag_trace_and_difftest = check_all_wp();
    if (!flag_trace_and_difftest) {
        nemu_state.state = NEMU_STOP;  // 停止仿真
        return;  // 提前返回，不再执行后续跟踪
    }
#endif
}




static void exec_once(Decode *s, vaddr_t pc) {
    /* 1. 设置当前指令的PC值 */
    s->pc = pc;
    
    /* 
     * 静态下一条PC（假设顺序执行）
     * 对于大多数指令：snpc = pc + 指令长度
     * 对于跳转指令：后续会被动态PC覆盖
     */
    s->snpc = pc;
    
    /* 2. 执行指令（架构相关） */
    /* 
     * isa_exec_once(): 指令集架构相关的执行函数
     * 内部会：取指、译码、执行、更新snpc/dnpc
     */
    isa_exec_once(s);
    
    /* 
     * 动态下一条PC（实际执行后的PC）
     * 对于顺序指令：dnpc = snpc
     * 对于跳转/分支指令：dnpc = 目标地址
     */
    cpu.pc = s->dnpc;
    
    /* 3. 生成指令踪迹（如果启用） */
#ifdef CONFIG_ITRACE
    /* 指向日志缓冲区 */
    char *p = s->logbuf;
    
    /* 3.1 输出PC地址 */
    /* 格式：0x80000000: */
    p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc); // FMT_WORD : pc length
    
    /* 3.2 计算指令长度（字节） */
    int ilen = s->snpc - s->pc;  // 通过snpc-pc得到指令长度
    
    /* 3.3 输出机器码字节 */
    int i;
    uint8_t *inst = (uint8_t *)&s->isa.inst;
    
    /* 
     * 不同架构的字节顺序不同：
     * x86: 小端，从低地址到高地址输出
     * RISC-V/MIPS: 大端，从高地址到低地址输出（指令存储器视图）
     */
#ifdef CONFIG_ISA_x86
    for (i = 0; i < ilen; i++) {
#else
    for (i = ilen - 1; i >= 0; i--) {
#endif
        /* 输出十六进制字节：%02x */
        p += snprintf(p, 4, " %02x", inst[i]);
    }
    
    /* 3.4 格式化对齐 */
    /* 
     * 不同架构最大指令长度不同：
     * x86: 可变长度，最大约15字节，这里设为8
     * RISC-V/MIPS: 固定4字节（或包含压缩指令）
     */
    int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
    int space_len = ilen_max - ilen;
    if (space_len < 0) space_len = 0;
    space_len = space_len * 3 + 1;  // 每个字节占3字符（空格+2十六进制）
    
    /* 填充空格使所有行对齐 */
    memset(p, ' ', space_len);
    p += space_len;
    
    /* 3.5 调用反汇编器生成人类可读的汇编代码 */
    void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
    
    /* 
     * 反汇编参数：
     * p: 输出缓冲区当前位置
     * s->logbuf + sizeof(s->logbuf) - p: 剩余缓冲区大小
     * MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc): 反汇编地址（x86特殊处理）
     * (uint8_t *)&s->isa.inst: 机器码指针
     * ilen: 指令长度
     */
    disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
                MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), 
                (uint8_t *)&s->isa.inst, ilen);
#endif /* CONFIG_ITRACE */
}



static void execute(uint64_t n) {
  Decode s;
  for (;n > 0; n --) {
    // Poll for pending interrupts before each instruction
    word_t intr = isa_query_intr();
    if (intr != INTR_EMPTY) {
      cpu.pc = isa_raise_intr(intr, cpu.pc);
    }
    exec_once(&s, cpu.pc);
    g_nr_guest_inst ++;
    trace_and_difftest(&s, cpu.pc);
    if (nemu_state.state != NEMU_RUNNING) break;
    IFDEF(CONFIG_DEVICE, device_update());
  }
}

static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {
  isa_reg_display();
  statistic();
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT: case NEMU_QUIT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();
  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break;

    case NEMU_END: case NEMU_ABORT:
        log_write("====================  cpu_exec ====================");

      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
      nemu_state.halt_pc);
      // PA2 add
      #ifdef CONFIG_ITRACE
        if (nemu_state.halt_ret != 0) {
            pop_iringbuf();
        }
      #endif
      // fall through
    case NEMU_QUIT: statistic();
  }
}


