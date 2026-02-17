#ifndef __FTRACE_H__
#define __FTRACE_H__

#include <elf.h>
#include <stdbool.h>
#include "../../include/memory/paddr.h"

typedef struct {
	char name[32]; // func name, 32 should be enough
	paddr_t addr;
	unsigned char info;
	Elf32_Xword size;
} SymEntry;

void parse_elf_file(int fd) ;
SymEntry* find_symbol_func(paddr_t target) ;
void parse_elf(const char **elf_files, int elf_file_count) ;
void trace_func_call(paddr_t pc, paddr_t target, bool is_tail) ;
void trace_func_ret(paddr_t pc) ;


#endif
