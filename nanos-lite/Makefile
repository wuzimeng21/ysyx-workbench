# Nanos-lite Makefile
# 这个 Makefile 用于构建 Nanos-lite 操作系统

# ==================== 基本配置 ====================

# HAS_NAVY 控制是否使用 Navy-apps 用户程序
# 如果取消注释，将会使用 Navy-apps 编译用户程序
#HAS_NAVY = 1

# ramdisk 镜像文件路径
# 这个文件包含用户程序，会被嵌入到 Nanos-lite 中
RAMDISK_FILE = build/ramdisk.img

# 目标名称
NAME = nanos-lite

# 查找所有源文件（.c, .cpp, .S）
SRCS = $(shell find -L ./src/ -name "*.c" -o -name "*.cpp" -o -name "*.S")

# 包含 AM 的 Makefile，它会处理编译规则
include $(AM_HOME)/Makefile

# ==================== 架构相关配置 ====================

# 如果是 native 架构，设置 ISA 为 am_native
ifeq ($(ARCH),native)
ISA = am_native
else
# 非 native 架构需要添加 Navy 的 libc 头文件路径
INC_PATH += include $(NAVY_HOME)/libs/libc/include
endif

# ==================== 资源文件生成 ====================

# resources.S 依赖于 ramdisk.img
# 每次 ramdisk.img 更新时，都会 touch resources.S
# 这会导致 resources.S 重新编译，从而将新的 ramdisk 嵌入
./src/resources.S: $(RAMDISK_FILE)
	@touch $@

# ==================== 没有 Navy-apps 的配置 ====================

ifeq ($(HAS_NAVY),)
# 如果没有定义 HAS_NAVY，使用空文件
files = $(RAMDISK_FILE) src/files.h src/syscall.h

# 如果文件不存在，创建空文件
$(foreach f,$(files),$(if $(wildcard $f),, $(shell touch $f)))

# ==================== 有 Navy-apps 的配置 ====================

else

# 检查 Navy-apps 是否存在
ifeq ($(wildcard $(NAVY_HOME)/libs/libos/src/syscall.h),)
  $(error $$NAVY_HOME must be a Navy-apps repo)
endif

# update 目标：更新 ramdisk 和相关头文件
update:
	# 编译 Navy-apps，生成 ramdisk 镜像
	$(MAKE) -s -C $(NAVY_HOME) ISA=$(ISA) ramdisk
	
	# 创建软链接到 build/ramdisk.img
	@ln -sf $(NAVY_HOME)/build/ramdisk.img $(RAMDISK_FILE)
	
	# 创建软链接到 src/files.h
	@ln -sf $(NAVY_HOME)/build/ramdisk.h src/files.h
	
	# 创建软链接到 src/syscall.h
	@ln -sf $(NAVY_HOME)/libs/libos/src/syscall.h src/syscall.h

.PHONY: update
endif