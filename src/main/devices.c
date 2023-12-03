#include "devices.h"
#include "syscall.h"

// global scancode_buffer used to find what user inputted to keyboard
// NOT_PRINTABLE-> all 1s is error meaning nothing to print if typed (I just chose this).
// https://wiki.osdev.org/Keyboard
// size 0x3A for needed buffer upto space




// global variable test for rtc
int test = 0; 


/*Defined Scan Code values for keyboard handler*/
#define SIZE_SCANCODE_BUF   0x3A
#define NOT_PRINTABLE       0x7F
#define CAPS_CODE           0x3A

#define CTRL_CODE           0x1D
#define UCTRL_CODE          0x9D
#define L_CODE              0x26
#define ALT_CODE            0x38
#define UALT_CODE           0xB8

#define LSHIFT_CODE         0x2A
#define RSHIFT_CODE         0x36
#define ULSHIFT_CODE        0xAA
#define URSHIFT_CODE        0xB6
#define BACKSPACE_CODE      0x0E
#define ENTER_CODE          0x1C
#define ESCAPE_CODE         0x01 
#define TAB_CODE            0x0F
#define ESC_CODE            0x01

#define UP_CODE             0x48

// size of keyboard buffer for checkpoint 2 is 128
// tab does 4 spaces
#define NUM_TAB_SPACES      4
#define NUM_EXE             10
#define F1  0x3b
#define F2  0x3c
#define F3  0x3d

#define SIZE_KB_BUFFER      0x80
#define SIZE_NAME_BUF       15




/*scancode buffer for codes found here:  https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html*/

char scancodes_buffer[SIZE_SCANCODE_BUF][2] = 
{
    {   NOT_PRINTABLE,NOT_PRINTABLE   } ,
    {   NOT_PRINTABLE,NOT_PRINTABLE   } ,
    { '1','!' } , { '2','@' } , { '3','#' } , { '4','$' } , { '5','%' } , 
    { '6','^' } , { '7','&' } , { '8','*' } , { '9','(' } , { '0',')' } , 
    { '-','_' } , { '=','+' } , 
    {   NOT_PRINTABLE,NOT_PRINTABLE   } ,
    {   NOT_PRINTABLE,NOT_PRINTABLE   } , 
    { 'q','Q' } , { 'w','W' } , { 'e','E' } , { 'r','R' } , { 't','T' } , 
    { 'y','Y' } , { 'u','U' } , { 'i','I' } , { 'o','O' } , { 'p','P' } , 
    { '[','{' } , { ']','}' } , { NOT_PRINTABLE, NOT_PRINTABLE } ,
    {   NOT_PRINTABLE,NOT_PRINTABLE   } ,
    { 'a','A' } , { 's','S' } , { 'd','D' } , { 'f','F' } , { 'g','G' } ,
    { 'h','H' } , { 'j','J' } , { 'k','K' } , { 'l','L' } , { ';',':' } ,
    {  39, 34 } , { '`','~' } , /*39 and 34 to represent ' and " in char form*/
    {   NOT_PRINTABLE,NOT_PRINTABLE   } ,        
    { '\\','|'} , { 'z','Z' } , { 'x','X' } , { 'c','C' } , { 'v','V' } ,
    { 'b','B' } , { 'n','N' } , { 'm','M' } , { ',','<' } , { '.','>' } ,
    { '/','?' } ,
    {   NOT_PRINTABLE,NOT_PRINTABLE   } ,        
    {   NOT_PRINTABLE,NOT_PRINTABLE   } ,        
    {   NOT_PRINTABLE,NOT_PRINTABLE   } ,
    { ' ',' ' }   
};


typedef struct auto_fill {
    char * name_buf[SIZE_NAME_BUF];
    char * frame0;
    char * frame1;


} auto_fill_t;


auto_fill_t auto_fill_buffer;


// 3 = 3 terminals

// kb buffer to store keys during a terminal read
char kb_buffer_array[3][SIZE_KB_BUFFER];

// 2D array kb)buffer, 1st index being current terminal, 2nd cur_key_index
char kb_buffer_array_prev[3][SIZE_KB_BUFFER];


// current index of next char to be pressed
int cur_key_index[3];

// for up arrow
int cur_key_index_prev[3]; 

// flag to wake a terminal read (depending on which terminal has finished reading)
int terminal_wake[3];

// global variable used for caps lock
int caps = 0;

// global variable used for ctrl
int ctrl = 0;

// scancode input from keyboard for each terminal
unsigned char kb_input;


