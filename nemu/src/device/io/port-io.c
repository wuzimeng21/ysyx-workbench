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

/*

特性	    端口 I/O (port-io.c)	  内存映射 I/O (mmio.c)
地址空间	独立端口空间 (0-65535)	  共享物理地址空间
访问方式	in/out 指令	             普通访存指令
注册函数	add_pio_map()	          add_mmio_map()
读写接口	pio_read()/pio_write()	mmio_read()/mmio_write()
底层调用	map_read()/map_write()	map_read()/map_write()

*/


#include <device/map.h>
#define PORT_IO_SPACE_MAX 65535

#define NR_MAP 16  // 最多支持 16 个端口映射
static IOMap maps[NR_MAP] = {};
static int nr_map = 0;

/* 
  device interface 
  注册端口映射	设备初始化时调用
*/
void add_pio_map(const char *name, ioaddr_t addr, void *space, uint32_t len, io_callback_t callback) {
  assert(nr_map < NR_MAP);
  assert(addr + len <= PORT_IO_SPACE_MAX);
  maps[nr_map] = (IOMap){ .name = name, .low = addr, .high = addr + len - 1,
    .space = space, .callback = callback };
  Log("Add port-io map '%s' at [" FMT_PADDR ", " FMT_PADDR "]",
      maps[nr_map].name, maps[nr_map].low, maps[nr_map].high);

  nr_map ++;
}

/* CPU interface */
// CPU in 指令的底层实现
uint32_t pio_read(ioaddr_t addr, int len) {
  assert(addr + len - 1 < PORT_IO_SPACE_MAX);
  int mapid = find_mapid_by_addr(maps, nr_map, addr);
  assert(mapid != -1);
  return map_read(addr, len, &maps[mapid]);
}

// CPU out 指令的底层实现
void pio_write(ioaddr_t addr, int len, uint32_t data) {
  assert(addr + len - 1 < PORT_IO_SPACE_MAX);
  int mapid = find_mapid_by_addr(maps, nr_map, addr);
  assert(mapid != -1);
  map_write(addr, len, data, &maps[mapid]);
}
