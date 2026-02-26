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
// #elif defined(__ISA_LOONGARCH32R__) // can not find...
// # define EXPECT_TYPE EM_LOONGARCH  
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



// static uintptr_t loader(PCB *pcb, const char *filename) {
//   // TODO();
//   // add for PA3.2
//   // parse ELF header
//   Elf_Ehdr ehdr;
//   ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr));


//   assert(*(uint32_t *)ehdr->e_ident == 0x464C457F); // little
//   puts("ELF头:");
//   printf("Magic:\t\t\t");
//   for(int i = 0; i < EI_NIDENT; ++i) printf("%02x ", ehdr->e_ident[i]);
//   printf("\n类别:\t\t\t");
//   switch(ehdr->e_type) {
//       case 0 : printf("未知文件类型\n"); break;
//       case 1 : printf("可重定位文件\n"); break;
//       case 2 : printf("可执行文件\n"); break;
//       case 3 : printf("动态链接库文件\n"); break;
//       case 4 : printf("Core文件\n"); break;
//       case 0xff00 : printf("特定处理器文件扩展下边界\n"); break;
//       case 0xffff : printf("特定处理器文件扩展上边界\n"); break;
//   }

//   assert(ehdr.e_machine != EXPECT_TYPE);

//   printf("处理器体系结构:\t\t");
//   switch(ehdr->e_machine) {
//       case 0 : printf("未知体系结构\n"); break;
//       case 1 : printf("AT&T WE 32100\n"); break;
//       case 2 : printf("SPARC\n"); break;
//       case 3 : printf("Intel Architecture\n"); break;
//       case 4 : printf("Motorola 68000\n"); break;
//       case 5 : printf("Motorola 88000\n"); break;
//       case 7 : printf("Intel 80860\n"); break;
//       case 8 : printf("MIPS RS3000 Big-Endian\n"); break;
//       case 10 : printf("MIPS RS4000 Big-Endian\n"); break;
//       case 62 : printf("AMD x86-64 architecture\n"); break;
//       case 243:printf("RISCV architecture\n"); break;
// 		break;

//   }
//   printf("version:\t\t");
//   switch(ehdr->e_version) {
//       case 0 : printf("非法版本号\n"); break;
//       case 1 : printf("当前版本号\n"); break;
//   }
//   printf("入口虚拟地址:\t\t0x%016x\n", ehdr->e_entry);
//   printf("程序头表偏移量:\t\t0x%08x\n", ehdr->e_phoff);
//   printf("节头表偏移量:\t\t0x%08x\n", ehdr->e_shoff);
//   printf("处理器标志位:\t\t%x\n", ehdr->e_flags);
//   printf("ELF文件头大小:\t\t%u bytes\n", ehdr->e_ehsize);
//   printf("程序头标每一表项大小:\t%u bytes\n", ehdr->e_phentsize);
//   printf("程序头表表项数量:\t%u\n", ehdr->e_phnum);
//   printf("节头表每一表项大小:\t%u bytes\n", ehdr->e_shentsize);
//   printf("节头表表项数量:\t\t%u\n", ehdr->e_shnum);
//   printf("字符串表在节头表中索引:\t%u\n", ehdr->e_shstrndx);
//   // load execuable segments from program header
//   for(uint16_t i = 0; i < ehdr.e_phnum; i ++) {
//     Elf_Phdr phdr;
//     ramdisk_read(&phdr, ehdr.e_phoff + i * sizeof(Elf_Phdr), sizeof(Elf_Phdr));
//     if(phdr->p_type == PT_LOAD) { 
//       memset(phdr->p_vaddr + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
//       ramdisk_read((void *) phdr->p_vaddr, phdr->p_offset,phdr->p_filesz);
//     }
//   }

//   return ehdr->e_entry;
// }

