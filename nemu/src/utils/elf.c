// #include <common.h>
#include <fcntl.h>
#include <unistd.h>
// #include <elf.h>
#include "ftrace.h"

// typedef struct {
// 	char name[32]; // func name, 32 should be enough
// 	paddr_t addr;
// 	unsigned char info;
// 	Elf32_Xword size;
// } SymEntry;

SymEntry *symbol_tbl[2]; // dynamic allocated
int symbol_tbl_size[2];
int elf_count = 0;

static void read_elf_header(int fd, Elf32_Ehdr *eh)
{
	assert(lseek(fd, 0, SEEK_SET) == 0);
	assert(read(fd, (void *)eh, sizeof(Elf32_Ehdr)) == sizeof(Elf32_Ehdr));

	// check if is elf using fixed format of Magic: 7f 45 4c 46 ...
	if (strncmp((char *)eh->e_ident, "\177ELF", 4))
	{
		Log("malformed ELF file");
		return;
	}
}

static void display_elf_hedaer(Elf32_Ehdr eh)
{
	/* Storage capacity class */
	Log("Storage class\t= ");

	switch (eh.e_ident[EI_CLASS])
	{

	case ELFCLASS32:
		Log("32=bit objects\n");
		break;

	case ELFCLASS64:
		Log("64=bit objects\n");
		break;

	default:
		Log("INVALID CLASS\n");
		break;
	}

	/* Data Format */
	Log("Data format\t= ");
	switch (eh.e_ident[EI_DATA])
	{

	case ELFDATA2LSB:
		Log("2's complement, little endian\n");
		break;

	case ELFDATA2MSB:
		Log("2's complement, big endian\n");
		break;

	default:
		Log("INVALID Format\n");
		break;
	}

	/* OS ABI */
	Log("OS ABI\t\t= ");
	switch (eh.e_ident[EI_OSABI])
	{

	case ELFOSABI_SYSV:
		Log("UNIX System V ABI\n");
		break;

	case ELFOSABI_HPUX:
		Log("HP=UX\n");
		break;

	case ELFOSABI_NETBSD:
		Log("NetBSD\n");
		break;

	case ELFOSABI_LINUX:
		Log("Linux\n");
		break;

	case ELFOSABI_SOLARIS:
		Log("Sun Solaris\n");
		break;

	case ELFOSABI_AIX:
		Log("IBM AIX\n");
		break;

	case ELFOSABI_IRIX:
		Log("SGI Irix\n");
		break;

	case ELFOSABI_FREEBSD:
		Log("FreeBSD\n");
		break;

	case ELFOSABI_TRU64:
		Log("Compaq TRU64 UNIX\n");
		break;

	case ELFOSABI_MODESTO:
		Log("Novell Modesto\n");
		break;

	case ELFOSABI_OPENBSD:
		Log("OpenBSD\n");
		break;

	case ELFOSABI_ARM_AEABI:
		Log("ARM EABI\n");
		break;

	case ELFOSABI_ARM:
		Log("ARM\n");
		break;

	case ELFOSABI_STANDALONE:
		Log("Standalone (embedded) app\n");
		break;

	default:
		Log("Unknown (0x%x)\n", eh.e_ident[EI_OSABI]);
		break;
	}

	/* ELF filetype */
	Log("Filetype \t= ");
	switch (eh.e_type)
	{

	case ET_NONE:
		Log("N/A (0x0)\n");
		break;

	case ET_REL:
		Log("Relocatable\n");
		break;

	case ET_EXEC:
		Log("Executable\n");
		break;

	case ET_DYN:
		Log("Shared Object\n");
		break;
	default:
		Log("Unknown (0x%x)\n", eh.e_type);
		break;
	}

	/* ELF Machine=id */
	Log("Machine\t\t= ");
	switch (eh.e_machine)
	{

	case EM_NONE:
		Log("None (0x0)\n");
		break;

	case EM_386:
		Log("INTEL x86 (0x%x)\n", EM_386);
		break;

	case EM_X86_64:
		Log("AMD x86_64 (0x%x)\n", EM_X86_64);
		break;

	case EM_AARCH64:
		Log("AARCH64 (0x%x)\n", EM_AARCH64);
		break;

	case EM_RISCV:
		Log("RISCV (0x%x)\n", EM_RISCV);
		break;

	default:
		Log(" 0x%x\n", eh.e_machine);
		break;
	}

	/* Entry point */
	Log("Entry point\t= %#x\n", eh.e_entry);

	/* ELF header size in bytes */
	Log("ELF header size\t= 0x%08x\n", eh.e_ehsize);

	/* Program Header */
	Log("Program Header\t= ");
	Log("\t\t0x%08x\n", eh.e_phoff);		 /* start */
	Log("\t\t  %d entries\n", eh.e_phnum);	 /* num entry */
	Log("\t\t  %d bytes\n", eh.e_phentsize); /* size/entry */

	/* Section header starts at */
	Log("Section Header\t= ");
	Log("\t\t0x%08x\n", eh.e_shoff);		 /* start */
	Log("\t\t  %d entries\n", eh.e_shnum);	 /* num entry */
	Log("\t\t  %d bytes\n", eh.e_shentsize); /* size/entry */
	Log("\t\t  0x%08x (string table offset)\n", eh.e_shstrndx);
	/* File flags (Machine specific)*/
	Log("File flags \t= 0x%08x\n", eh.e_flags);

	/* ELF file flags are machine specific.
	 * INTEL implements NO flags.
	 * ARM implements a few.
	 * Add support below to parse ELF file flags on ARM
	 */
	int32_t ef = eh.e_flags;
	/* MSB of flags conatins ARM EABI version */
	Log("ARM EABI\t= Version %d\n", (ef & EF_ARM_EABIMASK) >> 24);

	Log("\t\t  ");

	if (ef & EF_ARM_RELEXEC)
		Log(",RELEXEC ");
	if (ef & EF_ARM_HASENTRY)
		Log(",HASENTRY ");
	if (ef & EF_ARM_INTERWORK)
		Log(",INTERWORK ");
	if (ef & EF_ARM_APCS_26)
		Log(",APCS_26 ");
	if (ef & EF_ARM_APCS_FLOAT)
		Log(",APCS_FLOAT ");
	if (ef & EF_ARM_PIC)
		Log(",PIC ");
	if (ef & EF_ARM_ALIGN8)
		Log(",ALIGN8 ");
	if (ef & EF_ARM_NEW_ABI)
		Log(",NEW_ABI ");
	if (ef & EF_ARM_OLD_ABI)
		Log(",OLD_ABI ");
	if (ef & EF_ARM_SOFT_FLOAT)
		Log(",SOFT_FLOAT ");
	if (ef & EF_ARM_VFP_FLOAT)
		Log(",VFP_FLOAT ");
	if (ef & EF_ARM_MAVERICK_FLOAT)
		Log(",MAVERICK_FLOAT ");
}

