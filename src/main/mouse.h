#include "multiboot.h"

#include "types.h"
#include "x86_desc.h"
#include "devices.h"


int mouse_x_overall;
int mouse_y_overall;
//Mouse functions
void mouse_handler(); //struct regs *a_r (not used but just there)

void mouse_wait(char a_type); //unsigned char

void mouse_write_help(unsigned char a_write); //unsigned char

void mouse_init();
