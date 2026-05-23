#include <common.h>

static Context* do_event(Event e, Context* c) {
  switch (e.event) {
    // add for PA3.2
    case EVENT_YIELD:
      c = schedule(c); 
      Log("HAS_CTE->init_irq->do_event: EVENT_YIELD...");
      break;
    case EVENT_SYSCALL:
      Log("HAS_CTE->init_irq->do_event: EVENT_SYSCALL...");
      do_syscall(c);
      break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  // Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