static void read_section(int fd, Elf32_Shdr sh, void *dst)
{
	assert(dst != NULL);
	assert(lseek(fd, (off_t)sh.sh_offset, SEEK_SET) == (off_t)sh.sh_offset);
	assert(read(fd, dst, sh.sh_size) == sh.sh_size);
}

static void read_section_headers(int fd, Elf32_Ehdr eh, Elf32_Shdr *sh_tbl)
{
	assert(lseek(fd, eh.e_shoff, SEEK_SET) == eh.e_shoff);
	// lseek(fd, eh.e_shoff, SEEK_SET);

	for (int i = 0; i < eh.e_shnum; i++)
	{
		assert(read(fd, (void *)&sh_tbl[i], eh.e_shentsize) == eh.e_shentsize);
	}
}
static const char* get_section_type_str(Elf32_Word type) {
    switch(type) {
        case SHT_NULL:          return "NULL";
        case SHT_PROGBITS:      return "PROGBITS";
        case SHT_SYMTAB:        return "SYMTAB";
        case SHT_STRTAB:        return "STRTAB";
        case SHT_RELA:          return "RELA";
        case SHT_HASH:          return "HASH";
        case SHT_DYNAMIC:       return "DYNAMIC";
        case SHT_NOTE:          return "NOTE";
        case SHT_NOBITS:        return "NOBITS";
        case SHT_REL:           return "REL";
        case SHT_SHLIB:         return "SHLIB";
        case SHT_DYNSYM:        return "DYNSYM";
        case SHT_INIT_ARRAY:    return "INIT_ARRAY";
        case SHT_FINI_ARRAY:    return "FINI_ARRAY";
        case SHT_PREINIT_ARRAY: return "PREINIT_ARRAY";
        case SHT_GROUP:         return "GROUP";
        case SHT_SYMTAB_SHNDX:  return "SYMTAB_SHNDX";
        default: {
            static char buf[32];
            snprintf(buf, sizeof(buf), "0x%x", type);
            return buf;
        }
    }
}

static const char* get_section_flags_str(Elf32_Word flags) {
    static char buf[16];  // 足够存放 "WAX" 这样的标志
    buf[0] = '\0';
    
    if (flags & SHF_WRITE)     strcat(buf, "W");
    if (flags & SHF_ALLOC)     strcat(buf, "A");
    if (flags & SHF_EXECINSTR) strcat(buf, "X");
    if (flags & SHF_MERGE)     strcat(buf, "M");
    if (flags & SHF_STRINGS)   strcat(buf, "S");
    if (flags & SHF_INFO_LINK) strcat(buf, "I");
    if (flags & SHF_LINK_ORDER) strcat(buf, "L");
    if (flags & SHF_OS_NONCONFORMING) strcat(buf, "O");
    if (flags & SHF_GROUP)     strcat(buf, "G");
    if (flags & SHF_TLS)       strcat(buf, "T");
    if (flags & SHF_COMPRESSED) strcat(buf, "C");
    
    return buf[0] ? buf : "";
}

