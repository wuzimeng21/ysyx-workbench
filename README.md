# "一生一芯"工程项目

这是"一生一芯"的工程项目. 通过运行
```bash
bash init.sh subproject-name
```
进行初始化, 具体请参考[实验讲义][lecture note].

[lecture note]: https://ysyx.oscc.cc/docs/

## manual
man string
man stdarg

## git
git branch
git status
git add .
git commit -m "sth you want to say"
git push

## nemu
cd nemu/
make menuconfig
make clean
make
make run

## native
make ARCH=native ALL=dummy run

## am-kernels
cd am-kernels/tests/cpu-tests
make ARCH=riscv32-nemu ALL=dummy run > batch.md// 测试nemu
make ARCH=riscv32-nemu ALL=dummy gdb
riscv64-linux-gnu-objdump -d build/dummy-riscv32-nemu.elf
riscv64-linux-gnu-readelf -a build/dummy-riscv32-nemu.elf
### am-kernels test klib
cd am-kernels/tests/cpu-tests/
make ALL=string ARCH=native run
make ALL=stdio ARCH=native run

## navy-apps
<!-- update ramdisk -->
make ARCH=$ISA-nemu update 
<!-- if you want to add an app to navy-apps, please add it to navy-apps/Makefile TESTS -->

## project structure
abstract-machine
├── am                                  # AM相关
│   ├── include
│   │   ├── amdev.h
│   │   ├── am.h
│   │   └── arch                        # 架构相关的头文件定义
│   ├── Makefile
│   └── src
│       ├── mips
│       │   ├── mips32.h
│       │   └── nemu                    # mips32-nemu相关的实现
│       ├── native
│       ├── platform
│       │   └── nemu                    # 以NEMU为平台的AM实现
│       │       ├── include
│       │       │   └── nemu.h
│       │       ├── ioe                 # IOE
│       │       │   ├── audio.c
│       │       │   ├── disk.c
│       │       │   ├── gpu.c
│       │       │   ├── input.c
│       │       │   ├── ioe.c
│       │       │   └── timer.c
│       │       ├── mpe.c               # MPE, 当前为空
│       │       └── trm.c               # TRM
│       ├── riscv
│       │   ├── nemu                    # riscv32(64)相关的实现
│       │   │   ├── cte.c               # CTE
│       │   │   ├── start.S             # 程序入口
│       │   │   ├── trap.S
│       │   │   └── vme.c               # VME
│       │   └── riscv.h
│       └── x86
│           ├── nemu                    # x86-nemu相关的实现
│           └── x86.h
├── klib                                # 常用函数库
├── Makefile                            # 公用的Makefile规则
└── scripts                             # 构建/运行二进制文件/镜像的Makefile
    ├── isa
    │   ├── mips32.mk
    │   ├── riscv32.mk
    │   ├── riscv64.mk
    │   └── x86.mk
    ├── linker.ld                       # 链接脚本
    ├── mips32-nemu.mk
    ├── native.mk
    ├── platform
    │   └── nemu.mk
    ├── riscv32-nemu.mk
    ├── riscv64-nemu.mk
    └── x86-nemu.mk


am-kernels
├── benchmarks                  # 可用于衡量性能的基准测试程序
│   ├── coremark
│   ├── dhrystone
│   └── microbench
├── kernels                     # 可展示的应用程序
│   ├── hello
│   ├── litenes                 # 简单的NES模拟器
│   ├── nemu                    # NEMU
│   ├── slider                  # 简易图片浏览器
│   ├── thread-os               # 内核线程操作系统
│   └── typing-game             # 打字小游戏
└── tests                       # 一些具有针对性的测试集
    ├── am-tests                # 针对AM API实现的测试集
    └── cpu-tests               # 针对CPU指令实现的测试集















层次	本质	运行环境	抽象程度
native	直接在真机上运行	Linux/Windows/macOS	最高
AM	抽象机器层	可运行在多种平台上	中间
NEMU	模拟器	运行在host系统上	最低


在 PA 中的角色
组件	作用
native	快速测试、验证算法正确性
AM	提供统一接口，隔离平台差异
NEMU	模拟硬件，运行客户程序
三者配合：

在 native 上快速开发调试

通过 AM 保持代码可移植

在 NEMU 上验证硬件模拟的正确性

八、总结
概念	一句话定义
native	直接在真机上运行，最快但不可移植
AM	抽象机器层，让代码可在不同平台运行
NEMU	模拟器，模拟硬件执行客户程序
native 是真机，AM 是桥梁，NEMU 是模拟器！ ✅