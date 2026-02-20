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

#include <utils.h>
#include <device/map.h>

/* http://en.wikibooks.org/wiki/Serial_Programming/8250_UART_Programming */
// NOTE: this is compatible to 16550

#define CH_OFFSET 0  // 数据寄存器在设备内的偏移

static uint8_t *serial_base = NULL;

// 输出一个字符
static void serial_putc(char ch) {
  MUXDEF(CONFIG_TARGET_AM, putch(ch), putc(ch, stderr));
}

// 串口 I/O 回调函数
static void serial_io_handler(uint32_t offset, int len, bool is_write) {
  assert(len == 1);  // 串口只能按字节访问

  switch (offset) {
    case CH_OFFSET:  // 数据寄存器（偏移 0）
        /* We bind the serial port with the host stderr in NEMU. */
      if (is_write) {
        // 写操作：从寄存器读取字符并输出
        serial_putc(serial_base[0]);
      } else {
        // 读操作：不支持，报错
        panic("do not support read");
      }
      break;

    default:  // 其他寄存器（未实现）
      panic("do not support offset = %d", offset);
  }
}

void init_serial() {
  serial_base = new_space(8);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("serial", CONFIG_SERIAL_PORT, serial_base, 8, serial_io_handler);
#else
  add_mmio_map("serial", CONFIG_SERIAL_MMIO, serial_base, 8, serial_io_handler);
#endif

}