// see if enter has been pressed 
int enter_flag[3];

// global variable to tell if termial is opened
int terminal_opened = 0;

// global var used for alt 
int alt = 0; 

// sys call names
char cat_f[SIZE_KB_BUFFER] = "cat";
char counter_f[SIZE_KB_BUFFER] = "counter";
char grep_f[SIZE_KB_BUFFER] = "grep";
char hello_f[SIZE_KB_BUFFER] = "hello";
char ls_f[SIZE_KB_BUFFER] = "ls";
char pingpong_f[SIZE_KB_BUFFER] = "pingpong";
char shell_f[SIZE_KB_BUFFER] = "shell";
char sigtest_f[SIZE_KB_BUFFER] = "sigtest";
char syserr_f[SIZE_KB_BUFFER] = "syserr";
char testprint_f[SIZE_KB_BUFFER] = "testprint";
char exit_f[SIZE_KB_BUFFER] = "exit";
char clear_f[SIZE_KB_BUFFER] = "clear";
char verylarge_f[SIZE_KB_BUFFER] = "verylargetextwithverylongname.tx";
char fish_f[SIZE_KB_BUFFER] = "fish";
char created_f[SIZE_KB_BUFFER] = "created.txt";
char frame0_f[SIZE_KB_BUFFER] = "frame0.txt";
char frame1_f[SIZE_KB_BUFFER] = "frame1.txt";






/* init_KB_buffer
 * 
 * Initializes current terminal keyboard buffer
 * Inputs: None
 * Outputs: None
 * Side Effects: clearkb_buffer_array[currentTerminal]
 */
void init_KB_Buffer() {


    int i;
    // clear out buffer upon initialization
    for(i = 0; i < SIZE_KB_BUFFER; i++) {
       kb_buffer_array[currentTerminal][i] = NULL;
    }
    enter_flag[currentTerminal] = 0;  // reset current terminal enter flag (to allow for keyboard input)
    cur_key_index[currentTerminal] = 0; // reset current index of next char to be pressed


}




/* KB_init
 * 
 * initializes KB, allows interrupts, initializes cursor
 * Inputs: None
 * Outputs: None
 * Side Effects: Allows for interrupts from Keyboard
 */
void KB_init() {
    // for auto fill on tab
    auto_fill_buffer.name_buf[0] = cat_f;
    auto_fill_buffer.name_buf[1] = counter_f;
    auto_fill_buffer.name_buf[2] = grep_f;
    auto_fill_buffer.name_buf[3] = hello_f;
    auto_fill_buffer.name_buf[4] = ls_f;
    auto_fill_buffer.name_buf[5] = pingpong_f;
    auto_fill_buffer.name_buf[6] = shell_f;
    auto_fill_buffer.name_buf[7] = sigtest_f;
    auto_fill_buffer.name_buf[8] = syserr_f;
    auto_fill_buffer.name_buf[9] = testprint_f;
    auto_fill_buffer.name_buf[10] = exit_f;
    auto_fill_buffer.name_buf[11] = clear_f;
    auto_fill_buffer.name_buf[12] = verylarge_f;
    auto_fill_buffer.name_buf[13] = fish_f;
    auto_fill_buffer.name_buf[14] = created_f;
    auto_fill_buffer.frame0 = frame0_f;
    auto_fill_buffer.frame1 = frame1_f;


    init_KB_Buffer();

    wake = 1;   // initialize terminals to be able to be switched to (1).
    int i;
    for(i = 0; i < 3; i++) {
        terminal_wake[i] = 0;     // initialize no terminals have finished reading
    }
    cur_key_index[currentTerminal] = 0;
    enable_cursor(14, 15); // 14 and 15 for lowest scan lines in blinker
    enable_irq(0x01); // enables keybaord interrupts (from IDT table, kb = 0x21 -> P_PIC IRQ1)
}


/* terminal_open
 * 
 * Enables Terminal Functions
 * Inputs: 
 *          filename -- file name
 *      
 * Outputs: None
 * Side Effects: none
 */
int32_t terminal_open(const uint8_t* filename) {
    terminal_opened = 1;
    init_KB_Buffer();
    //printf("Terminal functions have been enabled!\n");
    return 0;
}



/* terminal_close
 * 
 * Disables Terminal Functions
 * INPUTS: 
 *           fd -- file descriptor table index
 * Outputs: None
 * Side Effects: none
 */
int32_t terminal_close(int32_t fd) {
    terminal_opened = 0;
    //printf("Terminal functions have been disabled!\n");
    return 0;
}


