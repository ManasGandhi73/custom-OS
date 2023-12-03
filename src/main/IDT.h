#include "x86_desc.h"
#include "lib.h"
#include "IDT_handler.h"

// initializes idt
extern void init_idt();

// enum for handler name to right idt number
enum handlers {
    vec_divide_error = 0, // set enum to start at 0
    vec_debug,
    vec_nmi_interrupt,
    vec_breakpoint,
    vec_overflow, 
    vec_bound_range_exceeded,
    vec_invalid_opcode,
    vec_device_not_available,
    vec_double_fault,
    vec_coprocessor_segment_overrun,
    vec_invalid_tss,
    vec_segment_not_present,
    vec_stack_fault,
    vec_general_protection,
    vec_page_fault,
    vec_x87_floating_point = 16,    // skip excetion 15, as it doesn't exist
    vec_alignment_check, 
    vec_machine_check,
    vec_simd_floating_point
    


};

// Handler functions for all intel exceptions
void divide_error();
void debug();
void nmi_interrupt();
void breakpoint();
void overflow();
void bound_range_exceeded();
void invalid_opcode();
void device_not_available();
void double_fault();
void coprocessor_segment_overrun();
void invalid_tss();
void segment_not_present();
void stack_fault();
void general_protection();
void page_fault();
void x87_floating_point();
void alignment_check();
void machine_check();
void simd_floating_point();

// (technically now a blue screen)
void black_screen(int vec);

// black_screen_string holds strings that indicate different interrupts
// to be displayed on the black (blue for our purposes) interrupt screen
static const char black_screen_string[21][50] = { // 21 exceptions, 50 for long enough string
    "Interrupt 0: Divide Error Exception", 
    "Interrupt 1: Debug Exception", 
    "Interrupt 2: NMI Interrupt", 
    "Interrupt 3: Breakpoint Exception",
    "Interrupt 4: Overflow Exception",
    "Interrupt 5: BOUND Range Exceeded Exception",
    "Interrupt 6: Invalid Opcode Exception",
    "Interrupt 7: Device Not Available Exception",
    "Interrupt 8: Double Fault Exception",
    "Interrupt 9: Coprocessor Segment Overrun",
    "Interrupt 10: Invalid TSS Exception",
    "Interrupt 11: Segment Not Present",
    "Interrupt 12: Stack Fault Exception",
    "Interrupt 13: General Protection Exception",
    "Interrupt 14: Page-Fault Exception",
    "will never reach",
    "Interrupt 16: x87 FPU Floating-Point Error",
    "Interrupt 17: Alignment Check Exception",
    "Interrupt 18: Machine-Check Exception",
    "Interrupt 19: SIMD Floating-Point Exception",
};
