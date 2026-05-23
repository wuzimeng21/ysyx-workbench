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
#include <memory/vaddr.h>
#include <memory/paddr.h>

// paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
//   return MEM_RET_FAIL;
// }


paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type)
{
    assert(cpu.satp >> 31);
    paddr_t pt1 = (cpu.satp & 0x3FFFFF) << 12;
    word_t pte1 = paddr_read(pt1 + (vaddr >> 22) * 4, 4);
    assert(pte1 & 1);
    paddr_t pt2 = pte1 >> 10 << 12;
    word_t pte2 = paddr_read(pt2 + (vaddr << 10 >> 22) * 4, 4);
    assert(pte2 & 1);
    paddr_t pa = (pte2 >> 10 << 12) | (vaddr & 0xFFF);
    return pa;
}