/* terminal_read
 * 
 * Reads keyboard input 
 *   INPUTS: 
 *          fd -- file descriptor table index
 *          buf -- buffer to put enter chars into
 *          n -- nothing for terminal read
 * Outputs: None
 * Side Effects: Saveskb_buffer_array[currentTerminal] to given buf
 */
int32_t terminal_read(int32_t fd, void * buf, int32_t n) {
    if(terminal_opened) {   // if terminal init has been called
        cli();


        // changed to reset because was getting weird errors doing it the other way
        if(gui_flag == 0) {
            tss.esp0 = ESP0 - 0x04;
            // clear_all(); 
            unmap(_128MB, _4MBPAGE);
            unmap(_128MB+ EIGHTMB, _4kBPAGE);
            // unmap(VIDEOT1, _4kBPAGE);
            // unmap(VIDEOT2, _4kBPAGE); 
            // unmap(VIDEOT3, _4kBPAGE); 

            // map((uint32_t) VIDEOT1, (uint32_t) VIDEOT1, _4kBPAGE); //map the 4kb pages to be used for video memory (one to one)
            // map((uint32_t) VIDEOT2, (uint32_t) VIDEOT2, _4kBPAGE); 
            // map((uint32_t) VIDEOT3, (uint32_t) VIDEOT3, _4kBPAGE); 

            // int i; 
            // for(int i = 0; i < 6; i++)
            // {
            //     if(pcb_array[i]->parentID != -1)
            //         init_pcb(pcb_array[i]); 
            // }

            init_pcb(pcb_array[0]);  //reset all of the pcb's 
            init_pcb(pcb_array[1]); 
            init_pcb(pcb_array[2]); 
            init_pcb(pcb_array[3]); 
            init_pcb(pcb_array[4]); 
            init_pcb(pcb_array[5]); 

            /* set all of the global variables */
            pcbIndex = 0; 
            dir_read_idx = 0;
            currentTerminal = 0;
            paint_flag = 0;
            paint_color = 0;
            finish_paint = 1;
            gui_flag = 0; 
            welcome = 0;
            cur_vidmap = -1;

            // set global flag variables
            terminal1_switch = 0;
            terminal2_switch = 0;
            terminal3_switch = 0;
            // set second and third terminal flags to be uninitialized
            secondTerm = 0; 
            thirdTerm = 0; 

            clear_all();
            currentTerminal = 0;
            init_KB_Buffer();
            ATTRIB[currentTerminal] = T1_COLOR;
            clear();
            putc_kb('\r'); 
            currentTerminal = 1;
            init_KB_Buffer();
            ATTRIB[currentTerminal] = T2_COLOR;
            clear();
            putc_kb('\r'); 
            currentTerminal = 2;
            init_KB_Buffer();
            ATTRIB[currentTerminal] = T3_COLOR;
            clear();
            putc_kb('\r'); 
            currentTerminal = 0;

            send_eoi(0x01);
            sys_execute_setup((uint8_t *)"shell", NULL, NULL); // call the execute (set esp and edp to NULL since not going to be used)
        }
        //pingpong
        if(gui_flag == 2) {
            clear();
            putc('\r');
            ((char *)buf)[0] = 'p';
            ((char *)buf)[1] = 'i';
            ((char *)buf)[2] = 'n';
            ((char *)buf)[3] = 'g';
            ((char *)buf)[4] = 'p';
            ((char *)buf)[5] = 'o';
            ((char *)buf)[6] = 'n';
            ((char *)buf)[7] = 'g';
            ((char *)buf)[8] = '\n';
            gui_flag = -2; 
            return 8;
        }
        //paint
        if(gui_flag == 3) {
            clear();
            putc('\r');
            ((char *)buf)[0] = 'p';
            ((char *)buf)[1] = 'a';
            ((char *)buf)[2] = 'i';
            ((char *)buf)[3] = 'n';
            ((char *)buf)[4] = 't';
            ((char *)buf)[5] = '\n';
            gui_flag = -2; 
            return 5;
        }
        //fish
        if(gui_flag == 4) {
            clear();
            putc('\r');
            ((char *)buf)[0] = 'f';
            ((char *)buf)[1] = 'i';
            ((char *)buf)[2] = 's';
            ((char *)buf)[3] = 'h';
            ((char *)buf)[4] = '\n';
            gui_flag = -2;
            return 4;
        }
        //hello
        if(gui_flag == 5) {
            clear();
            putc('\r');
            ((char *)buf)[0] = 'h';
            ((char *)buf)[1] = 'e';
            ((char *)buf)[2] = 'l';
            ((char *)buf)[3] = 'l';
            ((char *)buf)[4] = 'o';
            ((char *)buf)[5] = '\n';
            gui_flag = -1;
         
            return 5;
        }
        //counter
        if(gui_flag == 6) {
            clear();
            putc('\r');
            ((char *)buf)[0] = 'c';
            ((char *)buf)[1] = 'o';
            ((char *)buf)[2] = 'u';
            ((char *)buf)[3] = 'n';
            ((char *)buf)[4] = 't';
            ((char *)buf)[5] = 'e';
            ((char *)buf)[6] = 'r';
            ((char *)buf)[7] = '\n';
            gui_flag = -1;
         
            return 7;
        }
        //ls
        if(gui_flag == 7) {
            clear();
            putc('\r');
            ((char *)buf)[0] = 'l';
            ((char *)buf)[1] = 's';
            ((char *)buf)[2] = '\n';
            gui_flag = -2;
            return 3;
        }

        if(gui_flag == 8) {
            clear();
            putc('\r');
            ((char *)buf)[0] = 'c';
            ((char *)buf)[1] = 'a';
            ((char *)buf)[2] = 't';
            ((char *)buf)[3] = '\n';
            gui_flag = -2;
            return 4;
        }
        int i;
        int j = 0;
        pcb_array[pcbIndex]->is_waiting = 1;    // set current pcb waitng so only switch when on the waiting terminal
 
        sti();
        do {
            if(terminal_wake[currentTerminal]) {  // if terminal_wake when enter was pressed is the current terminal being displayed
                if(gui_flag == -1)
                    gui_flag = -2;


                cli();  // clear interrupts to not change kb buffer



                if(cur_key_index[currentTerminal] > n) {    // if input is < n
                    cur_key_index[currentTerminal] = n;
                }

                // loop through kb_buffer of current termianl until cur_key_index and store into inputted buffer
                for(i = 0; i < cur_key_index[currentTerminal]; i++) {
                    if(kb_buffer_array[currentTerminal][i] != '\t' && kb_buffer_array[currentTerminal][i] != '\v'){
                        ((char *)buf)[j] = kb_buffer_array[currentTerminal][i]; 
                        j++;
                    }

                }
                terminal_wake[currentTerminal] = 0;

                break;  // break from infinite while loop
            }
        } while(1);
        
        pcb_array[pcbIndex]->is_waiting = 0;    // set current pcb to no longer be waiting
        enter_flag[currentTerminal] = 0;        // set enter flag to 0 to allow for keyboard inputs on that terminal again
        init_KB_Buffer();                       // initialize current kb_buffer
        sti();



        return j;   // return number of bits read
    }

    return -1;
}


