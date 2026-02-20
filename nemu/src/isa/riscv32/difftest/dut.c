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
#include <cpu/difftest.h>
#include "../local-include/reg.h"


bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  for (int i = 0; i < 16; i++) {
    if (ref_r->gpr[i] != cpu.gpr[i]) {
      // Log(ANSI_FMT("x%d mismatch at pc = 0x%08x: NEMU = 0x%08x, REF = 0x%08x",
      //     i, pc, cpu.gpr[i], ref_r->gpr[i], ANSI_FG_RED));
      Log(ANSI_FMT("x%d mismatch at pc = 0x%08x: NEMU = 0x%08x, REF = 0x%08x", ANSI_FG_RED),
    i, pc, cpu.gpr[i], ref_r->gpr[i]);
      return false;
    }
  }

  if (ref_r->pc != cpu.pc) {
    // Log(ANSI_FMT("PC mismatch at pc = 0x%08x: NEMU next PC = 0x%08x, REF next PC = 0x%08x",
    //     pc, cpu.pc, ref_r->pc, ANSI_FG_RED));
    Log(ANSI_FMT("PC mismatch at pc = 0x%08x: NEMU next PC = 0x%08x, REF next PC = 0x%08x", ANSI_FG_RED),
    pc, cpu.pc, ref_r->pc);
    return false;
  }

  return true;
}

void isa_difftest_attach() {
}
