#include "types.h"
#include "lib.h"
#include "i8259.h"
#include "syscall.h"

/*handle pit interrupts*/
extern void pit_handler();
/*intialize pit on pic*/
extern void pit_init(); 
/*impliment round-robin scheduling*/
extern void schedule_setup();

