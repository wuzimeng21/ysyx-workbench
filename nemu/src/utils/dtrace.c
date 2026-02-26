#include <device/map.h>

void read_dtrace(IOMap *map, paddr_t addr) {
    if (map == NULL) {
        Assert(map != NULL, "[READ] address (" FMT_PADDR ") is out of bound at pc = " FMT_WORD, addr, cpu.pc);
    } else {
        Assert(addr <= map->high && addr >= map->low,
            "[READ] address (" FMT_PADDR ") is out of bound {%s} [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
            addr, map->name, map->low, map->high, cpu.pc);
    }
    Log("[READ] address (" FMT_PADDR ") | device name: " ANSI_FMT("{%s}", ANSI_FG_RED) " [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
    addr, map->name, map->low, map->high, cpu.pc);    return ;

    return ;
}

void write_dtrace(IOMap *map, paddr_t addr) {
    if (map == NULL) {
        Assert(map != NULL, "[WRITE] address (" FMT_PADDR ") is out of bound at pc = " FMT_WORD, addr, cpu.pc);
    } else {
        Assert(addr <= map->high && addr >= map->low,
            "[WRITE] address (" FMT_PADDR ") is out of bound {%s} [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
            addr, map->name, map->low, map->high, cpu.pc);
    }
    Log("[WRITE] address (" FMT_PADDR ") | device name: " ANSI_FMT("{%s}", ANSI_FG_RED) " [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
    addr, map->name, map->low, map->high, cpu.pc);    return ;
   
}

