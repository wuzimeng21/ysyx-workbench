#include <memory.h>
#include <proc.h>

static void *pf = NULL;

void *new_page(size_t nr_page) {
    void *ret = pf;
    pf += nr_page * PGSIZE;
    return ret;
}

#ifdef HAS_VME
static void *pg_alloc(int n)
{
    assert(n % PGSIZE == 0);
    void *ret = new_page(n / PGSIZE);
    memset(ret, 0, n);
    return ret;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

// /* The brk() system call handler. */
// int mm_brk(uintptr_t brk) {
//   return 0;
// }

int mm_brk(uintptr_t brk)
{
#ifdef HAS_VME
    if (brk <= current->max_brk)
        return 0;
    for (uintptr_t va = current->max_brk,
                   vp = ROUNDDOWN(current->max_brk, PGSIZE);
         va < brk; vp += PGSIZE, va = vp)
    {
        if (va % PGSIZE == 0)
        {
            uintptr_t pp = (uintptr_t)new_page(1);
            map(&current->as, (void *)vp, (void *)pp, MMAP_READ | MMAP_WRITE);
        }
    }
    current->max_brk = brk;
    return 0;
#else
    if (brk > current->max_brk)
        current->max_brk = brk;
    return 0;
#endif
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
