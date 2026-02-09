/***************************************************************************************
* PA2 ftrace
***************************************************************************************/
#include <elf.h>
#include <utils.h>  // for using Log && log_write
// #include <common.h>


FILE *fp;
char strtable[9999];

void init_ftrace(Elf32_Ehdr *ehdr) {
    // Log("out!!!");
    // assert(ehdr != NULL);
    if (ehdr) {
        ELF_header_32_parse(ehdr);  // 解析ELF
        section_header_32_parse(ehdr);  // 解析ELF
}
    return ;
}


// ATTENTION: https://zhuanlan.zhihu.com/p/544198038

// ELF structure:
    // ELF header (Ehdr)
    // The ELF header is described by the type Elf32_Ehdr or Elf64_Ehdr:
    //     typedef struct {
    //         unsigned char e_ident[EI_NIDENT];
    //         uint16_t      e_type;
    //         uint16_t      e_machine;
    //         uint32_t      e_version;
    //         ElfN_Addr     e_entry;
    //         ElfN_Off      e_phoff;
    //         ElfN_Off      e_shoff;
    //         uint32_t      e_flags;
    //         uint16_t      e_ehsize;
    //         uint16_t      e_phentsize;
    //         uint16_t      e_phnum;
    //         uint16_t      e_shentsize;
    //         uint16_t      e_shnum;
    //         uint16_t      e_shstrndx;
    //     } ElfN_Ehdr;

    // Section header (Shdr)
    // typedef struct {
    //     uint32_t   sh_name;
    //     uint32_t   sh_type;
    //     uint32_t   sh_flags;
    //     Elf32_Addr sh_addr;
    //     Elf32_Off  sh_offset;
    //     uint32_t   sh_size;
    //     uint32_t   sh_link;
    //     uint32_t   sh_info;
    //     uint32_t   sh_addralign;
    //     uint32_t   sh_entsize;
    // } Elf32_Shdr;

    // Program header (Phdr)
    //         typedef struct {
    //             uint32_t   p_type;
    //             Elf32_Off  p_offset;
    //             Elf32_Addr p_vaddr;
    //             Elf32_Addr p_paddr;
    //             uint32_t   p_filesz;
    //             uint32_t   p_memsz;
    //             uint32_t   p_flags;
    //             uint32_t   p_align;
    //         } Elf32_Phdr;

//    String and symbol tables
//            typedef struct {
//                uint32_t      st_name;
//                Elf32_Addr    st_value;
//                uint32_t      st_size;
//                unsigned char st_info;
//                unsigned char st_other;
//                uint16_t      st_shndx;
//            } Elf32_Sym;

// ELF Header -> ELF Section Header Table -> ELF Symbol Table 
/**
 * 32位 ELF 头结构体详解
 * 
 * 内存布局（总计52字节）：
 * 偏移     大小      字段          描述
 * -----------------------------------------------------------------
 * 0x00     16字节   e_ident       ELF标识和基本信息
 * 0x10     2字节    e_type        文件类型（可执行/共享库等）
 * 0x12     2字节    e_machine     目标机器架构（x86/ARM/RISC-V等）
 * 0x14     4字节    e_version     ELF格式版本
 * 0x18     4字节    e_entry       程序入口点虚拟地址
 * 0x1C     4字节    e_phoff       程序头表在文件中的偏移
 * 0x20     4字节    e_shoff       节头表在文件中的偏移
 * 0x24     4字节    e_flags       处理器特定标志
 * 0x28     2字节    e_ehsize      ELF头自身大小（总是52）
 * 0x2A     2字节    e_phentsize   每个程序头的大小
 * 0x2C     2字节    e_phnum       程序头数量
 * 0x2E     2字节    e_shentsize   每个节头的大小
 * 0x30     2字节    e_shnum       节头数量
 * 0x32     2字节    e_shstrndx    节名字符串表的节索引
 */