/* terminal_write
 * 
 * Reads keyboard input 
 *   INPUTS: 
 *          fd -- file descriptor table index
 *          buf -- buffer to put enter chars into
 *          n -- number of bytes to write
 * Outputs: prints inputted buffer onto screen
 * Side Effects: none
 */
int32_t terminal_write(int32_t fd, void * buf, int32_t n) {
    if(terminal_opened) {
        cli();  // clear interrupts so 2 putc's can't happen during a terminal write
        int i;
        for(i = 0; i < n; i++) {
            if( ((uint8_t *)buf)[i] != NULL)
                putc(((uint8_t *)buf)[i]);      // calls putc which uses cur_pcb->terminal number to write to video memory
        }
        sti();

        return n;   // return number of bits that were written
    }

    return -1;
}


/* KB_handler
 * 
 * handles keyboard interrupts 
 * Inputs: None
 * Outputs: None
 * Side Effects: Prints key that is pressed, caps lock works, enter key clears screen
 *                  allows for tab and backspace
 */

void KB_handler() {
    // send EOI to IRQ1 of PIC to allow higher priority to work
      // PS/2 Controller IO Ports from https://wiki.osdev.org/%228042%22_PS/2_Controller#Translation 
    /* IO Port	Access Type	    Purpose
        0x60	Read/Write	    Data Port
        0x64	Read	        Status Register
        0x64	Write	        Command Register
    */
    cli();
    kb_input = inb(0x60); 


    if(paint_flag) {
        if(kb_input == ESC_CODE) {
            finish_paint = 1;
        }
        send_eoi(0x01);
        return;
    }
    if(gui_flag == -2) {
        if(kb_input == ESC_CODE) {
            tss.esp0 = ESP0 - 0x04;
            // clear_all(); 
            unmap(_128MB, _4MBPAGE);
            unmap(_128MB+ EIGHTMB, _4kBPAGE);
            // unmap(VIDEOT1, _4kBPAGE);
            // unmap(VIDEOT2, _4kBPAGE); 
            // unmap(VIDEOT3, _4kBPAGE); 

            // map((uint32_t) VIDEOT1, (uint32_t) VIDEOT1, _4kBPAGE); //map the 4kb pages to be used for video memory (one to one)
            // map((uint32_t) VIDEOT2, (uint32_t) VIDEOT2, _4kBPAGE); 
            // map((uint32_t) VIDEOT3, (uint32_t) VIDEOT3, _4kBPAGE); 

            // int i; 
            // for(int i = 0; i < 6; i++)
            // {
            //     if(pcb_array[i]->parentID != -1)
            //         init_pcb(pcb_array[i]); 
            // }

            init_pcb(pcb_array[0]);  //reset all of the pcb's 
            init_pcb(pcb_array[1]); 
            init_pcb(pcb_array[2]); 
            init_pcb(pcb_array[3]); 
            init_pcb(pcb_array[4]); 
            init_pcb(pcb_array[5]); 

            /* set all of the global variables */
            pcbIndex = 0; 
            dir_read_idx = 0;
            currentTerminal = 0;  
            startmouse = 0;
            paint_flag = 0;
            paint_color = 0;
            finish_paint = 1;
            gui_flag = 0; 
            welcome = 0;
            cur_vidmap = -1;

            // set global flag variables
            terminal1_switch = 0;
            terminal2_switch = 0;
            terminal3_switch = 0;
            // set second and third terminal flags to be uninitialized
            secondTerm = 0; 
            thirdTerm = 0; 

            clear_all();
            currentTerminal = 0;
            init_KB_Buffer();
            ATTRIB[currentTerminal] = T1_COLOR;
            clear();
            putc_kb('\r'); 
            currentTerminal = 1;
            init_KB_Buffer();
            ATTRIB[currentTerminal] = T2_COLOR;
            clear();
            putc_kb('\r'); 
            currentTerminal = 2;
            init_KB_Buffer();
            ATTRIB[currentTerminal] = T3_COLOR;
            clear();
            putc_kb('\r'); 
            currentTerminal = 0;

            send_eoi(0x01);
            sys_execute_setup((uint8_t *)"shell", NULL, NULL); // call the execute (set esp and edp to NULL since not going to be used)
        }
        send_eoi(0x01);
        return; 
    }

    if(gui_flag == 1)
    {
        if(kb_input == ESC_CODE)
        {
            tss.esp0 = ESP0 - 0x04;
            // clear_all(); 
            unmap(_128MB, _4MBPAGE);
            unmap(_128MB+ EIGHTMB, _4kBPAGE);
            // unmap(VIDEOT1, _4kBPAGE);
            // unmap(VIDEOT2, _4kBPAGE); 
            // unmap(VIDEOT3, _4kBPAGE); 

            // map((uint32_t) VIDEOT1, (uint32_t) VIDEOT1, _4kBPAGE); //map the 4kb pages to be used for video memory (one to one)
            // map((uint32_t) VIDEOT2, (uint32_t) VIDEOT2, _4kBPAGE); 
            // map((uint32_t) VIDEOT3, (uint32_t) VIDEOT3, _4kBPAGE); 

            // int i; 
            // for(int i = 0; i < 6; i++)
            // {
            //     if(pcb_array[i]->parentID != -1)
            //         init_pcb(pcb_array[i]); 
            // }

            init_pcb(pcb_array[0]);  //reset all of the pcb's 
            init_pcb(pcb_array[1]); 
            init_pcb(pcb_array[2]); 
            init_pcb(pcb_array[3]); 
            init_pcb(pcb_array[4]); 
            init_pcb(pcb_array[5]); 

            /* set all of the global variables */
            pcbIndex = 0; 
            dir_read_idx = 0;
            currentTerminal = 0;  
            startmouse = 0;
            paint_flag = 0;
            paint_color = 0;
            finish_paint = 1;
            gui_flag = 0; 
            welcome = 0;
            cur_vidmap = -1;

            // set global flag variables
            terminal1_switch = 0;
            terminal2_switch = 0;
            terminal3_switch = 0;
            // set second and third terminal flags to be uninitialized
            secondTerm = 0; 
            thirdTerm = 0; 

            clear_all();
            currentTerminal = 0;
            init_KB_Buffer();
            ATTRIB[currentTerminal] = T1_COLOR;
            clear();
            putc_kb('\r'); 
            currentTerminal = 1;
            init_KB_Buffer();
            ATTRIB[currentTerminal] = T2_COLOR;
            clear();
            putc_kb('\r'); 
            currentTerminal = 2;
            init_KB_Buffer();
            ATTRIB[currentTerminal] = T3_COLOR;
            clear();
            putc_kb('\r'); 
            currentTerminal = 0;

            send_eoi(0x01);
            sys_execute_setup((uint8_t *)"shell", NULL, NULL); // call the execute (set esp and edp to NULL since not going to be used)
        }
    }

    //this is to make sure we can type (send_eoi)
    //gui_flag = 1 --> terminal
    //gui_flag = -1 --> hello, counter
    if(gui_flag != 1 && gui_flag != -1) {
        send_eoi(0x01);
        return;
    }

    // check that there is a terminal waiting(process is in terminal read) and that waiting is on the current terminal
    int flag_reading = 0;
    int i;
    for(i = 0; i < PCBNUM; i++) {
        if(pcb_array[i]->is_waiting && pcb_array[i]->terminalNum == currentTerminal) {
            flag_reading = 1;
            break;
        }
    }

    
    if(kb_input == ALT_CODE)
        alt = 1; 

    else if(kb_input == UALT_CODE)
        alt = 0; 

    // set wait flags and terminal wanting to be switched to during next schedule (if a termianl switch hasn't already been done in same time slice)
    else if(kb_input == F1 && alt == 1 && currentTerminal != 0 && wake && gui_flag == 1)
    {
        terminal1_switch = 1;
        wake = 0;
    }

    else if(kb_input == F2 && alt == 1 && currentTerminal != 1 && wake && gui_flag == 1)
    {
        terminal2_switch = 1;
        wake = 0;
    }

    else if(kb_input == F3 && alt == 1 && currentTerminal != 2 && wake && gui_flag == 1)
    {
        terminal3_switch = 1;
        wake = 0;
    }


    // if there is a terminal waiting for the termianl being written into
    else if(flag_reading) {

        // check shift and caps lock
        if(kb_input == CAPS_CODE || kb_input == LSHIFT_CODE || kb_input == RSHIFT_CODE
                || kb_input == ULSHIFT_CODE || kb_input == URSHIFT_CODE) {
            if (caps == 1) {
                caps = 0;
            }
            else {
                caps = 1;
            }
        }


        // crtl code
        else if(kb_input == CTRL_CODE) {
            ctrl = 1;
        }

        else if(kb_input == UCTRL_CODE) {
            ctrl = 0;
        }


        // if up
        else if(kb_input == UP_CODE) {

            // if keyboard_prev has stored data from previous enter press
            if(cur_key_index_prev[currentTerminal] != 0) {

                map(VIDEO, VIDEO, _4kBPAGE);    // map video to itself to update video_mem to cur_terminal no matter the proccess
                int i;
                for(i = cur_key_index[currentTerminal] - 1; i >= 0; i--) {
                    if(kb_buffer_array[currentTerminal][i] != '\t' && kb_buffer_array[currentTerminal][i] != '\v') {
                        putc_kb('\b');
                    }
                    kb_buffer_array[currentTerminal][i] = NULL;
                }
                for(i = 0; i < SIZE_KB_BUFFER; i++) {
                    kb_buffer_array[currentTerminal][i] =  kb_buffer_array_prev[currentTerminal][i];
                }
                cur_key_index[currentTerminal] = cur_key_index_prev[currentTerminal];
                kb_buffer_array[currentTerminal][cur_key_index_prev[currentTerminal] - 1] = '\0';
                cur_key_index[currentTerminal]--;
                for(i = 0; i < cur_key_index[currentTerminal]; i++) {
                    putc_kb(kb_buffer_array[currentTerminal][i]);
                }
                cur_key_index_prev[currentTerminal] = 0;
                map(VIDEO, cur_vidmap, _4kBPAGE);    // set video memory back to vidmap set during scheduing for current process
                
            }

        }

      


        // clear screen on ctrl L
        else if(kb_input == L_CODE && ctrl == 1) {
            map(VIDEO, VIDEO, _4kBPAGE);    // map video to itself to update video_mem to cur_terminal no matter the proccess
            clear();
            putc_kb('\r');
            putc_kb('3');
            putc_kb('9');
            putc_kb('1');
            putc_kb('O');
            putc_kb('S');
            putc_kb('>');
            putc_kb(' ');
            init_KB_Buffer();
            map(VIDEO, cur_vidmap, _4kBPAGE);    // set video memory back to vidmap set during scheduing for current process

        }

        // backspace when key pressed and not at start of buffer
        else if(BACKSPACE_CODE == kb_input && cur_key_index[currentTerminal] && !enter_flag[currentTerminal]) {

            map(VIDEO, VIDEO, _4kBPAGE);    // map video to itself to update video_mem to cur_terminal no matter the proccess
            if(cur_key_index[currentTerminal] != 0) {

                // if backspacing a tab
                if(kb_buffer_array[currentTerminal][cur_key_index[currentTerminal] - 1] == '\t') {
                    int i;
                    // reset kb_buffer for spaces
                    for(i = 0; i < NUM_TAB_SPACES; i++){
                        cur_key_index[currentTerminal]--;
                        kb_buffer_array[currentTerminal][cur_key_index[currentTerminal]] = NULL;
                        putc_kb('\b');
                    }
                    // do one more deletion to remove tab
                    cur_key_index[currentTerminal]--;
                    kb_buffer_array[currentTerminal][cur_key_index[currentTerminal]] = NULL; 

                }
                // remove normal char
                else {
                    if(kb_buffer_array[currentTerminal][cur_key_index[currentTerminal] - 1] == '\v') {
                        cur_key_index[currentTerminal]--;
                        kb_buffer_array[currentTerminal][cur_key_index[currentTerminal]] = NULL; 
                    }
                    cur_key_index[currentTerminal]--;
                    kb_buffer_array[currentTerminal][cur_key_index[currentTerminal]] = NULL;
                    putc_kb('\b');
                }
            }
            map(VIDEO, cur_vidmap, _4kBPAGE);   // set video memory back to vidmap set during scheduing for current process
       
            
        }

        // Insert spaces when tab is pressed
        else if(kb_input == TAB_CODE) {      // && !enter_flag[currentTerminal] && cur_key_index[currentTerminal] < SIZE_KB_BUFFER - 5

            map(VIDEO, VIDEO, _4kBPAGE);    // map video to itself to update video_mem to cur_terminal no matter the proccess


            int space_length = check_space();
            int i;
            for(i = 0; i < SIZE_NAME_BUF; i++) {
                if(strncmp(kb_buffer_array[currentTerminal] + space_length, auto_fill_buffer.name_buf[i], 2) == 0) {
                    auto_fill_func(auto_fill_buffer.name_buf[i], space_length);
                    break;
                }
            }
            if(i == SIZE_NAME_BUF) {

                // 6 for frame_.txt, compares number so autofill can work with both
                if(cur_key_index[currentTerminal] - space_length < 6 && strncmp(kb_buffer_array[currentTerminal] + space_length, auto_fill_buffer.frame0, 2) == 0) {
                    auto_fill_func(auto_fill_buffer.frame0, space_length);
                }
                else {
                    if(strncmp(kb_buffer_array[currentTerminal] + space_length, auto_fill_buffer.frame0, 6) == 0) {
                        auto_fill_func(auto_fill_buffer.frame0, space_length);
                    }
                    else if(strncmp(kb_buffer_array[currentTerminal] + space_length, auto_fill_buffer.frame1, 6) == 0) {
                        auto_fill_func(auto_fill_buffer.frame1, space_length);
                    }
                    else if(!enter_flag[currentTerminal] && cur_key_index[currentTerminal] < SIZE_KB_BUFFER - (NUM_TAB_SPACES+1)){ 
                        int j = 0;
                        for(j = 0; j < NUM_TAB_SPACES; j++){
                            cur_key_index[currentTerminal]++;
                            kb_buffer_array[currentTerminal][cur_key_index[currentTerminal] - 1] = ' ';
                            putc_kb(' ');
                        }
                        cur_key_index[currentTerminal]++;
                        kb_buffer_array[currentTerminal][cur_key_index[currentTerminal] - 1] = '\t'; 
                    }
                }
            

    
            }
            map(VIDEO, cur_vidmap, _4kBPAGE);    // set video memory back to vidmap set during scheduing for current process
            
        }

        // When key that can be printed is pressed
        if(kb_input <= SIZE_SCANCODE_BUF - 1 && !ctrl && cur_key_index[currentTerminal] < SIZE_KB_BUFFER - 1 && !enter_flag[currentTerminal]) { // BUG LOG!
            // don't print if ERROR code
            if(scancodes_buffer[kb_input][caps] == NOT_PRINTABLE) {}
            
            // print char to screen and save to buffer
            else {
                map(VIDEO, VIDEO, _4kBPAGE);    // map video to itself to update video_mem to cur_terminal no matter the proccess

                cur_key_index[currentTerminal]++;
                kb_buffer_array[currentTerminal][cur_key_index[currentTerminal] - 1] = scancodes_buffer[kb_input][caps];
                putc_kb(kb_buffer_array[currentTerminal][cur_key_index[currentTerminal] - 1]);

                map(VIDEO, cur_vidmap, _4kBPAGE);    // set video memory back to vidmap set during scheduing for current process
            }
            
        }

        // if enter is pressed, wake terminal read and not allow inputs until read is processed
        if(kb_input == ENTER_CODE && !enter_flag[currentTerminal]) {

            enter_flag[currentTerminal] = 1;    // set enter foag to one so no more inputs can be inputed until termianl read is finished

            kb_buffer_array[currentTerminal][cur_key_index[currentTerminal]] = '\n';
            cur_key_index[currentTerminal]++;

            map(VIDEO, VIDEO, _4kBPAGE);    // map video to itself to update video_mem to cur_terminal no matter the proccess
            putc_kb('\n');
            map(VIDEO, cur_vidmap, _4kBPAGE);    // set video memory back to vidmap set during scheduing for current process

            // only save data to kb_buffer if > 1 keys were pressed (like in counter)
            if(strlen( kb_buffer_array[currentTerminal]) > 2) {
                int i;
                for(i = 0; i < SIZE_KB_BUFFER; i++) {
                    kb_buffer_array_prev[currentTerminal][i] =  kb_buffer_array[currentTerminal][i];
                }
                cur_key_index_prev[currentTerminal] = cur_key_index[currentTerminal];
            }


            terminal_wake[currentTerminal] = 1;     // set terminal to wake up to finish terminal read
            
        }

    }

    send_eoi(0x01); // sends end of interrupt fo keyboard (from IDT table, kb = 0x21 -> P_PIC IRQ1)

}


