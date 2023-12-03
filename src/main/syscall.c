#include "syscall.h"


#define BUFSIZE 1024 // buf size based on given syscall code in ../syscalls folder
#define MAGIC_NUM_0 0x7f
#define MAGIC_NUM_1 0x45
#define MAGIC_NUM_2 0x4c
#define MAGIC_NUM_3 0x46



/* 
 * sys_close
 *   DESCRIPTION:   provides access to the file system
 *   INPUTS: filename -- pointer to the filename
 *   OUTPUTS: None
 *   RETURN VALUE: fd index number
 *                 -1 -- upon failure
 *   SIDE EFFECTS: None
 */
int32_t sys_open(const uint8_t* filename)
{   
    /* set up the elements that we need to put into the fda entry in pcb */
    dentry_t current; 
    if(read_dentry_by_name(filename, &current) != 0)
    {
        return -1; 
    }
    file_operations_t fotp_current; 

    /*fill out fotp based on file typte: 0 for RTC, 1 for Directory, 2 for File*/
    switch(current.file_type){
        case 0: // first case = rtc
            fotp_current.read = (void *) rtc_read; 
            fotp_current.write = (void *) rtc_write; 
            fotp_current.open = (void *) rtc_open; 
            fotp_current.close = (void *) rtc_close; 
            break;
        case 1: // second case = directory
            fotp_current.read = (void *) directory_read; 
            fotp_current.write = (void *) directory_write; 
            fotp_current.open = (void *) directory_open; 
            fotp_current.close = (void *) directory_close; 

            break;
        case 2: // third case = file
            fotp_current.read = (void *) file_read; 
            fotp_current.write = (void *) file_write; 
            fotp_current.open = (void *) file_open; 
            fotp_current.close = (void *) file_close; 
            break;
    } 
    
    /* only one pcb should be active at once, so we check which one is active to tell which process we are in */ 
    if(pcb_array[pcbIndex]->active == 1) // if it's active
    {
        int i;  
        for(i = START_FDT_FILES; i < NUM_FDT_ENTRIES; i++)    // sort through the fdt entries minus the boot block
        {
            if(pcb_array[pcbIndex]->fd_array.fdt[i].flags[FD_FLAG] == 0)  // only if flags = 0, meaning it was previously closed
            {
                /* populates the fda array entry with the correct elements */
                pcb_array[pcbIndex]->fd_array.fdt[i].flags[FILE_TYPE] = (uint16_t) current.file_type;       // set to the current file type. --> this flag is used by sys_write so the rtc can be written to (only file with write permissions besides std_out)
                pcb_array[pcbIndex]->fd_array.fdt[i].flags[FD_FLAG] = 1;                                  //set the flag to 1 for active 
                pcb_array[pcbIndex]->fd_array.fdt[i].inode_num = current.inode_num;                 //set the inode ptr 
                pcb_array[pcbIndex]->fd_array.fdt[i].fotp = fotp_current;                                                //set the fotp to the fotp created 
                pcb_array[pcbIndex]->fd_array.fdt[i].file_pos = 0; //pcb_array[pcbIndex]->fd_array.fdt[i].inode_ptr->data_block_num[1]; start reading at byte 0     //set the file_pos to the current data block at the start, 0 is the len
                
                return i; // what do we return if successful? --return fd index number
            }
        }
    }
    return -1;  //return -1 if there are no spots on either fda to open a new file 
}

/* 
 * sys_close
 *   DESCRIPTION:  closes the specified file descriptor and makes it available for return from later calls to open
 *   INPUTS: fd -- file descriptor for what type of file it is (directory, rtc, or file)
 *   OUTPUTS: None
 *   RETURN VALUE: -1 -- upon failure/invalid
 *                 0 -- upon success
 *   SIDE EFFECTS: None
 */