void ELF_header_32_parse(Elf32_Ehdr* ehdr) {
    fseek(fp, 0, SEEK_SET);
    int ret = fread(ehdr, sizeof(Elf32_Ehdr), 1, fp);
    assert(ret == 1);

    Log("ELF header:");
    Log("Magic:\t\t\t");
    for(int i = 0; i < EI_NIDENT; ++i) Log("%02x ", ehdr->e_ident[i]);
    Log("\n类别:\t\t\t");
    switch(ehdr->e_type) {
        case 0 : Log("未知文件类型\n"); break;
        case 1 : Log("可重定位文件\n"); break;
        case 2 : Log("可执行文件\n"); break;
        case 3 : Log("动态链接库文件\n"); break;
        case 4 : Log("Core文件\n"); break;
        case 0xff00 : Log("特定处理器文件扩展下边界\n"); break;
        case 0xffff : Log("特定处理器文件扩展上边界\n"); break;
    }
    Log("处理器体系结构:\t\t");
    switch(ehdr->e_machine) {
        case 0 : Log("未知体系结构\n"); break;
        case 1 : Log("AT&T WE 32100\n"); break;
        case 2 : Log("SPARC\n"); break;
        case 3 : Log("Intel Architecture\n"); break;
        case 4 : Log("Motorola 68000\n"); break;
        case 5 : Log("Motorola 88000\n"); break;
        case 7 : Log("Intel 80860\n"); break;
        case 8 : Log("MIPS RS3000 Big-Endian\n"); break;
        case 10 : Log("MIPS RS4000 Big-Endian\n"); break;
        case 62 : Log("AMD x86-64 architecture\n"); break;
    }
    Log("version:\t\t");
    switch(ehdr->e_version) {
        case 0 : Log("非法版本号\n"); break;
        case 1 : Log("当前版本号\n"); break;
    }
    Log("入口虚拟地址:\t\t0x%08x\n", ehdr->e_entry);
    Log("程序头表偏移量:\t\t0x%04x\n", ehdr->e_phoff);
    Log("节头表偏移量:\t\t0x%04x\n", ehdr->e_shoff);
    Log("处理器标志位:\t\t%x\n", ehdr->e_flags);
    Log("ELF文件头大小:\t\t%u bytes\n", ehdr->e_ehsize);
    Log("程序头标每一表项大小:\t%u bytes\n", ehdr->e_phentsize);
    Log("程序头表表项数量:\t%u\n", ehdr->e_phnum);
    Log("节头表每一表项大小:\t%u bytes\n", ehdr->e_shentsize);
    Log("节头表表项数量:\t\t%u\n", ehdr->e_shnum);
    Log("字符串表在节头表中索引:\t%u\n", ehdr->e_shstrndx);


}

void section_header_32_parse(Elf32_Ehdr* ehdr) {
    Elf32_Shdr shdr[99];
    int i, count = ehdr->e_shnum;    //节头表数量
    fseek(fp, ehdr->e_shoff, SEEK_SET);
    int ret1 = fread(shdr, sizeof(Elf32_Shdr), count, fp);
    assert(ret1 == 1);
    // 跳转到存储所有节名称的那个字符串表在ELF文件中的位置
    fseek(fp, shdr[ehdr->e_shstrndx].sh_offset, SEEK_SET);
    // 读取字符串表内容到内存
    int ret2 = fread(strtable, 1, shdr[ehdr->e_shstrndx].sh_size, fp);
    assert(ret2 == 1);
    Log("There are %d section headers, starting at offset 0x%04x:\n\n", count, ehdr->e_shoff);
    Log("Section header:");
    Log("[编号]\t名称\t 类型\t\t属性\t虚拟地址\t偏移量\t大小\t\t索引值\t信息\t对齐长度\t表项大小\n");
    for(i = 0; i < count; ++i) {
        Log("[%02d]\t%s", i, &strtable[shdr[i].sh_name]);
        if (shdr[i].sh_type == SHT_STRTAB) {  // 如果是符号表节
            break;
        }
    }
    /* 1. 跳转到符号表在文件中的位置 */
    fseek(fp, shdr[i].sh_offset, SEEK_SET);
    
    /* 2. 读取符号表 */
    int num_symbols = shdr[i].sh_size / sizeof(Elf32_Sym);
    Elf32_Sym *symtab = malloc(shdr[i].sh_size);
    int ret3 = fread(symtab, sizeof(Elf32_Sym), num_symbols, fp);
    assert(ret3 == 1);

    /* 3. 找到对应的字符串表（通过 sh_link） */
    int strtab_idx = shdr[i].sh_link;  // 字符串表的节索引
    if (strtab_idx < count) {
        fseek(fp, shdr[strtab_idx].sh_offset, SEEK_SET);
        char *sym_strtab = malloc(shdr[strtab_idx].sh_size);
        int ret4 = fread(sym_strtab, 1, shdr[strtab_idx].sh_size, fp);
        assert(ret4 == 1);

        /* 4. 解析每个符号 */
        for (int j = 0; j < num_symbols; j++) {
            unsigned char type = ELF32_ST_TYPE(symtab[j].st_info);
            if (type == STT_FUNC) {  // 只关心函数符号
                const char *func_name = sym_strtab + symtab[j].st_name;
                Log("Function: %s at 0x%08x, size: %u\n",
                    func_name, symtab[j].st_value, symtab[j].st_size);
            }
        }
        
        free(sym_strtab);
    }
    free(symtab);

    return ;

}



void pop_ftrace() {
    return ;
}