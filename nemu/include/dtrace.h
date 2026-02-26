#include <device/map.h>

// ----------- dtrace -----------

void read_dtrace(IOMap *map, paddr_t addr);
void write_dtrace(IOMap *map, paddr_t addr);