static void display_section_headers(int fd, Elf32_Ehdr eh, Elf32_Shdr sh_tbl[])
{
	char sh_str[sh_tbl[eh.e_shstrndx].sh_size];
	read_section(fd, sh_tbl[eh.e_shstrndx], sh_str);
	Log("-------------------------------------------------------\n");
	for (int i = 0; i < eh.e_shnum; i++)
	{
		Log("idx:[%2d]\tname:%-10s\toffset:0x%08x  load=addr:0x%08x  size:0x%08x  align:%-4d\tflags:%-4s  type:%8s\n",
			 i, (sh_str + sh_tbl[i].sh_name), sh_tbl[i].sh_offset,
			sh_tbl[i].sh_addr, sh_tbl[i].sh_size, sh_tbl[i].sh_addralign,
			 get_section_flags_str(sh_tbl[i].sh_flags),  get_section_type_str(sh_tbl[i].sh_type));
	}
	Log("-------------------------------------------------------\n");
}



static const char *get_symbol_type_str(unsigned char info)
{
	unsigned char type = ELF32_ST_TYPE(info);

	switch (type)
	{
	case STT_NOTYPE:
		return "NOTYPE";
	case STT_OBJECT:
		return "OBJECT";
	case STT_FUNC:
		return "FUNC";
	case STT_SECTION:
		return "SECTION";
	case STT_FILE:
		return "FILE";
	case STT_COMMON:
		return "COMMON";
	case STT_TLS:
		return "TLS";
	default:
		return "UNKNOWN";
	}
}

static void read_symbol_table(int fd, Elf32_Ehdr eh, Elf32_Shdr sh_tbl[], int sym_idx)
{
	Elf32_Sym sym_tbl[sh_tbl[sym_idx].sh_size];
	read_section(fd, sh_tbl[sym_idx], sym_tbl);

	int str_idx = sh_tbl[sym_idx].sh_link;
	char str_tbl[sh_tbl[str_idx].sh_size];
	read_section(fd, sh_tbl[str_idx], str_tbl);

	int sym_count = (sh_tbl[sym_idx].sh_size / sizeof(Elf32_Sym));
	// log
	Log("Symbol count: %d\n", sym_count);
	Log("-------------------------------------------------------\n");
	for (int i = 0; i < sym_count; i++)
	{
		Log("num:%-3d\tvalue:0x%08x\ttype:%-8s\tsize:%-10u\tname:%s\n",
			i,
			sym_tbl[i].st_value,
			get_symbol_type_str(sym_tbl[i].st_info),
			sym_tbl[i].st_size,
			str_tbl + sym_tbl[i].st_name);
	}
	Log("-------------------------------------------------------\n");

	// read
	symbol_tbl_size[elf_count] = sym_count;
	symbol_tbl[elf_count] = malloc(sizeof(SymEntry) * sym_count);
	for (int i = 0; i < sym_count; i++)
	{
		symbol_tbl[elf_count][i].addr = sym_tbl[i].st_value;
		symbol_tbl[elf_count][i].info = sym_tbl[i].st_info;
		symbol_tbl[elf_count][i].size = sym_tbl[i].st_size;
		memset(symbol_tbl[elf_count][i].name, 0, 32);
		strncpy(symbol_tbl[elf_count][i].name, str_tbl + sym_tbl[i].st_name, 31);
	}
}

static void read_symbols(int fd, Elf32_Ehdr eh, Elf32_Shdr sh_tbl[])
{
	for (int i = 0; i < eh.e_shnum; i++)
	{
		switch (sh_tbl[i].sh_type)
		{
		case SHT_SYMTAB:
		case SHT_DYNSYM:
			read_symbol_table(fd, eh, sh_tbl, i);
			break;
		}
	}
}

/* structured result are stored in symbol_tbl */
void parse_elf_file(int fd)
{
	Elf32_Ehdr eh;
	read_elf_header(fd, &eh);
	display_elf_hedaer(eh);

	Elf32_Shdr sh_tbl[eh.e_shentsize * eh.e_shnum];
	read_section_headers(fd, eh, sh_tbl);
	display_section_headers(fd, eh, sh_tbl);

	read_symbols(fd, eh, sh_tbl);
	elf_count++;
}

/* return the index in symbol_tbl or =1 if not found */
SymEntry *find_symbol_func(paddr_t target)
{
	int idx;
	for (idx = 0; idx < elf_count; idx++)
	{
		int i;
		for (i = 0; i < symbol_tbl_size[idx]; i++)
		{
			if (ELF32_ST_TYPE(symbol_tbl[idx][i].info) == STT_FUNC)
			{
				if (symbol_tbl[idx][i].addr <= target && target < symbol_tbl[idx][i].addr + symbol_tbl[idx][i].size)
				{
					return symbol_tbl[idx] + i;
				}
			}
		}
	}

	return NULL;
}