int32_t sys_close(int32_t fd)
{
    
    if(fd < START_FDT_FILES || fd > MAX_FD) // cant close stdin and stdout
        return -1; //invalid fd number, and cannot close stdin and stdout 

    // set them all to null
    file_operations_t fotp_null; 
    fotp_null.read = NULL; 
    fotp_null.write = NULL; 
    fotp_null.open = NULL; 
    fotp_null.close = NULL; 

    if(pcb_array[pcbIndex]->active == 1 && pcb_array[pcbIndex]->fd_array.fdt[fd].flags[FD_FLAG] == 1)    // if active
    {
        /* makes the fda entry inactive and makes all the elements 0 or NULL*/ 
        pcb_array[pcbIndex]->fd_array.fdt[fd].flags[FILE_TYPE] = -1;    // reset first flag
        pcb_array[pcbIndex]->fd_array.fdt[fd].flags[FD_FLAG] = 0;     // reset second flag
        pcb_array[pcbIndex]->fd_array.fdt[fd].fotp = fotp_null; 
        pcb_array[pcbIndex]->fd_array.fdt[fd].inode_num = -1;   // reset inode number
        pcb_array[pcbIndex]->fd_array.fdt[fd].file_pos = 0;     // reset file_pos

        //pcb_array[pcbIndex]->fd_array.fdt[fd].fotp.close(fd);  //call close on the file we want to close, should do nothing 
        return 0; //successfully closed 
    }
    return -1;  //no processes open 
}

/* 
 * sys_read
 *   DESCRIPTION:  call reads data from the keyboard, a file, device (RTC), or directory
 *   INPUTS: fd -- file descriptor for what type of file it is (directory, rtc, or file)
 *           buf -- buffer for what to read
 *           nbytes -- number of bytes to read
 *   OUTPUTS: None
 *   RETURN VALUE: -1 -- if it didn't work
 *                 otherwise -- number of bytes read
 *   SIDE EFFECTS: Changes frequency value for rtc
 */
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes)
{
    if(fd < 0 || fd > MAX_FD || fd == STDOUT_FDT) // < 0 does not exist
        return -1; // return -1 for invalid
    
    if(pcb_array[pcbIndex]->active == 1 && pcb_array[pcbIndex]->fd_array.fdt[fd].flags[FD_FLAG] == 1){   // if active
        return pcb_array[pcbIndex]->fd_array.fdt[fd].fotp.read(fd, buf, nbytes);
    }
    return -1;
}

/* 
 * sys_write
 *   DESCRIPTION:  writes data to the terminal or to a device (RTC)
 *   INPUTS: fd -- file descriptor for what type of file it is (directory, rtc, or file)
 *           buf -- buffer for what to write
 *           nbytes -- number of bytes to write
 *   OUTPUTS: None
 *   RETURN VALUE: -1 -- if it didn't work
 *                 -1 -- [file and directory] cant write to file or directory
 *                 nbytes -- [rtc] number of bytes written for rtc
 *   SIDE EFFECTS: Changes frequency value for rtc
 */
int32_t sys_write(int32_t fd, const void * buf, int32_t nbytes)
{
    // testing if write works from intx80 call
    if(fd == STDOUT_FDT) { // for non directory or rtc files
        if(pcb_array[pcbIndex]->active == 1) { // if it is active
            return pcb_array[pcbIndex]->fd_array.fdt[STDOUT_FDT].fotp.write(fd, buf, nbytes); // execute system call to write
        }
    }

    // if out of bounds
    if(fd < STDOUT_FDT || fd > MAX_FD){// if out of bounds, cant read from stdout
        return -1; // return -1 for invalid
    } 

    if(pcb_array[pcbIndex]->fd_array.fdt[fd].flags[FILE_TYPE] == 0 && pcb_array[pcbIndex]->fd_array.fdt[fd].flags[FD_FLAG] == 1){ //0 for rtc
        return pcb_array[pcbIndex]->fd_array.fdt[fd].fotp.write(fd, buf, nbytes);
    } 

    return -1;
}

/* 
 * sys_execute_setup
 *   DESCRIPTION: does the execute system call, this is where the inline assembly jumps -- executes a given system call
 *   INPUTS: cmd -- command for the system call (ex. ls)
 *           esp_needed -- the esp value we need for context switch
 *           ebp_needed -- the ebp value we need for context switch
 *   OUTPUTS: None
 *   RETURN VALUE: -1 -- if it didn't work
 *                  0 -- if everything worked correctly
 *   SIDE EFFECTS: Will print to the screen
 */
