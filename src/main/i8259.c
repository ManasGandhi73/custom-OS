/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t cached_primary_mask; /* IRQs 0-7  */
uint8_t cached_secondary_mask;  /* IRQs 8-15 */


 


/* i8259_init
 * 
 * initializes PIC, allows interrupts 
 * Inputs: None
 * Outputs: None
 * Side Effects: sets up PIC, allows for device interrutps to occur
 */
/* Initialize the 8259 PIC */
void i8259_init(void) {
    // From PowerPoint slides in lecture 10:
    // Don't include lock for now

    // mask PICS
    // Use 0xFF -> 1111 1111 for any bit set 
    cached_primary_mask = MASK_8_INT;
    cached_secondary_mask = MASK_8_INT;
    outb(MASK_8_INT, PIC_PRIMARY_IMR);    
    outb(MASK_8_INT, PIC_SECONDARY_IMR);

    // Select Primary PIC for intitialization:
    outb(ICW1, PRIMARY_8259_PORT);

    // Primary offset
    // maps IR0->IR7 to 0x20->0x27
    outb(ICW2_PRIMARY, PIC_PRIMARY_IMR);

    // primary has secondary on IR2 (0000 0100)
    // bit mask of what bit lines have secondary on them
    outb(ICW3_PRIMARY, PIC_PRIMARY_IMR);
    // Set mode for pri PIC (enables PIC)
    outb(ICW4, PIC_PRIMARY_IMR);

    // Same for Secondary:

    // Select Secondary PIC for initialization
    outb(ICW1, SECONDARY_8259_PORT);
    // Secondary offset
    // maps IR0->IR7 to 0x28->0x2F
    outb(ICW2_SECONDARY, PIC_SECONDARY_IMR);

    // Tell secondary it's a secondary
    outb(ICW3_SECONDARY, PIC_SECONDARY_IMR);
    
    // Set mode for sec PIC (enables PIC)
    outb(ICW4, PIC_SECONDARY_IMR);  
  
    // restore masks
    outb(cached_primary_mask, PIC_PRIMARY_IMR);
    outb(cached_secondary_mask, PIC_SECONDARY_IMR);

    // allow int to occur from IR2 of primary PIC
    enable_irq(0x0002);


}

// base idea from https://wiki.osdev.org/8259_PIC and lecture 10 slides:




/* enable_IRQ
 * 
 * Enable (unmask) the specified IRQ 
 * Inputs: None
 * Outputs: None
 * Side Effects: enables interrupts from passed IRQ_num
 */
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    if(irq_num & 0x08) {
        // Is from Secondary (since 28->xF)
        irq_num -=0x08;
        // cache mask
        // Set  the bit corresponding to the IRQ number to 0, enabling interrupts on that IRQ
        cached_secondary_mask = cached_secondary_mask & ~(0x01 << irq_num);
        outb(cached_secondary_mask, PIC_SECONDARY_IMR);
    }
    else {
        // Is primary PIC
        // Set  the bit corresponding to the IRQ number to 0, enabling interrupts on that IRQ
        cached_primary_mask = cached_primary_mask & ~(0x01 << irq_num);
        outb(cached_primary_mask, PIC_PRIMARY_IMR);
    }
}


/* diable_IRQ
 * 
 * Disable (mask) the specified IRQ 
 * Inputs: None
 * Outputs: None
 * Side Effects: disables interrupts from passed IRQ_num
 */
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    if(irq_num & 0x08) {
        // Is from Secondary (since x8->xF)
        irq_num -=0x08;
        // cache mask
        // Set bit to 1 that is the corresponding to the IRQ number
        cached_secondary_mask = cached_secondary_mask | (0x01 << irq_num);
        outb(cached_secondary_mask, PIC_SECONDARY_IMR);
    }
    else {
        // Is primary PIC
        // Set bit to 1 that is the corresponding to the IRQ number
        cached_primary_mask = cached_primary_mask | (0x01 << irq_num);
        outb(cached_primary_mask, PIC_PRIMARY_IMR);
    }
}




/* send_EOI
 * 
 * Sends end of interrupt signal to PIC
 * Inputs: None
 * Outputs: None
 * Side Effects: Enables interrupts from devices
 */
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    // info in header file above #define EOI
    // 8 b/c sec is iRQ 8-F
    if(irq_num & 0x08) {
        // Is from Secondary (since x28->x2F)
        // must end interrupt at secondary pic IR# and primary pic IR2
        irq_num -= 0x08;
        outb((EOI | irq_num), SECONDARY_8259_PORT);
        outb((EOI | 0x02), PRIMARY_8259_PORT);
    }
    else {
        // just EOI primary PIC
        outb((EOI | irq_num), PRIMARY_8259_PORT);
    }
}
