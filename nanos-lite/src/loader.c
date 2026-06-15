#include <fs.h>
#include <proc.h>
#include <elf.h>

#if defined(__ISA_AM_NATIVE__)
# define EXPECT_TYPE EM_RISCV
#elif defined(__ISA_X86__)
# define EXPECT_TYPE EM_386
#elif defined(__ISA_X86_64__)
# define EXPECT_TYPE EM_X86_64
#elif defined(__ISA_MIPS32__)
# define EXPECT_TYPE EM_MIPS
#elif defined(__riscv)
# define EXPECT_TYPE EM_RISCV
#else
# error "Unsupported ISA"
#endif

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  assert(fd >= 0);

  Elf_Ehdr ehdr;
  fs_read(fd, &ehdr, sizeof(Elf_Ehdr));
  assert(*(uint32_t *)ehdr.e_ident == 0x464C457F);
  assert(ehdr.e_machine == EXPECT_TYPE);

  for (uint16_t i = 0; i < ehdr.e_phnum; i++) {
    Elf_Phdr phdr;
    fs_lseek(fd, ehdr.e_phoff + i * sizeof(Elf_Phdr), SEEK_SET);
    fs_read(fd, &phdr, sizeof(Elf_Phdr));
    if (phdr.p_type == PT_LOAD) {
      fs_lseek(fd, phdr.p_offset, SEEK_SET);
      fs_read(fd, (void *)(uintptr_t)phdr.p_vaddr, phdr.p_filesz);
      if (phdr.p_memsz > phdr.p_filesz) {
        memset((void *)(uintptr_t)(phdr.p_vaddr + phdr.p_filesz), 0,
               phdr.p_memsz - phdr.p_filesz);
      }
    }
  }
  fs_close(fd);
  return ehdr.e_entry;
}

void context_uload(PCB *pcb, const char *filename, char *const argv[],
                   char *const envp[]) {
  uintptr_t entry = loader(pcb, filename);

  // Count argc and envc
  int argc = 0;
  if (argv) { while (argv[argc]) argc++; }
  int envc = 0;
  if (envp) { while (envp[envc]) envc++; }

  // Build user stack with argc/argv/envp (same layout as naive_uload)
  // Layout: [argc, argv_ptrs..., NULL, envp_ptrs..., NULL, str_data...]
  int nr_args = 1 + argc + 1 + envc + 1;
  uintptr_t *user_sp =
      (uintptr_t *)(&pcb->stack[sizeof(pcb->stack)] - nr_args * sizeof(uintptr_t));

  int idx = 0;
  user_sp[idx++] = argc;
  for (int i = 0; i < argc; i++) user_sp[idx++] = (uintptr_t)argv[i];
  user_sp[idx++] = 0;
  for (int i = 0; i < envc; i++) user_sp[idx++] = (uintptr_t)envp[i];
  user_sp[idx++] = 0;

  // Initialize the address space
  pcb->as.pgsize = PGSIZE;
  pcb->as.area = (Area){&pcb->stack[0], &pcb->stack[sizeof(pcb->stack)]};
  pcb->as.ptr = NULL;  // No hardware page table in NEMU

  Context *c = ucontext(&pcb->as, pcb->as.area, (void *)(uintptr_t)entry);
  c->GPRx = (uintptr_t)user_sp;  // a0 = pointer to argc/argv/envp on user stack
  pcb->cp = c;
}

void naive_uload(PCB *pcb, const char *filename, char *const argv[],
                 char *const envp[]) {
  // Copy filename + argv/envp strings to kernel buffer BEFORE loader(),
  // because loader() writes ELF segments that may overwrite user memory
  // where these strings live.

  int argc = 0;
  if (argv) { while (argv[argc]) argc++; }
  int envc = 0;
  if (envp) { while (envp[envc]) envc++; }

  // Calculate total string bytes needed (including filename)
  int str_bytes = strlen(filename) + 1;
  for (int i = 0; i < argc; i++) str_bytes += strlen(argv[i]) + 1;
  for (int i = 0; i < envc; i++) str_bytes += strlen(envp[i]) + 1;

  // Layout (high to low address):
  //   [argc, argv_ptrs..., NULL, envp_ptrs..., NULL, str_data...]
  int nr_args = 1 + argc + 1 + envc + 1;
  size_t meta_size = nr_args * sizeof(uintptr_t);
  static uint8_t user_stack[8192] __attribute__((aligned(16)));

  uintptr_t *args =
      (uintptr_t *)(user_stack + sizeof(user_stack) - meta_size);
  char *str_area = (char *)args - str_bytes;
  // Make sure we don't overflow the buffer
  assert((uintptr_t)str_area >= (uintptr_t)user_stack);

  // Copy string data into kernel buffer
  char *sp = str_area;
  size_t len;

  len = strlen(filename) + 1; memcpy(sp, filename, len);
  char *kfile = sp; sp += len;

  for (int i = 0; i < argc; i++) {
    len = strlen(argv[i]) + 1;
    memcpy(sp, argv[i], len);
    sp += len;
  }
  for (int i = 0; i < envc; i++) {
    len = strlen(envp[i]) + 1;
    memcpy(sp, envp[i], len);
    sp += len;
  }

  // Now safe to load ELF (may overwrite user memory)
  uintptr_t entry = loader(pcb, kfile);
  Log("Jump to entry = %x, filename = %s", entry, kfile);

  // Fill in args array with pointers to copied strings
  int idx = 0;
  args[idx++] = argc;
  sp = str_area + strlen(kfile) + 1;  // skip filename
  for (int i = 0; i < argc; i++) {
    args[idx++] = (uintptr_t)sp;
    sp += strlen(sp) + 1;
  }
  args[idx++] = 0;
  for (int i = 0; i < envc; i++) {
    args[idx++] = (uintptr_t)sp;
    sp += strlen(sp) + 1;
  }
  args[idx++] = 0;

  asm volatile(
    "mv a0, %0\n\t"
    "jr %1"
    :
    : "r"((uintptr_t)args), "r"(entry)
    : "a0", "memory"
  );
  __builtin_unreachable();
}
