#ifndef RTC_H
#define RTC_H

#include "types.h"
#include "lib.h"
#include "i8259.h" 

#define RTC_ADDRESS_PORT 0x70 /* address port */
#define RTC_DATA_PORT 0x71 /* data port */

/* initializes rtc to 1024 Hz*/
void RTC_init();

/* handler for rtc */
void RTC_handler();

/* blocks until the next interrupt */
int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes); //int32_t fd, void* buf, int32_t nbytes

/* changes frequency */
int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes); //int32_t fd, const void* buf, int32_t nbytes

/* initializes frequency, does nothing here */
int32_t rtc_open (const uint8_t* filename); //const uint8_t* filename

/* does nothing */
int32_t rtc_close (int32_t fd);

/* helpers */

/* gets the rate based on frequency */
int32_t rtc_get_rate(int32_t frequency);

/* CLI */
void disable_ints();

/* STI */
void enable_ints();

#endif /* RTC_H */
