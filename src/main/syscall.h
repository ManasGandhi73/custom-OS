#include "types.h"
#include "x86_desc.h"
#include "lib.h"
#include "filesys.h"
#include "rtc.h"
#include "devices.h"


/*provides access to the file system*/
extern int32_t sys_open(const uint8_t* filename);

/*closes the specified file descriptor and makes it available for return from later calls to open*/
extern int32_t sys_close(int32_t fd);

/*closes the specified file descriptor and makes it available for return from later calls to open*/
extern int32_t sys_read(int32_t fd, void* buf, int32_t nbytes);

/*writes data to the terminal or to a device (RTC)*/
extern int32_t sys_write(int32_t fd, const void * buf, int32_t nbytes);

/*executes system call*/
extern int32_t sys_execute(const uint8_t* command);

/*system call terminates a process, returning the specified value to its parent process*/
extern int32_t sys_halt(uint8_t status);

/*does the execute system call, this is where the inline assembly jumps -- executes a given system call*/
extern uint32_t sys_execute_setup(uint8_t * cmd, uint32_t esp_needed, uint32_t ebp_needed);

/*gets argument*/
extern int32_t sys_getargs(uint8_t* buf, int32_t nbytes);

/*sets vidmap*/
extern int32_t sys_vidmap(uint8_t** screen_start);

int32_t sys_halt_pcb(uint8_t pcb); 