/* auto_fill_func
 * 
 * auto fill the kb_buffer_array when tab is pressed 
 * Inputs:  needed_auto_fill -- character array that contains the characters that will be automatically filled in the input field
 *          space_length -- amount of spaces to be filled
 * Outputs: None
 * Side Effects: Prints needed_auto_fill
 */
void auto_fill_func(char * needed_auto_fill, int space_length) {
    int i = 0;  // incrementer
    int fill_length = strlen(needed_auto_fill); // get the length of the auto fill
    int j = space_length;   //amount of spaces to be filled

    // fill it with a backspace
    for(i = cur_key_index[currentTerminal] -1; i >= space_length; i--) {
        putc_kb('\b');
    }

    // inserting characters from the needed_auto_fill into kb_buffer_array
    // then print them to the screen
    for(i = 0; i < fill_length && j < SIZE_KB_BUFFER - 2; i++, j++) {
        kb_buffer_array[currentTerminal][j] = needed_auto_fill[i];
        putc_kb(needed_auto_fill[i]);
    }

    //update index of the current key in kb_buffer_array and adds a vertical tab

    // if run out of space on keyboard
    if(j == SIZE_KB_BUFFER - 2) {
        // if it reaches end of the line, put a vertical tab at wherever we are in kb_buffer_array
        kb_buffer_array[currentTerminal][j] = '\v';
        cur_key_index[currentTerminal] = j + 1;
    }
    else{
        //otherwise, just put it right after the current character
        kb_buffer_array[currentTerminal][j] = '\v';
        cur_key_index[currentTerminal] = fill_length + space_length + 1; //update cur_key_index to the very end of the array
    }
}


/* check_space
 * 
 * determines the length of whitespace before the current position of the cursor in kb_buffer_array
 * 
 * Inputs: None
 * Outputs: Number of spaces needed
 * Side Effects: None
 */
int check_space() {
    int i;
    int space_length = 0;
    // check for space behind current typing
    for(i = cur_key_index[currentTerminal] -1; i >= 0; i--) {
        if( kb_buffer_array[currentTerminal][i] == ' ' || kb_buffer_array[currentTerminal][i] == '\t' || kb_buffer_array[currentTerminal][i] == '\v') {
            space_length = i;
            break;
        }
    }
    // if no space found, set returned index to 0
    if(space_length == 0) {
        return space_length;
    }
    // return next index after space if space found
    return space_length + 1;
}

