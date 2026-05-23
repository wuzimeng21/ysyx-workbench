#include <device/map.h>

void read_dtrace(IOMap *map, paddr_t addr) {
    if (map == NULL) {
        Assert(map != NULL, "[READ] address (" FMT_PADDR ") is out of bound ", addr);
    } else {
        Assert(addr <= map->high && addr >= map->low,
            "[READ] address (" FMT_PADDR ") is out of bound {%s} [" FMT_PADDR ", " FMT_PADDR "] " ,
            addr, map->name, map->low, map->high);
    }
    Log("[READ] address (" FMT_PADDR ") | device name: %s [" FMT_PADDR ", " FMT_PADDR "]" ,
    addr, map->name, map->low, map->high); 

    return ;
}

void write_dtrace(IOMap *map, paddr_t addr) {
    if (map == NULL) {
        Assert(map != NULL, "[WRITE] address (" FMT_PADDR ") is out of bound", addr);
    } else {
        Assert(addr <= map->high && addr >= map->low,
            "[WRITE] address (" FMT_PADDR ") is out of bound {%s} [" FMT_PADDR ", " FMT_PADDR "]" ,
            addr, map->name, map->low, map->high);
    }
    Log("[WRITE] address (" FMT_PADDR ") | device name: %s [" FMT_PADDR ", " FMT_PADDR "]" ,
    addr, map->name, map->low, map->high);    
   
    return ;
   
}

