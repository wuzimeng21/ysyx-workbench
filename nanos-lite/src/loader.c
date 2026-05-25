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
  Area kstack = (Area){pcb, pcb + 1};
  pcb->cp = kcontext(kstack, (void (*)(void *))entry, NULL);
  pcb->cp->GPRx = 0;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p, filename = %s", entry, filename);

  // CRT0 expects a0 to point to args: [argc][padding 3 words][argv][envp]
  static uintptr_t args[] = {0, 0, 0, 0, 0, 0, 0, 0};

  // Jump to user entry with a0 = args pointer
  asm volatile(
    "mv a0, %0\n\t"
    "jr %1"
    :
    : "r"((uintptr_t)args), "r"(entry)
    : "a0", "memory"
  );
  __builtin_unreachable();
}
