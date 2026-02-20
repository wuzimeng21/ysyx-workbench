
void read_dtrace(IOMap *map, paddr_t addr) {
    if (map == NULL) {
        Assert(map != NULL, "[READ] address (" FMT_PADDR ") is out of bound at pc = " FMT_WORD, addr, cpu.pc);
    } else {
        Assert(addr <= map->high && addr >= map->low,
            "[READ] address (" FMT_PADDR ") is out of bound {%s} [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
            addr, map->name, map->low, map->high, cpu.pc);
    }
    Log("[READ] address (" FMT_PADDR ") | device name: {%s} [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
    addr, ANSI_FMT(map->name, ANSI_FG_RED), map->low, map->high, cpu.pc);

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
    Log("[WRITE] address (" FMT_PADDR ") | device name: {%s} [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
    addr, ANSI_FMT(map->name, ANSI_FG_RED), map->low, map->high, cpu.pc);

    return ;
   
}

