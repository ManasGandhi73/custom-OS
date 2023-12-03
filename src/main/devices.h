
#include "types.h"
#include "lib.h"
#include "i8259.h"
// Enables KB
extern void KB_init();
// handles keyboard interrups
extern void KB_handler();

/*enables terminal functions */
int32_t terminal_open(const uint8_t* filename); 
/*Disables terminal*/
int32_t terminal_close(int32_t fd);
/*Read keyboard input to enter*/
int32_t terminal_read(int32_t fd, void * buf, int32_t n);
/*Write to terminal from buffer*/
int32_t terminal_write(int32_t fd, void * buf, int32_t n);
/*Initialize current KB_buffer */
extern void init_KB_Buffer(); 
/*Auto fill if tab is pressed*/
void auto_fill_func(char * needed_auto_fill, int space_length);
/*determines the length of whitespace before the current position of the cursor in kb_buffer_array*/
int check_space();