uint32_t sys_execute_setup(uint8_t * cmd, uint32_t esp_needed, uint32_t ebp_needed) {
    
    dentry_t file_dentry; // file dentry to read from
    int i; // loop var
    char executable_magic_num[4]; // magic number is four bytes
    int is_exe = 0; // file is exe bool
    uint32_t file_name_check; 

    uint8_t r_cmd[BUFSIZE];
    uint8_t temp_buf[BUFSIZE];
    int checkShell = 0; 
    int k = 0;
    //parse arg, make sure it's validW
    int8_t shell[6] = "shell\0";
    int8_t color[6] = "color\0";
    int8_t paint[6] = "paint\0";
    
    // impliments clear if typed
    int8_t clear_arg[6] = "clear\0";
    if(!strncmp((int8_t*)cmd, clear_arg, 6)){
        clear();
        putc('\r');
        init_KB_Buffer();
        return 0;
    }

    // cmd must not be a user level pointer to store in pcb to get name
    if(strncmp((int8_t*) cmd, shell,6) == 0)
    {
        checkShell = 1;
        r_cmd[0] = 's';
        r_cmd[1] = 'h';
        r_cmd[2] = 'e';
        r_cmd[3] = 'l';
        r_cmd[4] = 'l';
        r_cmd[5] = '\0';
    }

    if(strncmp((int8_t*) cmd, color,6) == 0) {
       return color_helper();
    }

    if(strncmp((int8_t*) cmd, paint,6) == 0) {
        map(VIDEO, VIDEO, _4kBPAGE);
        paint_color = 0x0C; // start with orange
        finish_paint = 0;
        paint_flag = 1;
        int8_t saved_attrib = ATTRIB[currentTerminal];
        ATTRIB[currentTerminal] = 0xF0;
        clear();
        putc_kb('\r');

        status_bar_paint();
        putc_kb('\r');
        sti();
        while(1){

            if(finish_paint) {
                paint_flag = 0;
                cli(); 
                break;
            }


        }
        gui_flag = 0;
        ATTRIB[currentTerminal] = saved_attrib;
        clear();
        putc_kb('\r');
        return 0;
    }


    // parses through command
    if(pcb_array[pcbIndex]->active == 1 && checkShell == 0) {
        int8_t space[1] = {' '};
        int8_t term_null[1] = {'\0'};

        while(1){
            // put buffer into real cmd until space
            if(strncmp((int8_t*) cmd + k, space, 1) == 0 || strncmp((int8_t*) cmd + k, term_null, 1) == 0) {
                if(strncmp((int8_t*) cmd + k, space, 1) == 0) {
                    r_cmd[k] = '\0';
                }
                // then put arguments into temp_buf
                while(strncmp((int8_t*) cmd + k, term_null, 1) != 0) {
                    temp_buf[k] = cmd[k];
                    k++;
                }
                // breaks out of entire while loop
                break;
            }
            // before spaces, save all
            r_cmd[k] = cmd[k]; 
            temp_buf[k] = cmd[k];
            k++;
        }
        r_cmd[k] = '\0';
        temp_buf[k] = '\0';
    }

    file_name_check = read_dentry_by_name(r_cmd, &file_dentry); // read the dentry by filename

    // check if it's an exe, read in the cmd
                                            /* changed cmd to r_cmd*/
    if(file_name_check != 0) // if file name is invalid
        return -1; // invalid = return -1

    uint8_t buf[200000]; // set buf arbitrarily large
    int32_t bytes = read_data(file_dentry.inode_num, 0, buf, 200000); // read the data with 2000000 bytes (size of buf)

    for(i = 0; i < 4; i++){ /* get the magic number from buf */
        executable_magic_num[i] = buf[i];
    }

    if(executable_magic_num[0] == MAGIC_NUM_0 && executable_magic_num[1] == MAGIC_NUM_1 && executable_magic_num[2] == MAGIC_NUM_2 && executable_magic_num[3] == MAGIC_NUM_3){ /* if magic number is according to these numbers*/
        is_exe = 1; //it is an exe file
    }
    else{
        return -1; /* if nothing, return -1 for error */
    }
    
    
    //get bytes 24 - 27 (the first instruction)
    uint32_t first_instruction = 0x00000000; // to bit shift into this
    uint8_t first_instruction_char[4]; // four bytes in first instruction
    int j = 0; 
    for(i = 24; i < 28; i++){ /* bytes 24 - 27 = first instruction */
        first_instruction_char[j] = buf[i]; // write to the array
        j++;
    }

    // bit shift the bytes into first_instruction (Little Endian)
    first_instruction = ((uint32_t) first_instruction_char[3] << 24); // put first byte of eip into top of first_instruction W000
    first_instruction += ((uint32_t) first_instruction_char[2] << 16); // put second byte of eip in. WX00
    first_instruction += ((uint32_t) first_instruction_char[1] << 8); // put third byte of eip in. WXY0
    first_instruction += (uint32_t) first_instruction_char[0]; // put fourth byte of eip in. WXYZ

    /*
    * map virtual address of 128 MB to physical address of 8 MB
    * virtual address = 0x08000000 = 128 MB
    * physical address = 0x800000 = 8 MB
    */
    cli();
    int lastpcbindex = pcbIndex;

    // move below too many processes so pcb_incex is not incremented
    int pcbNextIdx = getNextPCBIndex(); // get the next pcb's index
    int numProc = getNumProcess(currentTerminal); 
    // makes sure not more than 4 process running before terminal 2 and terminal 3 shells have been activated, 6 total once all are active
    if(pcbNextIdx == -1 || numProc >= 4 || (getNumProcess(1) == 0 && (getNumProcess(0)+getNumProcess(2) >= 5 && currentTerminal != 1)) || (getNumProcess(2) == 0 && (getNumProcess(0)+getNumProcess(1) >= 5) && currentTerminal != 2))
    {
        pcbIndex = lastpcbindex; 
        int8_t saved_attrib = ATTRIB[pcb_array[pcbIndex]->terminalNum];
        ATTRIB[pcb_array[pcbIndex]->terminalNum] = (ATTRIB[pcb_array[pcbIndex]->terminalNum] & 0xF0) + WHITE;
        printf("  Too many procceses open, quit some!\n"); 
       ATTRIB[pcb_array[pcbIndex]->terminalNum] = saved_attrib; 
        return 0; // return 0 if too many processes
    }

    /* we need to mark the parent pcb as -1 if main shell */ 
    if(lastpcbindex == 0 && pcbIndex == 0)
    {
        lastpcbindex = -1; 
    }

    // to find the number of processes
    if(getNumProcess(currentTerminal) == 0)
    {
        pcb_array[pcbNextIdx]->mainShell = 1;
        // 3 pcbs will have parentID -1 (for three terminals) 
        pcb_array[pcbNextIdx]->parentID = -1; 
    }
    else
    {
        pcb_array[pcbNextIdx]->parentID = lastpcbindex; //set parent to previous index
        pcb_array[lastpcbindex]->has_child = 1;
    }

    /*setup the new pcb */
    sys_open((uint8_t *)file_dentry.file_name);  //open the file to put in the pcb 
    pcb_array[pcbNextIdx]->saved_ebp = ebp_needed;  //save the ebp and esp (for scheduling)
    pcb_array[pcbNextIdx]->saved_esp = esp_needed; 
    pcb_array[pcbNextIdx]->original_ebp = ebp_needed;  //save the ebp and esp (for halt)
    pcb_array[pcbNextIdx]->original_esp = esp_needed; 
    pcb_array[pcbNextIdx]->pid = pcbNextIdx; //set pid to next pid
    pcb_array[pcbNextIdx]->active = 1; // set active bit to 1  
    pcb_array[pcbNextIdx]->terminalNum = currentTerminal; 
    pcb_array[pcbNextIdx]->is_waiting = 0;
    pcb_array[pcbNextIdx]->name = (int8_t *) r_cmd;
    

    // terminal_write(0, cmd, 15); 
    map(_128MB, EIGHTMB + (pcbNextIdx*FOURMB), _4MBPAGE); /* map from 128 MB --> physical memory (8MB) */
    // terminal_write(0, cmd, 15);
    pcb_array[pcbNextIdx]->cmd_ = temp_buf;

    // void* memcpy(void* dest, const void* src, uint32_t n) {
    memcpy((void*) PROGIMG, (void*) buf, bytes); /* copy to memory */

    if(welcome == 0) {
        clear();
        putc('\r');
        int8_t saved_attrib = ATTRIB[pcb_array[pcbIndex]->terminalNum];
        ATTRIB[pcb_array[pcbIndex]->terminalNum] = RED;
        status_bar_welcome();
        ATTRIB[pcb_array[pcbIndex]->terminalNum] = saved_attrib;
        welcome = 1;
        startmouse = 1; 
        saved_cursor_font = ATTRIB[currentTerminal];
        gui_flag = 0;
        paintScreen(); 
        update_cursor(0,0); 
        sti(); 
        while(gui_flag == 0)
        {

        }
        cli();

    }
    
    if(gui_flag == 1)
        status_bar_init();
    if(gui_flag == 2) {
        status_bar_pingpong();
    }
    if(gui_flag == 3) {
        status_bar_paint();
    }
    if(gui_flag == 4) {
        status_bar_fish();
    }
    if(gui_flag == 5) {
        status_bar_hello();
    }
    if(gui_flag == 6) {
        status_bar_counter();
    }
    if(gui_flag == 7) {
        status_bar_ls();
    }

    tss.esp0 = ESP0-(EIGHTKB*(pcbNextIdx)) - 0x04;  // set up kernel stack with tss (-0x04 to not overwrite pcb)

    //User DS
    //ESP
    //EFLAG
    //CS
    //EIP 
    uint32_t user_ds = USER_DS; // selector
    uint32_t user_cs = USER_CS; // selector
    uint32_t user_esp = USER_ESP; //since the stack is the bottom of the program in memory, added 4MB and subtracted 4B
    uint32_t user_eip = first_instruction; 

    /* do inline instead of calling because that sometimes messes with the stack */
    asm volatile (
        

        "pushl %0; \n"            //pushes the ds 
        "pushl %2; \n"            //pushes the current ESP
        "pushfl;   \n"    
        "popl %%edx;\n"           //takes the flags 
        "orl $0x200, %%edx; \n"   //bit mask to disabled intterupts 
        "pushl %%edx; \n"         //pushes them back 
        "pushl %1; \n"            //pushes cs 
        "pushl %3; \n"            //pushes the eip 
        "iret; \n"


        

        :
        : "g"(user_ds),"g"(user_cs),"g"(user_esp),"g"(user_eip) //adding the variables into the inline assembly 
        : "%edx","memory","cc" //restore the edx, memory, and cc if clobbered 
    );

    return 0; //return nothing (will never get to)
}

