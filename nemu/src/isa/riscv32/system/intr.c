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

static volatile int timer_intr_pending = 0;

void set_timer_intr() {
  timer_intr_pending = 1;
}

word_t isa_query_intr() {
  if (timer_intr_pending && (cpu.mstatus & 0x8)) {  // MIE must be set
    timer_intr_pending = 0;  // consume the interrupt
    return 0x80000007;
  }
  return INTR_EMPTY;
}

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  // For interrupts (bit 31 set), check MIE (bit 3 in mstatus)
  if (NO & 0x80000000) {
    if (!(cpu.mstatus & 0x8)) {
      return epc;  // interrupts disabled, continue
    }
    timer_intr_pending = 0;
  }

  cpu.mcause = NO;
  cpu.mepc = epc;

  cpu.mstatus &= ~(1 << 7);   // clear MPIE
  cpu.mstatus |= (cpu.mstatus >> 3 & 1) << 7;  // save MIE → MPIE
  cpu.mstatus &= ~(1 << 3);   // clear MIE (disable interrupts in handler)

  return cpu.mtvec;
}
