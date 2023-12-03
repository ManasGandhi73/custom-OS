/* i8259.h - Defines used in interactions with the 8259 interrupt
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _I8259_H 
#define _I8259_H

#include "types.h"
#include "lib.h"

/* Ports that each PIC sits on */
#define PRIMARY_8259_PORT    0x20
#define SECONDARY_8259_PORT     0xA0

/* Initialization control words to init each PIC.
 * See the Intel manuals for details on the meaning
 * of each word */
#define ICW1                0x11
#define ICW2_PRIMARY        0x20
#define ICW2_SECONDARY      0x28
#define ICW3_PRIMARY        0x04
#define ICW3_SECONDARY      0x02
#define ICW4                0x01

// My definitions
// From https://wiki.osdev.org/8259_PIC
// The PIC has an internal register called the IMR, or the Interrupt Mask Register. 
// It is 8 bits wide. This register is a bitmap of the request lines going into the PIC. 
// When a bit is set, the PIC ignores the request and continues normal operation. 
#define PIC_PRIMARY_IMR     0x21
#define PIC_SECONDARY_IMR   0xA1

#define MASK_8_INT          0xFF


/* End-of-interrupt byte.  This gets OR'd with
 * the interrupt number and sent out to the PIC
 * to declare the interrupt finished */
#define EOI                 0x60

/* Externally-visible functions */

/* Initialize both PICs */
void i8259_init(void);
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num);
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num);
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num);

#endif /* _I8259_H */