/* 
 * sys_halt
 *   DESCRIPTION: system call terminates a process, returning the specified value to its parent process
 *   INPUTS: status -- current status of program, is it finished/not finished
 *   OUTPUTS: None
 *   RETURN VALUE: 0 -- if everything worked correctly
 *   SIDE EFFECTS: Will print to the screen
 */
int32_t sys_halt(uint8_t status){

    if(gui_flag == -2) {
        sti();
        while(1);
    }
    

    cli();
    pcb_t* curr_pcb = pcb_array[pcbIndex];
    

    // check if main shell = parent pid = -1
    if(pcb_array[pcbIndex]->parentID == -1){ 
        // close fd arrays, then execute shell again
        if(gui_flag == 1)
            init_pcb(pcb_array[pcbIndex]); //(reset shell pcb)
        pcbIndex = 0; // set index to 0 for start 
        sys_execute_setup((uint8_t *)"shell", NULL, NULL); // call the execute (set esp and edp to NULL since not going to be used)
    }
    else{ // if not main shell
        
        pcb_t* parent_pcb = pcb_array[curr_pcb->parentID];
        tss.esp0 = ESP0-(EIGHTKB*(parent_pcb->pid)) - 0x04; // reset tss to pid (-0x04 to not overwrite pcb)
        unmap(_128MB, _4MBPAGE);
        unmap(_128MB+ EIGHTMB, _4kBPAGE);
        map(_128MB, EIGHTMB + (parent_pcb->pid*FOURMB), _4MBPAGE); //map parent's paging
        uint32_t curr_ebp = pcb_array[pcbIndex]->original_ebp; // set the current ebp
        uint32_t curr_esp = pcb_array[pcbIndex]->original_esp; // set the current esp
        init_pcb(curr_pcb); // initialize (reset) pcb
        parent_pcb->has_child = 0;
        pcbIndex = parent_pcb->pid; // set it to the parent's pid
        if(gui_flag == 1)
            status_bar_init();


        // resetting esp ebp based on parent
        // moving return value into eax
         asm volatile (

            "movzbl %%bl, %%eax; \n"
            "movl %1, %%esp; \n"
            "movl %0, %%ebp; \n"
            "jmp SYS_HALT_RET; \n"

            :
            : "g"(curr_ebp),"g"(curr_esp) //adding the variables into the inline assembly 
            :"%esp","%ebp","memory","cc"
        );
    }

    return 0;
}

