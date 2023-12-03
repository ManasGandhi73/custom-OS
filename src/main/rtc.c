#include "rtc.h"
#include "tests.h"
#include "devices.h"

/* just for when I'm working from home, comment out otherwise
* typedef __int32 int32_t;
* typedef unsigned __int32 uint32_t;
* 
* 
* typedef __int8 uint8_t;
* typedef unsigned __int8 uint8_t;
 */

#define REAL_FREQUENCY 1024 // for virtualization, this is the real frequency that the rtc is at

int32_t frequency = 0x10; // this is the frequency we will show the user (virtualization) = 16
int32_t rtc_read_flag = 0; // flag for frequency of printing --> used in rtc_read
int rtc_tick = 0; /* the ticker for the delay with frequency, used in handler */

/* RTC_init
 * 
 * initializes RTC, allows interrupts 
 * Inputs: None
 * Outputs: None
 * Side Effects: Allows for interrupts from RTC
 */
// must turn on IRQ8: from https://wiki.osdev.org/RTC
void RTC_init() {
     /* IO Port	Access Type	    Purpose
        0x70	Read/Write	    Set Status Registers
        0x71	Read/Write	    Data
    */
    frequency = 2;          // initialize frequency to 2
    
    outb(0x8B, 0x70);		// select register B, and disable NMI
    char prev = inb(0x71);	// read the current value of register B
    outb(0x8B, 0x70);		// set the index again (a read will reset the index to register D)
    outb(prev | 0x40, 0x71);	// write the previous value ORed with 0x40. This turns on bit 6 of register B
    // printf("test");
    enable_irq(0x08);       // enables RTC interrupt (from IDT table, rtc = 0x28 -> S_PIC IRQ0)
}

/* RTC_handler
 * 
 * handles RTC interrupts, makes sure the 1s are printed at right frequency
 * Inputs: None
 * Outputs: None
 * Side Effects: Prints random stuff in screen until enter key is pressed (only when test_interrupts() is enabled)
 */
void RTC_handler() {
    /* IO Port	Access Type	    Purpose
        0x70	Read/Write	    Set Status Registers
        0x71	Read/Write	    Data
    */
    // if(test == 0) {
    //     test_interrupts(); // uncomment to test interrupts
    // }
    
    outb(0x0C, 0x70);	// select register C
    inb(0x71);	        // data port = 0x71
    send_eoi(0x08);     // send EOI to rtc on IRQ8 (S_PIC IRQ0)

    // printf("%d", 1);
    
    /* From TA who told me how to do this: */
    // every time handler is called, tick + 1
    // actual freq = 1024 hz, divide that by 
    // when rtc_tick % actual freq = 0, set flag to 1
    // freq[i] = freqeuncy showed to user
    // rtc_ticks % (actual / frequency) == 0

    rtc_tick++; /* counter */
    if(rtc_tick % (REAL_FREQUENCY / frequency) == 0){ /* virtualization, when ticks hits the right amount */
        rtc_read_flag = 0; /* turn flag off */
        rtc_tick = 0; /* reset counter to 0 */
    }
}

/* rtc_open
 * 
 * Initializes RTC frequency to 2HZ, return 0 
 * This is already done in rtc_init, so we don't need to do anything right now
 * 
 * Inputs: None
 * Outputs: 0 upon completion
 * Side Effects: None
 * 
 */
 int32_t rtc_open(const uint8_t* filename) {
    /* we don't need to do anything, frequency auto set to 2 */
    
    
    
    
    // frequency = 2;
    // int32_t rate = rtc_get_rate(frequency);

    // if(rate < 2 || rate > 15) return -1; // if rate is below 2 or over 15, 
    // rate &= 0x0F;			// rate must be above 2 and not over 15
    
    // disable_ints();         // CLI
    // outb(0x70, 0x8A);		// set index to register A, disable NMI
    // char prev=inb(0x71);	// get initial value of register A
    // outb(0x70, 0x8A);		// reset index to A
    // outb(0x71, (prev & 0xF0) | rate); //write only our rate to A. Note, rate is the bottom 4 bits.
    // enable_ints();          // STI
    frequency = 2;
    return 0;
}

