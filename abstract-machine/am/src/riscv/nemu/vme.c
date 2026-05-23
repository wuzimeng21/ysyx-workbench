#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

void map(AddrSpace *as, void *va, void *pa, int prot)
{
    assert(as->ptr);
    assert((uintptr_t)va % PGSIZE == 0);
    assert((uintptr_t)pa % PGSIZE == 0);
    uintptr_t pt1 = (uintptr_t)as->ptr;
    uintptr_t *pte1 = (uintptr_t *)(pt1 + ((uintptr_t)va >> 22) * 4);
    uintptr_t pt2 = *pte1 >> 10 << 12;
    if ((*pte1 & 1) == 0)
    {
        pt2 = (uintptr_t)pgalloc_usr(PGSIZE);
        *pte1 = 0;
        *pte1 |= 1;
        *pte1 |= pt2 >> 12 << 10;
    }
    uintptr_t *pte2 = (uintptr_t *)(pt2 + ((uintptr_t)va << 10 >> 22) * 4);
    assert((*pte2 & 1) == 0);
    *pte2 = 0;
    *pte2 |= 1;
    *pte2 |= (uintptr_t)pa >> 12 << 10;
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context * c = (Context *)(kstack.end - sizeof(Context));
  c->mepc = (uintptr_t)entry;
  c->mstatus = 0x1800;
  // set sp pointer
  c->gpr[2] = (uintptr_t)kstack.end;
  // keep arg
  // c->gpr[10] = (uintptr_t)arg;
  return c;
}