int32_t sys_halt_pcb(uint8_t pcb){

    cli();
    pcb_t* curr_pcb = pcb_array[pcb];
    
    pcb_t* parent_pcb = pcb_array[curr_pcb->parentID];
    tss.esp0 = ESP0-(EIGHTKB*(parent_pcb->pid)) - 0x04; // reset tss to pid (-0x04 to not overwrite pcb)
    unmap(_128MB, _4MBPAGE);
    unmap(_128MB+ EIGHTMB, _4kBPAGE);
    map(_128MB, EIGHTMB + (parent_pcb->pid*FOURMB), _4MBPAGE); //map parent's paging
    // uint32_t curr_ebp = pcb_array[pcb]->original_ebp; // set the current ebp
    // uint32_t curr_esp = pcb_array[pcb]->original_esp; // set the current esp
    init_pcb(curr_pcb); // initialize (reset) pcb
    parent_pcb->has_child = 0;
    // pcbIndex = parent_pcb->pid; // set it to the parent's pid
    // if(gui_flag == 1)
    //     status_bar_init();


    // // resetting esp ebp based on parent
    // // moving return value into eax
    //  asm volatile (

    //     "movzbl %%bl, %%eax; \n"
    //     "movl %1, %%esp; \n"
    //     "movl %0, %%ebp; \n"
    //     "jmp SYS_HALT_RET; \n"

    //     :
    //     : "g"(curr_ebp),"g"(curr_esp) //adding the variables into the inline assembly 
    //     :"%esp","%ebp","memory","cc"
    // );

    return 0;
}