/* disable_ints
 * 
 * just a cli() -- begin atomic code
 * 
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * 
 */
void disable_ints(){
    cli();
}

/* disable_ints
 * 
 * just a sti() -- end atomic code
 * 
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * 
 */
void enable_ints(){
    sti();
}

/* rtc_get_rate
 * 
 * The rate is what needs to be written to control register A in order for the frequency to be changed 
 * This is the helper function that gets that rate
 * 
 * Inputs: frequency -- the frequency we want to be at
 * Outputs: the rate value, -1 if invalid frequency
 * Side Effects: None
 */
int32_t rtc_get_rate(int32_t frequency){
    
    /*
    * formula we want: frequency =  32768 >> (rate-1);
    * frequency = 32768 * 2^(-(rate - 1))
    * frequency/(32768) = 2^(-rate) * 2^(1)
    * frequency/(2*32768) = 2^(-rate)
    * equation values put in switch case
    */

    /*
    * The rate answers to the 10 possible frequencies:
    */
    switch(frequency){
        case 2: return 15; // when frequency = 2, rate = 0x0F = 15
        case 4: return 14; // when frequency = 4, rate = 0x0E = 14
        case 8: return 13; // when frequency = 8, rate = 0x0D = 13
        case 16: return 12; // when frequency = 16, rate = 0x0C = 12 
        case 32: return 11; // when frequency = 32, rate = 0x0B = 11
        case 64: return 10; // when frequency = 64, rate = 0x0A = 10
        case 128: return 9; // when frequency = 128, rate = 0x09 = 9
        case 256: return 8; // when frequency = 256, rate = 0x08 = 8
        case 512: return 7; // when frequency = 512, rate = 0x07 = 7
        case 1024: return 6; // when frequency = 1024, rate = 0x06 = 6
    }

    return -1; // if none of those, then return -1
}

/* RTC_close
 * 
 * RTC close() does nothing
 * Inputs: fd -- file stuff, doesn't matter here
 * Outputs: 0 upon completion
 * Side Effects: None
 */
int32_t rtc_close(int32_t fd) {
    return 0; // should do nothing, just return 0 --> because the interrupts should always be on
}

/* rtc_read
 * 
 * RTC read() should block until the next interrupt, return 0
 * Inputs: none
 * Outputs: 0 upon completion
 * Side Effects: Delay for the handler, allows the user to get the right frequency
 */
int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes) { //int32_t fd, void* buf, int32_t nbytes
    rtc_read_flag = 1;          // set the flag to 1
    while(rtc_read_flag == 1){} // delay while the flag is 1
    return 0;                   // return 0 once the delay is done
}

/* rtc_write
 * 
 * RTC write() changes frequency, return 0 or -1
 * Inputs: buf -- just a buffer for the value of the frequency
 * Outputs: size of bytes written to control register A 
 * Side Effects: None
 *
 * Code from https://wiki.osdev.org/RTC
 */
int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes) {

    // frequency = *((int32_t *) buf);
    frequency = *(int32_t*)(buf); // get the buffer. Alternative without dereferencing = (int32_t) buf, TA said dereference
    // ((int32_t*) buf) or (int32_t) buf --> for trying to dereference. We have a paging error right now.

    int32_t rate = rtc_get_rate(frequency); /* get rate using helper */

    //make sure it's a power of two and between 1024 and 2 --> if not it failed
    if(rate == -1){ // if not...
        return -1; // return -1 if not a power of 2 and not between 2 and 1024
    }

    rate &= 0x0F;			            // rate must be above 2 and not over 15
    disable_ints();                     // cli
    outb(0x70, 0x8A);		            // set index to register A, disable NMI
    char prev=inb(0x71);	            // get initial value of register A
    outb(0x70, 0x8A);		            // reset index to A
    outb(0x71, (prev & 0xF0) | rate);   //write only our rate to A. Note, rate is the bottom 4 bits.
    // printf("%d", rate);              // for testing
    enable_ints();                      // sti

    return nbytes; //return the size of bytes written
}

