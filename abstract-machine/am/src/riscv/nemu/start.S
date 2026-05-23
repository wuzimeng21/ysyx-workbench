.section entry, "ax"
.globl _start
.type _start, @function # 告诉链接器 _start 是一个函数

_start:
  mv s0, zero
  mv sp, a0
  call _trm_init

.size _start, . - _start