/* 
 * sys_getargs
 *   DESCRIPTION: takes the command and parses it to find the arguments 
 *   INPUTS: buf -- a buffer used to put in the arguments on return, nbytes -- the number of bytes we to read 
 *   OUTPUTS: None
 *   RETURN VALUE: 0 -- if everything worked correctly
 *                 -1 -- if invalid input
 *   SIDE EFFECTS: None
 */
int32_t sys_getargs(uint8_t* buf, int32_t nbytes)
{

    if(cat_flag) {
        clear();
        putc('\r');
        status_bar_cat_setup();
        paintScreen_cat(); 
        update_cursor(0,0); 
        sti(); 
        while(cat_flag)
        {
            if(cat_flag == 2) {
                cli();
                buf[0] = 'f';
                buf[1] = 'r';
                buf[2] = 'a';
                buf[3] = 'm';
                buf[4] = 'e';
                buf[5] = '0';
                buf[6] = '.';
                buf[7] = 't';
                buf[8] = 'x';
                buf[9] = 't';
                buf[10] = '\0';
                cat_flag = 0;
                return 0;
            }
            if(cat_flag == 3) {
                cli();
                buf[0] = 'f';
                buf[1] = 'r';
                buf[2] = 'a';
                buf[3] = 'm';
                buf[4] = 'e';
                buf[5] = '1';
                buf[6] = '.';
                buf[7] = 't';
                buf[8] = 'x';
                buf[9] = 't';
                buf[10] = '\0';
                cat_flag = 0;
                return 0;
            }

            if(cat_flag == 4) {
                cli();
                buf[0] = 'v';
                buf[1] = 'e';
                buf[2] = 'r';
                buf[3] = 'y';
                buf[4] = 'l';
                buf[5] = 'a';
                buf[6] = 'r';
                buf[7] = 'g';
                buf[8] = 'e';
                buf[9] = 't';
                buf[10] = 'e';
                buf[11] = 'x';
                buf[12] = 't';
                buf[13] = 'w';
                buf[14] = 'i';
                buf[15] = 't';
                buf[16] = 'h';
                buf[17] = 'v';
                buf[18] = 'e';
                buf[19] = 'r';
                buf[20] = 'y';
                buf[21] = 'l';
                buf[22] = 'o';
                buf[23] = 'n';
                buf[24] = 'g';
                buf[25] = 'n';
                buf[26] = 'a';
                buf[27] = 'm';
                buf[28] = 'e';
                buf[29] = '.';
                buf[30] = 't';
                buf[31] = 'x';
                buf[32] = '\0';
                cat_flag = 0;
                return 0;

            }

        }
        cli();
    }
    uint8_t *real_cmd; 
    real_cmd = pcb_array[pcbIndex]->cmd_; 
    int k = 0;

    //add null check 
    if(real_cmd == NULL)
        return -1;
    if(*real_cmd == '\0')
        return -1;

    //parse arg, make sure it's valid
    if(pcb_array[pcbIndex]->active == 1) {

        int arg_flag = 0; //flag to check filename/ string args for cat/grep

        // Parse the command string to look for the first space character
        while(real_cmd[k] != '\0'){
            if(real_cmd[k] == ' '){
                arg_flag = 1;
                real_cmd[k] = '\0';
                k++;
                break;
            }
            
            k++;
        }

        // If a filename or string argument is present and its length is less than nbytes - k, copy it into the buffer
        if(arg_flag && strlen((int8_t*)(real_cmd)) <= nbytes-k){
            int j = 0;
            while(real_cmd[k] != '\0'){
                buf[j] = real_cmd[k];
                j++;
                k++;
            }
            buf[j] = '\0';
        }    
        else{
            buf = NULL;
            return -1; // Return error if buffer contains only whitespace characters
        }

        // Check if the buffer contains only whitespace characters
        int8_t space[1] = {""};
        int8_t space2[1] = {" "};
        if(strncmp((int8_t*) buf, space, 1) == 0 || strncmp((int8_t*) buf, space2, 1) == 0 ) //check for if there are no arguments 
        {
            buf = NULL;
            return -1; 
        }
        return 0; 
    }

    return -1; 
}

/* 
 * sys_vidmap
 *   DESCRIPTION: call maps the text-mode video memory into user space at a pre-set virtual address
 *   INPUTS: screen_start -- arbitrary user input of where the screen should start -- will change later to our location for start
 *   OUTPUTS: None
 *   RETURN VALUE: 0 -- if everything worked correctly
 *                 -1 -- if invalid input
 *   SIDE EFFECTS: None
 */
int32_t sys_vidmap(uint8_t** screen_start)
{
    // null case
    if(screen_start == NULL){
        return -1; // didnt work because null 
    }

    //make sure its within the bounds of user memory
    if((uint32_t) screen_start < _128MB || (uint32_t)screen_start >= _128MB + FOURMB){
        return -1; // didnt work because invalid memory location
    }

    map(_128MB + EIGHTMB, VIDEO, _4kBPAGE); // map video memory to our virtual location
    *screen_start = (uint8_t*) (_128MB + EIGHTMB); /* set the memory location to where we're mapping video memory*/
    return 0; // return 0 upon success  
}
