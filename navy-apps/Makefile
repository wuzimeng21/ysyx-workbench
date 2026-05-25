# Navy Applications and Libraries Makefile
# 用于构建 Navy 应用程序和库的通用 Makefile

### *通过 `make html` 获取更可读的版本* (需要 python-markdown)
html:
	cat Makefile | sed 's/^\([^#]\)/    \1/g' | markdown_py > Makefile.html
.PHONY: html

## ==================== 1. 基本设置和检查 ====================

### 默认构建应用程序
ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS  = app
  .DEFAULT_GOAL = app
endif

### 当执行 clean/clean-all/html/init 时跳过检查
ifeq ($(findstring $(MAKECMDGOALS),clean|clean-all|html|init),)

### 打印构建信息
$(info # Building $(NAME)-$(MAKECMDGOALS) [$(ISA)])

### 检查：环境变量 $NAVY_HOME 是否正确
ifeq ($(wildcard $(NAVY_HOME)/libs/libos/src/syscall.h),)
  $(error $$NAVY_HOME must be a Navy-apps repo)
endif

### 检查：环境变量 $ISA 必须在支持的列表中
ISAS = $(basename $(notdir $(shell ls $(NAVY_HOME)/scripts/*.mk)))
ifeq ($(filter $(ISAS), $(ISA)), )
  $(error Expected $$ISA in {$(ISAS)}, Got "$(ISA)")
endif

### 检查结束
endif

## ==================== 2. 通用编译目标 ====================

### 创建目标目录 (build/$ISA)
WORK_DIR  = $(shell pwd)
DST_DIR   = $(WORK_DIR)/build/$(ISA)
$(shell mkdir -p $(DST_DIR))

### 编译目标（应用程序或归档库）
APP     ?= $(WORK_DIR)/build/$(NAME)-$(ISA)
ARCHIVE  = $(WORK_DIR)/build/$(NAME)-$(ISA).a


### 为 ISA != native 添加默认库
ifneq ($(ISA), native)
LIBS += libc libos
CFLAGS += -U_FORTIFY_SOURCE  # 修复 Newlib 在 Ubuntu 上的编译错误
else
WL = -Wl,
endif

### 32位 ISA 可能需要 compiler-rt 来支持 64位乘除法
ifneq ($(findstring $(ISA), x86|mips32|riscv32|riscv32e|loongarch32r),)
LIBS += compiler-rt
endif

### 需要添加到包含路径的所有库
LIBS_INC := $(sort $(LIBS) $(LIB_DEP))

ifeq ($(MAKECMDGOALS),archive)
LIBS := $(LIB_DEP) # 覆盖：只构建和归档依赖的库
endif

### 需要链接的文件：目标文件 (.o) 和库文件 (.a)
OBJS      = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(SRCS))))
LIBS     := $(sort $(LIBS)) # 延迟求值会导致无限递归
LINKAGE  += $(OBJS)  # 库归档文件由下面的 LIB_TEMPLATE 添加

## ==================== 3. 通用编译标志 ====================

### (交叉)编译器
AS        = $(CROSS_COMPILE)gcc
CC        = $(CROSS_COMPILE)gcc
CXX       = $(CROSS_COMPILE)g++
LD        = $(CROSS_COMPILE)ld
AR        = $(CROSS_COMPILE)ar

### 编译标志
INC_PATH += $(WORK_DIR)/include $(foreach l,$(LIBS_INC),$(NAVY_HOME)/libs/$(l)/include/)
INCFLAGS += $(addprefix -I, $(INC_PATH))

CFLAGS   += -O2 -MMD $(INCFLAGS) -static -D__NAVY__ \
            -D__ISA__=\"$(ISA)\" -D__ISA_$(shell echo $(ISA) | tr a-z A-Z)__ \
            -fno-asynchronous-unwind-tables -fno-builtin -fno-stack-protector
CXXFLAGS +=  $(CFLAGS) -fno-rtti -fno-exceptions
ASFLAGS  +=  $(CFLAGS)

## ==================== 4. ISA 特定配置 ====================

### 包含 ISA 特定的配置文件（例如 scripts/x86.mk）
-include $(NAVY_HOME)/scripts/$(ISA).mk

## ==================== 5. 编译规则 ====================

### 规则：单个 .c -> .o (gcc)
$(DST_DIR)/%.o: %.c
	@mkdir -p $(dir $@) && echo + CC $<
	@$(CC) -std=gnu11 $(CFLAGS) -c -o $@ $(realpath $<)

### 规则：单个 .cc -> .o (g++)
$(DST_DIR)/%.o: %.cc
	@mkdir -p $(dir $@) && echo + CXX $<
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

### 规则：单个 .cpp -> .o (g++)
$(DST_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@) && echo + CXX $<
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

### 规则：单个 .S -> .o (gcc 调用 as)
$(DST_DIR)/%.o: %.S
	@mkdir -p $(dir $@) && echo + AS $<
	@$(AS) $(ASFLAGS) -c -o $@ $(realpath $<)

# $(1): 库名称
define LIB_TEMPLATE =
$$(NAVY_HOME)/libs/$(1)/build/$(1)-$$(ISA).a: force
	@$$(MAKE) -s -C $$(NAVY_HOME)/libs/$(1) archive
LINKAGE += $$(NAVY_HOME)/libs/$(1)/build/$(1)-$$(ISA).a
endef

### 规则：递归 make 构建依赖的库（libc, libos, ...）
$(foreach lib, $(LIBS), $(eval $(call LIB_TEMPLATE,$(lib))))

### 规则：链接 .o 和 .a -> $(APP)，最终的 ELF 二进制文件
$(APP): $(LINKAGE)
	@echo + LD "->" $(shell realpath $@ --relative-to .)
	@$(LD) $(LDFLAGS) -o $@ $(WL)--start-group $^ $(WL)--end-group

### 规则：归档 .o -> $(ARCHIVE).a
$(ARCHIVE): $(LINKAGE)
	@echo + AR "->" $(shell realpath $@ --relative-to .)
	@$(AR) rcs --thin $@ $^

### 规则：#include 依赖：包含 gcc 通过 -MMD 生成的 .d 文件
-include $(addprefix $(DST_DIR)/, $(addsuffix .d, $(basename $(SRCS))))

## ==================== 6. 其他功能 ====================

### 如果 libc 不存在，从 github 拉取 newlib
ifeq ($(wildcard $(NAVY_HOME)/libs/libc/Makefile),)
  $(shell cd $(NAVY_HOME)/libs && git clone git@github.com:NJU-ProjectN/newlib-navy.git libc)
endif

### 构建顺序控制
app: $(APP)
archive: $(ARCHIVE)
.PHONY: app archive

### 强制重新构建规则
force:
.PHONY: force

### 安装应用到 fsimg
install: app
	@echo + INSTALL "->" $(NAME)
	@mkdir -p $(NAVY_HOME)/fsimg/bin
	@cp $(APP) $(NAVY_HOME)/fsimg/bin/$(NAME)

.PHONY: install

### 清理单个项目（删除 build/）
clean:
	rm -rf Makefile.html $(WORK_DIR)/build/ build/
.PHONY: clean

### 清理深度 2 内的所有子项目（忽略错误）
CLEAN_ALL = $(dir $(shell find . -mindepth 2 -maxdepth 3 -name Makefile))
clean-all: $(CLEAN_ALL) clean
	-@rm -f $(NAVY_HOME)/fsimg/bin/*
$(CLEAN_ALL):
	-@$(MAKE) -s -C $@ clean
.PHONY: clean-all $(CLEAN_ALL)

### 为 Nanos-lite 构建 fsimg 和 ramdisk
APPS = menu nslider nterm bird
TESTS = dummy hello file-test event-test bmp-test

fsimg: $(addprefix apps/, $(APPS)) $(addprefix tests/, $(TESTS))
	-for t in $^; do $(MAKE) -s -C $(NAVY_HOME)/$$t install; done

RAMDISK = build/ramdisk.img
RAMDISK_H = build/ramdisk.h
$(RAMDISK): fsimg
	$(eval FSIMG_FILES := $(shell find -L ./fsimg -type f))
	@mkdir -p $(@D)
	@cat $(FSIMG_FILES) > $@
	@truncate -s \%512 $@
	@echo "// 文件路径, 文件大小, 磁盘偏移" > $(RAMDISK_H)
	@wc -c $(FSIMG_FILES) | grep -v 'total$$' | sed -e 's+ ./fsimg+ +' | awk -v sum=0 '{print "\x7b\x22" $$2 "\x22\x2c " $$1 "\x2c " sum "\x7d\x2c";sum += $$1}' >> $(RAMDISK_H)

ramdisk: $(RAMDISK)

.PHONY: fsimg ramdisk