static uintptr_t loader(PCB *pcb, const char *filename) {
  // 打开文件
  int fd = fs_open(filename, 0, 0);
  assert(fd >= 0);
  
  // 读取并验证 ELF 头
  Elf_Ehdr ehdr;
  fs_read(fd, &ehdr, sizeof(Elf_Ehdr));
  assert(*(uint32_t *)ehdr.e_ident == 0x464C457F);
  
  // 输出ELF基本信息
  printf("========== ELF Loader: %s ==========\n", filename);
  
  // 输出ELF标识信息
  printf("ELF Magic: ");
  for(int i = 0; i < EI_NIDENT; i++) {
    printf("%02x ", ehdr.e_ident[i]);
  }
  printf("\n");
  
  // 输出文件类型
  printf("ELF Type: ");
  switch(ehdr.e_type) {
    case ET_NONE: printf("Unknown\n"); break;
    case ET_REL:  printf("Relocatable\n"); break;
    case ET_EXEC: printf("Executable\n"); break;
    case ET_DYN:  printf("Shared Object\n"); break;
    case ET_CORE: printf("Core File\n"); break;
    default:      printf("Other (0x%x)\n", ehdr.e_type);
  }
  
  // 输出机器架构
  printf("Machine: ");
  switch(ehdr.e_machine) {
    case EM_386:    printf("Intel 80386\n"); break;
    case EM_X86_64: printf("AMD x86-64\n"); break;
    case EM_RISCV:  printf("RISC-V\n"); break;
    case EM_ARM:    printf("ARM\n"); break;
    default:        printf("Unknown (0x%x)\n", ehdr.e_machine);
  }
  
  // 架构检查
#ifdef __riscv
  assert(ehdr.e_machine == EM_RISCV);
#else
  assert(ehdr.e_machine == EM_X86_64 || ehdr.e_machine == EM_386);
#endif
  
  // 输出ELF头关键信息
  printf("Entry point:     0x%016lx\n", ehdr.e_entry);
  printf("Program headers: %d entries at offset 0x%lx\n", 
         ehdr.e_phnum, ehdr.e_phoff);
  printf("Section headers: %d entries at offset 0x%lx\n",
         ehdr.e_shnum, ehdr.e_shoff);
  printf("ELF header size: %d bytes\n", ehdr.e_ehsize);
  printf("Program header size: %d bytes\n", ehdr.e_phentsize);
  printf("Section header size: %d bytes\n", ehdr.e_shentsize);
  
  // 统计可加载段
  int load_segments = 0;
  size_t total_filesz = 0;
  size_t total_memsz = 0;
  
  // 加载所有可加载段
  printf("\nLoading segments:\n");
  for (uint16_t i = 0; i < ehdr.e_phnum; i++) {
    Elf_Phdr phdr;
    
    // 读取程序头
    fs_lseek(fd, ehdr.e_phoff + i * sizeof(Elf_Phdr), SEEK_SET);
    fs_read(fd, &phdr, sizeof(Elf_Phdr));
    
    // 输出程序头信息
    printf("  PH[%d]: type=0x%x, flags=", i, phdr.p_type);
    if(phdr.p_flags & PF_R) printf("R");
    if(phdr.p_flags & PF_W) printf("W");
    if(phdr.p_flags & PF_X) printf("X");
    
    if (phdr.p_type == PT_LOAD) {
      load_segments++;
      total_filesz += phdr.p_filesz;
      total_memsz += phdr.p_memsz;
      
      printf(" (LOAD) vaddr=0x%016lx, memsz=%lu(0x%lx), filesz=%lu(0x%lx), offset=0x%lx\n",
             phdr.p_vaddr, phdr.p_memsz, phdr.p_memsz, 
             phdr.p_filesz, phdr.p_filesz, phdr.p_offset);
      
      // 加载段数据
      fs_lseek(fd, phdr.p_offset, SEEK_SET);
      fs_read(fd, (void *)phdr.p_vaddr, phdr.p_filesz);
      
      // 清零 BSS
      if (phdr.p_memsz > phdr.p_filesz) {
        size_t bss_size = phdr.p_memsz - phdr.p_filesz;
        printf("    BSS: zeroing %lu(0x%lx) bytes at 0x%016lx\n",
               bss_size, bss_size, phdr.p_vaddr + phdr.p_filesz);
        memset((void *)(phdr.p_vaddr + phdr.p_filesz), 0, bss_size);
      }
    } else {
      printf(" (OTHER)\n");
    }
  }
  
  // 输出加载统计
  printf("\nLoad summary:\n");
  printf("  Total load segments: %d\n", load_segments);
  printf("  Total file size: %lu (0x%lx) bytes\n", total_filesz, total_filesz);
  printf("  Total memory size: %lu (0x%lx) bytes\n", total_memsz, total_memsz);
  printf("  BSS size: %lu (0x%lx) bytes\n", 
         total_memsz - total_filesz, total_memsz - total_filesz);
  printf("  Entry point: 0x%016lx\n", ehdr.e_entry);
  
  fs_close(fd);
  
  printf("====================================\n");
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

