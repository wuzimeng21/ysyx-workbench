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
#include <etrace.h>


// NO：异常号
// epc：触发异常的指令地址（Exception PC）
word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  // 保存当前 PC 到 mepc 寄存器
  // 保存异常原因到 mcause 寄存器
  // 可能修改 mstatus 寄存器
  // 将 CPU 的 PC 设置为异常处理入口地址

  cpu.mcause = NO;
  cpu.mepc = epc;
  cpu.mstatus = 0x1800;

// struct Context {
//   // TODO: fix the order of these members to match trap.S
//   // uintptr_t mepc, mcause, gpr[NR_REGS], mstatus;
//   uintptr_t gpr[NR_REGS], mcause, mstatus, mepc;
//   void *pdir;
// };
// add for PA3 etrace
  etrace_log(NO, epc, cpu.mtvec);

  return cpu.mtvec; // 异常处理入口地址（RISC-V 中就是 mtvec 寄存器的值）
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
