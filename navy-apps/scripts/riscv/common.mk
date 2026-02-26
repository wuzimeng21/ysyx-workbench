# ==================== RISC-V 64 位架构编译配置 ====================
# 这个配置文件用于设置 RISC-V 64 位程序的交叉编译环境和链接地址

# 交叉编译工具链前缀
# 所有编译工具都会加上这个前缀：
#   riscv64-linux-gnu-gcc   - C编译器
#   riscv64-linux-gnu-ld    - 链接器
#   riscv64-linux-gnu-as    - 汇编器
#   riscv64-linux-gnu-ar    - 归档器
CROSS_COMPILE = riscv64-linux-gnu-

# 链接地址选择
# 根据是否定义 VME 变量选择不同的链接地址
# VME 表示是否启用虚拟内存 (Virtual Memory)
#   - 如果 VME 已定义: 使用 0x40000000 (PA4 虚拟内存地址)
#   - 如果 VME 未定义: 使用 0x83000000 (PA3 物理内存地址)
LNK_ADDR = $(if $(VME), 0x40000000, 0x83000000)

# C 编译标志
# -fno-pic     : 不生成位置无关代码，生成绝对地址代码
#               程序运行在固定地址，不需要重定位
# -march=rv64g : 指定 RISC-V 64 位 G 扩展
#               G 扩展包含 I (整数), M (乘除), A (原子), F/D (浮点)
# -mcmodel=medany : 使用 medium-any 代码模型
#                   任何地址都可以通过 auipc + addi 两条指令访问
#                   适合地址空间较大的情况（支持 4GB 范围内的地址）
CFLAGS  += -fno-pic -march=rv64g -mcmodel=medany

# 链接标志
# --no-relax        : 禁用链接器优化
#                    避免某些重定位优化，确保地址计算正确
# -Ttext-segment    : 设置代码段(.text)的起始地址
#                    程序将从 $(LNK_ADDR) 开始执行
LDFLAGS += --no-relax -Ttext-segment $(LNK_ADDR)

# ==================== 生成的完整工具链名称 ====================
# 编译器:   riscv64-linux-gnu-gcc
# 汇编器:   riscv64-linux-gnu-as
# 链接器:   riscv64-linux-gnu-ld
# 归档器:   riscv64-linux-gnu-ar
# 反汇编:   riscv64-linux-gnu-objdump
# 格式转换: riscv64-linux-gnu-objcopy
# ELF信息:  riscv64-linux-gnu-readelf


