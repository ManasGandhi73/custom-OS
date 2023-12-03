#include "mouse.h"
#include "filesys.h"

//Mouse.inc by SANiK
//License: Use as you wish, except to cause damage
// int mouse_x = 0; 
// int mouse_y = 0;
int mouse_cycle = 0; 
uint8_t mouse_byte[3]; 

//Mouse functions
void mouse_handler() //struct regs *a_r (not used but just there)
{
  if(startmouse)  // flag needed to sync up mouse_cycle with inb(0x60). Set active when print Welcome to OS
    switch(mouse_cycle)
    {
        case 0:
            mouse_byte[0]=inb(0x60);
            // printf_kb("%x, ", mouse_byte[0]); 
            mouse_cycle++;
            break;
        case 1:
            mouse_byte[1]=inb(0x60);
            // printf_kb("%x,", mouse_byte[1]); 
            mouse_cycle++;
            break;
        case 2:
            mouse_byte[2]=inb(0x60);
            // printf_kb(" %x     ", mouse_byte[2]); 
            mouse_cycle=0;


            map(VIDEO, VIDEO, _4kBPAGE); 
            undraw_cursor((double) mouse_x_overall * MOUSE_RL_SCALE, (double)(mouse_y_overall * -1) * MOUSE_UD_SCALE);
            if(cur_vidmap != -1)
                map(VIDEO, cur_vidmap, _4kBPAGE); 

            uint32_t d; 
            uint32_t state; 

            state = mouse_byte[0]; 
            d = mouse_byte[1];  
            uint32_t rel_x = d - ((state << 4) & 0x100); 
            d = mouse_byte[2]; 
            uint32_t rel_y = d - ((state << 3) & 0x100); 
            mouse_x_overall += rel_x; 
            mouse_y_overall += rel_y; 
            if(mouse_x_overall > (1/MOUSE_RL_SCALE)*80) {mouse_x_overall = (1/MOUSE_RL_SCALE)*80;}
            else if(mouse_x_overall < 0) {mouse_x_overall = 0;}

            // y is backwards for mouse (down goes up)
            if(mouse_y_overall < -24*(1/MOUSE_UD_SCALE)) {mouse_y_overall = -24*(1/MOUSE_UD_SCALE);}
            else if(mouse_y_overall > 0) {mouse_y_overall = 0;}
            if(!paint_press)
              draw_cursor((double) mouse_x_overall * MOUSE_RL_SCALE, (double)(mouse_y_overall * -1) * MOUSE_UD_SCALE);



            if(!gui_flag && (state & 0x01) == 1)
            {
              // terminal
              if((double) mouse_x_overall* MOUSE_RL_SCALE > 10 && (double) mouse_x_overall* MOUSE_RL_SCALE < 20 && (int)((double) (mouse_y_overall * -1)* MOUSE_UD_SCALE) == 5)
              {
                gui_flag = 1; 
                clear();
                putc('\r');
              }

              // paint
              else if((double) mouse_x_overall* MOUSE_RL_SCALE > 35 && (double) mouse_x_overall* MOUSE_RL_SCALE < 45 && (int)((double) (mouse_y_overall * -1)* MOUSE_UD_SCALE) == 5)
              {
                gui_flag = 3; 
                clear();
                putc('\r');
              }

              //pingpong
              else if((double) mouse_x_overall* MOUSE_RL_SCALE > 60 && (double) mouse_x_overall* MOUSE_RL_SCALE < 75 && (int)((double) (mouse_y_overall * -1)* MOUSE_UD_SCALE) == 5)
              {
                gui_flag = 2; 
                clear();
                putc('\r');
              }


              // fish
              else if((double) mouse_x_overall* MOUSE_RL_SCALE > 10 && (double) mouse_x_overall* MOUSE_RL_SCALE < 20 && (int)((double) (mouse_y_overall * -1)* MOUSE_UD_SCALE) == 10)
              {
                gui_flag = 4; 
                clear();
                putc('\r');
              }

              // hello
              else if((double) mouse_x_overall* MOUSE_RL_SCALE > 35 && (double) mouse_x_overall* MOUSE_RL_SCALE < 45 && (int)((double) (mouse_y_overall * -1)* MOUSE_UD_SCALE) == 10)
              {
                gui_flag = 5; 
                clear();
                putc('\r');
              }

              //counter
              else if((double) mouse_x_overall* MOUSE_RL_SCALE > 60 && (double) mouse_x_overall* MOUSE_RL_SCALE < 75 && (int)((double) (mouse_y_overall * -1)* MOUSE_UD_SCALE) == 10)
              {
                gui_flag = 6; 
                clear();
                putc('\r');
              }

              //ls
              else if((double) mouse_x_overall* MOUSE_RL_SCALE > 10 && (double) mouse_x_overall* MOUSE_RL_SCALE < 15 && (int)((double) (mouse_y_overall * -1)* MOUSE_UD_SCALE) == 15)
              {
                gui_flag = 7; 
                clear();
                putc('\r');
              }

              //cat
              else if((double) mouse_x_overall* MOUSE_RL_SCALE > 35 && (double) mouse_x_overall* MOUSE_RL_SCALE < 40 && (int)((double) (mouse_y_overall * -1)* MOUSE_UD_SCALE) == 15)
              {
                cat_flag = 1;
                gui_flag = 8; 
                clear();
                putc('\r');
              }



            }

            // cat frame0.txt
            if(cat_flag && (state & 0x01) == 1) {
              if((double) mouse_x_overall* MOUSE_RL_SCALE > 10 && (double) mouse_x_overall* MOUSE_RL_SCALE < 20 && (int)((double) (mouse_y_overall * -1)* MOUSE_UD_SCALE) == 5)
              {
                cat_flag = 2; 
                clear();
                putc('\r');
          
              }

              // cat frame1.txt
              else if((double) mouse_x_overall* MOUSE_RL_SCALE > 35 && (double) mouse_x_overall* MOUSE_RL_SCALE < 45 && (int)((double) (mouse_y_overall * -1)* MOUSE_UD_SCALE) == 5)
              {
                cat_flag = 3; 
                clear();
                putc('\r');
           
              }

              // cat verylong
              else if((double) mouse_x_overall* MOUSE_RL_SCALE > 60 && (double) mouse_x_overall* MOUSE_RL_SCALE < 75 && (int)((double) (mouse_y_overall * -1)* MOUSE_UD_SCALE) == 5)
              {
                cat_flag = 4; 
                clear();
                putc('\r');
              }
            }

            if(paint_flag && (state & 0x01) == 1) {
              draw_cursor((double) mouse_x_overall * MOUSE_RL_SCALE, (double)(mouse_y_overall * -1) * MOUSE_UD_SCALE);
              paint_press = 1;
            }
            else if(paint_flag) {
              paint_press = 0;
            }
            if(paint_flag && (state & 0x02) == 0x02) {
              paint_color = ((paint_color + 0x01) % 0xF); 
              if(paint_color == 0x0) {
                paint_color = 0x01;
              }
              status_bar_paint();
            }

            break;
    }


      // right now I make what the cursor hovering over white. 





    // // unsigned int x = inb(0x60); 
    // // unsigned int y = inb(0x60); 
    // // unsigned int z = inb(0x60); 
    // // uint8_t button_info = (uint8_t) (0x0FF & x); 
    // // unsigned int x_info = (uint8_t) ((0xFF00 & x) >> 8; 
    // // unsigned int y_info = (uint8_t) ((0xFF0000 & x) >> 8); 
    
  //   unsigned int d; 
  //   unsigned int state; 

  //   state = x; 
	// d = y;  
	// int rel_x = d - ((state << 4) & 0x100); 
	// d = z; 
	// int rel_y = d - ((state << 3) & 0x100); 



    // printf_kb("%d %d %d     ", state, rel_x, rel_y); 
    // mouse_x += rel_x; 
    // mouse_y += rel_y; 


    // printf_kb("x change: %d ", rel_x); 
    // printf_kb("y change: %d \n", rel_y); 
    send_eoi(0x0C); 
}

