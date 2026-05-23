#include <common.h>
#include <cpu/cpu.h>
#include <isa.h>

extern CPU_state cpu;

// 异常名称表
static const char *exception_names[] = {
  [0]  = "Instruction address misaligned",
  [1]  = "Instruction access fault",
  [2]  = "Illegal instruction",
  [3]  = "Breakpoint",
  [4]  = "Load address misaligned",
  [5]  = "Load access fault",
  [6]  = "Store/AMO address misaligned",
  [7]  = "Store/AMO access fault",
  [8]  = "Environment call from U-mode",
  [9]  = "Environment call from S-mode",
  [11] = "Environment call from M-mode",
  [12] = "Instruction page fault",
  [13] = "Load page fault",
  [15] = "Store/AMO page fault",
};

void etrace_log(word_t NO, vaddr_t epc, vaddr_t mtvec) {
  printf("\33[1;35m[etrace]\33[0m ");
  printf("Exception %d (%s) at pc = 0x%08x\n", 
         NO, 
         NO < 16 && exception_names[NO] ? exception_names[NO] : "Unknown",
         cpu.pc);
  printf("  -> mepc = 0x%08x\n", epc);
  printf("  -> mtvec = 0x%08x\n", mtvec);
  
  // 打印当前特权模式
  const char *modes[] = {"U", "S", "?", "M"};
  printf("  -> mode = %s\n", modes[cpu.mode]);
  return ;
}

