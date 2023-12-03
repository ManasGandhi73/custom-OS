#include "scheduler.h"
#include "mouse.h"
/* 
 * pit_init
 *   DESCRIPTION: moves cursor
 *   INPUTS: 
 *   OUTPUTS: none
 *   RETURN VALUE: 
 *   SIDE EFFECTS: Enables IRQ0 (Timer Chip)
 */
void pit_init() {
    enable_irq(0x00); // IRQ 0 is PIT
}



/* 
 * pit_handler
 *   DESCRIPTION: handles PIT and terminal switching
 *   INPUTS: 
 *           previous process esp, ebp
 *   OUTPUTS: none
 *   RETURN VALUE: 
 *   SIDE EFFECTS: Switches terminal
 */
void pit_handler(uint32_t prev_esp, uint32_t prev_ebp) {
    
    cli();
    send_eoi(0x00); // IRQ 0 is PIT
    cli();
    pcb_t* currpcb = pcb_array[pcbIndex];
    

    map(VIDEO, VIDEO, _4kBPAGE);   // map video to itself so memcopy will work correctly

   

    if(terminal1_switch) {  // if Alt F1 was pressed
        // allow another terminal switch and reset flag
        undraw_cursor((double) mouse_x_overall * MOUSE_RL_SCALE, (double)(mouse_y_overall * -1) * MOUSE_UD_SCALE); // undraw cursor on terminal switch
        wake = 1;           
        terminal1_switch = 0;
        
        // set previous and currentTerminal
        int prevTerminal = currentTerminal;  
        currentTerminal = 0;
        terminalSwitch(prevTerminal, currentTerminal);
    }

    else if(terminal2_switch) {  // if ALt F2 was pressed
        // allow another terminal switch and reset flag    
        undraw_cursor((double) mouse_x_overall * MOUSE_RL_SCALE, (double)(mouse_y_overall * -1) * MOUSE_UD_SCALE); // undraw cursor on terminal switch 
        wake = 1;
        terminal2_switch = 0;

         // set previous and currentTerminal
        int prevTerminal = currentTerminal; 
        currentTerminal = 1; 

        if(secondTerm == 0) // if second terminal has yet to be initialized
        {
            terminalSwitch(prevTerminal, -1);   // only save previous terminal
            secondTerm = 1;  // set flag to 1
            clear();
            sys_execute_setup((uint8_t *)"shell", NULL, NULL);               
        }
        else
        {
            terminalSwitch(prevTerminal, currentTerminal); 
        }
    }
    else if(terminal3_switch) {
        undraw_cursor((double) mouse_x_overall * MOUSE_RL_SCALE, (double)(mouse_y_overall * -1) * MOUSE_UD_SCALE); // undraw cursor on terminal switch
        // allow another terminal switch and reset flag
        wake = 1;
        terminal3_switch = 0;

        // set previous and currentTerminal
        int prevTerminal = currentTerminal; 
        currentTerminal = 2; // we are on terminal 2

        if(thirdTerm == 0) // if second terminal has yet to be initialized
        {
            terminalSwitch(prevTerminal, -1); // only save previous terminal
            thirdTerm = 1; // set flag to 1
            clear();
            sys_execute_setup((uint8_t *)"shell", NULL, NULL);              
        }
        else
        {
            terminalSwitch(prevTerminal, currentTerminal); 
        }
    }

    schedule_setup(prev_esp, prev_ebp, currpcb);
    
}

/* 
 * schedule_setup
 *   DESCRIPTION: implements round-robin scheduling
 *   INPUTS: 
 *           previous process esp, ebp
 *   OUTPUTS: none
 *   RETURN VALUE: 
 *   SIDE EFFECTS: Switches process execution, sets video memory
 */
void schedule_setup(uint32_t prev_esp, uint32_t prev_ebp, pcb_t * currpcb){

    // save epb and esp into current pcb
    currpcb->saved_ebp = prev_ebp;
    currpcb->saved_esp = prev_esp;


    /*Round-Robin Scheduler: */

    // check every pcb from the next valid pcbIndex
    int32_t scheduledpcbIndex = (pcbIndex + 1)%PCBNUM;

    // Loop through all 5 other pcbIndexs
    while(scheduledpcbIndex != pcbIndex){

        // if pcb is opened and is not a parent shell
        if(pcb_array[scheduledpcbIndex]->active && !pcb_array[scheduledpcbIndex]->has_child){ 

            // Skip pcb if it's a shell waiting for input not on current terminal (for efficiency)
            if(pcb_array[scheduledpcbIndex]->is_waiting && pcb_array[scheduledpcbIndex]->terminalNum != currentTerminal) {
                scheduledpcbIndex = (scheduledpcbIndex + 1)%PCBNUM;
                continue;
            }
            // is valid pcb to switch too
            else{
                break;
            }
        }
        // increment scheduledpcbindex
        scheduledpcbIndex = (scheduledpcbIndex + 1)%PCBNUM; 
    }

    // setup next process (can be same as current)

    pcb_t * scheduledpcb = pcb_array[scheduledpcbIndex];
    
    // reset tlb
    unmap(_128MB, _4MBPAGE);
    unmap(_128MB+ EIGHTMB, _4kBPAGE);
    map(_128MB, EIGHTMB + (scheduledpcb->pid*FOURMB), _4MBPAGE); //map to next process
    uint32_t vidterm;
    
    // switch where video is mapped depending on terminal user is on and terminal pcb is on
    if(scheduledpcb->terminalNum == currentTerminal) {
        vidterm = VIDEO;
    }
    else{
        switch(scheduledpcb->terminalNum){
            case 0:
                vidterm = VIDEOT1;
                break;
            case 1:
                vidterm = VIDEOT2;
                break;
            case 2:
                vidterm = VIDEOT3;
                break;
            default:
                vidterm = VIDEO;
        }
    }
    // set global variable to use in kb_handler
    cur_vidmap = vidterm;
    map(VIDEO, vidterm, _4kBPAGE);
    map(_128MB+ EIGHTMB, vidterm, _4kBPAGE);


    uint32_t sched_ebp = scheduledpcb->saved_ebp; // set the current ebp
    uint32_t sched_esp = scheduledpcb->saved_esp; // set the current esp
    
    pcbIndex = scheduledpcbIndex; // set pcb_index
    tss.esp0 = ESP0-(EIGHTKB*(scheduledpcb->pid)) - 0x04; // reset tss to pid (-0x04 to not overwrite pcb) 
    // switch kernel stacks
    asm volatile (

        "movl %1, %%esp; \n"
        "movl %0, %%ebp; \n"
        "jmp testreturn; \n"
        :
        : "g"(sched_ebp),"g"(sched_esp) //adding the variables into the inline assembly 
        :"%esp","%ebp","memory","cc"
    );

}