inline void mouse_wait(char a_type) //unsigned char
{
  unsigned int _time_out = 100000; //unsigned int
  if(a_type==0)
  {
    while(_time_out--) //Data
    {
      if((inb(0x64) & 1)==1)
      {
        return;
      }
    }
    return;
  }
  else
  {
    while(_time_out--) //Signal
    {
      if((inb(0x64) & 2)==0)
      {
        return;
      }
    }
    return;
  }
}

inline void mouse_write_help(unsigned char a_write) //unsigned char
{
  //Wait to be able to send a command
  mouse_wait(1);
  //Tell the mouse we are sending a command
  outb(0xD4, 0x64);
  //Wait for the final part
  mouse_wait(1);
  //Finally write
  outb(a_write, 0x60);
}

unsigned char mouse_read_help()
{
  //Get's response from mouse
  mouse_wait(0);
  return inb(0x60);
}

void mouse_init()
{
  unsigned char _status;  //unsigned char

  //Enable the auxiliary mouse device
  mouse_wait(1);
  outb(0xA8, 0x64);
  //Enable the interrupts
  mouse_wait(1);
  outb(0x20, 0x64);
  mouse_wait(0);
  _status=(inb(0x60) | 2);
  mouse_wait(1);
  outb(0x60, 0x64);
  mouse_wait(1);
  outb(_status, 0x60);
 
  //Tell the mouse to use default settings
  mouse_write_help(0xF6);
  mouse_read_help();  //Acknowledge
 
  //Enable the mouse
  mouse_write_help(0xF4);
  mouse_read_help();  //Acknowledge

  mouse_x_overall = 40; 
  mouse_y_overall = -12; 

  //Setup the mouse handler
  enable_irq(0x0C); 
}

