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
#include <memory/host.h>
#include <memory/vaddr.h>
#include <device/map.h>
#include <dtrace.h>

#define IO_SPACE_MAX (32 * 1024 * 1024)

static uint8_t *io_space = NULL;
static uint8_t *p_space = NULL;

uint8_t* new_space(int size) {
  uint8_t *p = p_space;
  // page aligned;
  size = (size + (PAGE_SIZE - 1)) & ~PAGE_MASK;
  p_space += size;
  assert(p_space - io_space < IO_SPACE_MAX);
  return p;
}

static void check_bound(IOMap *map, paddr_t addr) {
  if (map == NULL) {
    Assert(map != NULL, "address (" FMT_PADDR ") is out of bound at pc = " FMT_WORD, addr, cpu.pc);
  } else {
    Assert(addr <= map->high && addr >= map->low,
        "address (" FMT_PADDR ") is out of bound {%s} [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
        addr, map->name, map->low, map->high, cpu.pc);
  }
}

static void invoke_callback(io_callback_t c, paddr_t offset, int len, bool is_write) {
  if (c != NULL) { c(offset, len, is_write); }
}

void init_map() {
  io_space = malloc(IO_SPACE_MAX);
  assert(io_space);
  p_space = io_space;
}

word_t map_read(paddr_t addr, int len, IOMap *map) {
  // 1. 参数合法性检查
  assert(len >= 1 && len <= 8);  // 保证访问长度合法（1-8字节）
  // 2. 边界检查
  check_bound(map, addr);  // 确保 addr 在 map->low 和 map->high 之间
  // 3. 计算设备内偏移
  paddr_t offset = addr - map->low;  // 设备内部偏移地址
  // 4. 调用设备回调（读之前）
  invoke_callback(map->callback, offset, len, false); // false 表示读操作
  // 5. 从设备空间读取数据
  word_t ret = host_read(map->space + offset, len);  // 从映射空间读数据
  // add PA2 for dtrace
  IFDEF(CONFIG_DTRACE, read_dtrace(map, addr));
  // 6. 返回读到的数据
  return ret;
}

void map_write(paddr_t addr, int len, word_t data, IOMap *map) {
  // 1. 参数合法性检查
  assert(len >= 1 && len <= 8);  // 保证访问长度合法
  // 2. 边界检查
  check_bound(map, addr);  // 确保 addr 在映射范围内
  // 3. 计算设备内偏移
  paddr_t offset = addr - map->low;
  // 4. 写入数据到设备空间
  host_write(map->space + offset, len, data);  // 将 data 写入映射空间
  // 5. 调用设备回调（写之后）
  invoke_callback(map->callback, offset, len, true); // true 表示写操作
  // add PA2 for dtrace
  IFDEF(CONFIG_DTRACE, write_dtrace(map, addr));

}


