#include "IDT.h"
#include "filesys.h"

/* init_idt
 * 
 * initializes idt, sets up exceptions, devices interrupts, and system call
 * Inputs: None
 * Outputs: None
 * Side Effects: sets up idt with requirements form checkpoint 3.1
 */
void init_idt() {
    int i;
    for(i = 0; i < NUM_VEC; i++) {
        // All info is from 5.11 to 5.14 of intel manual

        // set them all to not present
        idt[i].present = 0;
        // interrupt handlers have their DPL set to 0
        idt[i].dpl = KERNEL_PRIV;
        // From interrupt gate diagram:
        idt[i].reserved0 = 0;
        // Size of all interrupt gates is 32 bits = 1
        idt[i].size = 1;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        // set all as interrupt gates at first (since are exceptions)
        idt[i].reserved3 = 0;
        idt[i].reserved4 = 0;


        // Each IDT entry also contains a segment selector field that specifies 
        // a code segment in the GDT, and you should set this field to be the 
        // kernel’s code segment descriptor.
        idt[i].seg_selector = KERNEL_CS;

    }

    for(i = 0; i < NUM_EXCEPTIONS; i++) {
        // set exceptions to present
        idt[i].present = 1;
        idt[i].reserved3 = 1; // turn on trap gate
    }
    
    idt[15].present = 0; // set 15 to present

    // maybe interrupt gate?
    idt[vec_nmi_interrupt].reserved3 = 0;

    // Allow for user space to interrupt the system call handler
    idt[SYS_CALL_V].dpl = USER_PRIV;
    SET_IDT_ENTRY(idt[SYS_CALL_V], sys_call_linkage);
    idt[SYS_CALL_V].present = 1;

    // Set exception
    SET_IDT_ENTRY(idt[vec_divide_error], divide_error);
    SET_IDT_ENTRY(idt[vec_debug], debug);
    SET_IDT_ENTRY(idt[vec_nmi_interrupt], nmi_interrupt);
    SET_IDT_ENTRY(idt[vec_breakpoint], breakpoint);
    SET_IDT_ENTRY(idt[vec_overflow], overflow);
    SET_IDT_ENTRY(idt[vec_bound_range_exceeded], bound_range_exceeded);
    SET_IDT_ENTRY(idt[vec_invalid_opcode], invalid_opcode);
    SET_IDT_ENTRY(idt[vec_device_not_available], device_not_available);
    SET_IDT_ENTRY(idt[vec_double_fault], double_fault);
    SET_IDT_ENTRY(idt[vec_coprocessor_segment_overrun], coprocessor_segment_overrun);
    SET_IDT_ENTRY(idt[vec_invalid_tss], invalid_tss);
    SET_IDT_ENTRY(idt[vec_segment_not_present], segment_not_present);
    SET_IDT_ENTRY(idt[vec_stack_fault], stack_fault);
    SET_IDT_ENTRY(idt[vec_general_protection], general_protection);
    SET_IDT_ENTRY(idt[vec_page_fault], page_fault);
    SET_IDT_ENTRY(idt[vec_x87_floating_point], x87_floating_point);
    SET_IDT_ENTRY(idt[vec_alignment_check], alignment_check);
    SET_IDT_ENTRY(idt[vec_machine_check], machine_check);
    SET_IDT_ENTRY(idt[vec_simd_floating_point], simd_floating_point);

    // set interrupt vectors for devices and set them to present
    SET_IDT_ENTRY(idt[KB_V], KB_handler_linkage);
    idt[KB_V].present = 1;
    SET_IDT_ENTRY(idt[RTC_V], RTC_handler_linkage);
    idt[RTC_V].present = 1;
    SET_IDT_ENTRY(idt[PIT_V], pit_handler_linkage);
    idt[PIT_V].present = 1;
    SET_IDT_ENTRY(idt[MOUSE_V], mouse_handler_linkage);
    idt[MOUSE_V].present = 1; 
}


/* black_screen
 * 
 * will print to screen what exception occured 
 * Inputs: None
 * Outputs: None
 * Side Effects: will hanlde exceptions which will crash the kernel (while 1 loop)
 */
void black_screen(int vec) {
    cli();
    if(vec == vec_page_fault) {
        // move page fault addresss to screen
        asm volatile("movl %cr2, %ebx");
        register uint32_t saved_ebx asm ("ebx"); //tried to use eax, would get ATTRIB for some reason
        ATTRIB[currentTerminal] = 0x1F; // set color of text to white and background to blue (blue screen)
        clear();
        printf("\r\n\n\n\n\n\n\n                     %s\n                    Page Fault Occured At Address: 0x%#x\r", black_screen_string[vec], (uint32_t)saved_ebx);
        while(1);
    }
    ATTRIB[currentTerminal] = 0x1F; // blue screen needed colors
    clear();
    printf("\r\n\n\n\n\n\n\n                     %s\r", black_screen_string[vec]);
    // sys_halt(255); if want to do some page stuff for excr
    while(1);
}

/* EXCEPTION HANDLERS */

/* Exception handlers
 * 
 * will handle exceptions 
 * Inputs: None
 * Outputs: None
 * Side Effects: will put right exception number into function to cause balck screen
 */
void divide_error() {
    black_screen(vec_divide_error);
}

void debug() {
    black_screen(vec_debug);
}

void nmi_interrupt() {
    black_screen(vec_nmi_interrupt);
}
void breakpoint() {
    black_screen(vec_breakpoint);
}
void overflow() {
    black_screen(vec_overflow);
}
void bound_range_exceeded() {
    black_screen(vec_bound_range_exceeded);
}
void invalid_opcode() {
    black_screen(vec_invalid_opcode);
}
void device_not_available() {
    black_screen(vec_device_not_available);
}
void double_fault() {
    black_screen(vec_double_fault);
}
void coprocessor_segment_overrun() {
    black_screen(vec_coprocessor_segment_overrun);
}
void invalid_tss() {
    black_screen(vec_invalid_tss);
}
void segment_not_present() {
    black_screen(vec_segment_not_present);
}
void stack_fault() {
    black_screen(vec_stack_fault);
}
void general_protection() {
    black_screen(vec_general_protection);
}
void page_fault() {
    black_screen(vec_page_fault);
}
void x87_floating_point() {
    black_screen(vec_x87_floating_point);
}
void alignment_check() {
    black_screen(vec_alignment_check);
}
void machine_check() {
    black_screen(vec_machine_check);
}
void simd_floating_point() {
    black_screen(vec_simd_floating_point);
}






