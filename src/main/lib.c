/* lib.c - Some basic library functions (printf, strlen, etc.)
 * vim:ts=4 noexpandtab */

#include "lib.h"
#include "filesys.h"
#include "mouse.h"

#define STAT_LEFT   18
#define STAT_RIGHT  56
#define STAT_PID    23
#define STAT_TERM   9

#define NUM_TERMS   3


static int screen_x[NUM_TERMS]; 
static int screen_y[NUM_TERMS];
static char* video_mem = (char *)VIDEO;
int term0X = 0; 
int term0Y = 0; 
int term1X = 0; 
int term1Y = 0; 
int term2X = 0; 
int term2Y = 0; 

int pcbTerm0 = 0; 
int pcbTerm1 = 0; 
int pcbTerm2 = 0; 


/* void clear(void);
 * Inputs: void
 * Return Value: none
 * Function: Clears video memory */
void clear(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB[currentTerminal];
    }
}

void clear_all(void) {
    map(VIDEO, VIDEOT1, _4kBPAGE);
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = 0x0C;
    }

    map(VIDEO, VIDEOT2, _4kBPAGE);
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = 0x0C;
    }

    map(VIDEO, VIDEOT3, _4kBPAGE);
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = 0x0C;
    }

    map(VIDEO, VIDEO, _4kBPAGE);
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = 0x0C;
    }
}

/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output. */
int32_t printf(int8_t *format, ...) {

    /* Pointer to the format string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            putc('%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    puts(conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    puts(&conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                int8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    itoa(-value, &conv_buf[1], 10);
                                } else {
                                    itoa(value, conv_buf, 10);
                                }
                                puts(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            putc((uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            puts(*((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                putc(*buf);
                break;
        }
        buf++;
    }
    return (buf - format);
}


int32_t printf_kb(int8_t *format, ...) {

    /* Pointer to the format string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            putc_kb('%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    puts_kb(conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    puts_kb(&conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                puts_kb(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                int8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    itoa(-value, &conv_buf[1], 10);
                                } else {
                                    itoa(value, conv_buf, 10);
                                }
                                puts_kb(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            putc_kb((uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            puts_kb(*((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                putc_kb(*buf);
                break;
        }
        buf++;
    }
    return (buf - format);
}



int32_t puts_kb(int8_t* s) {
    register int32_t index = 0;
    while (s[index] != '\0') {
        putc_kb(s[index]);
        index++;
    }
    return index;
}

/* int32_t puts(int8_t* s);
 *   Inputs: int_8* s = pointer to a string of characters
 *   Return Value: Number of bytes written
 *    Function: Output a string to the console 
 *               will scroll screen*/
int32_t puts(int8_t* s) {
    register int32_t index = 0;
    while (s[index] != '\0') {
        putc(s[index]);
        index++;
    }
    return index;
}

/* void putc(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 *  Function: Output a character to the console depending on current program*/
void putc(uint8_t c) {

    // if screenx and screeny out of bounds, scroll up
    if((screen_x[pcb_array[pcbIndex]->terminalNum] == 0 && screen_y[pcb_array[pcbIndex]->terminalNum] == NUM_ROWS) || (screen_y[pcb_array[pcbIndex]->terminalNum] == NUM_ROWS - 1 && c == '\n')) {
        // Need Scolling
        int i, j;
        for(i = 0; i < NUM_ROWS - 1; i++) {
            for(j = 0; j < NUM_COLS; j++) {
                *(uint8_t *)(video_mem + ((NUM_COLS * (i) + j) << 1)) = *(uint8_t *)(video_mem + ((NUM_COLS * (i + 1) + j) << 1));
                *(uint8_t *)(video_mem + ((NUM_COLS * (i) + j) << 1) + 1) = (*(uint8_t *)(video_mem + ((NUM_COLS * (i) + j) << 1) + 1) & 0xF0) + (*(uint8_t *)(video_mem + ((NUM_COLS * (i + 1) + j) << 1) + 1)& 0x0F);

            }
        }
        for(j = 0; j < NUM_COLS; j++) {
            *(uint8_t *)(video_mem + ((NUM_COLS * (NUM_ROWS - 1) + j) << 1)) = ' ';
        }
        screen_x[pcb_array[pcbIndex]->terminalNum] = 0;
        screen_y[pcb_array[pcbIndex]->terminalNum] = NUM_ROWS - 1;

        // if enter was not pressed, need to actually put a character
        if(c != '\n') {
            *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[pcb_array[pcbIndex]->terminalNum] + screen_x[pcb_array[pcbIndex]->terminalNum]) << 1)) = c;
            *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[pcb_array[pcbIndex]->terminalNum] + screen_x[pcb_array[pcbIndex]->terminalNum]) << 1) + 1) = ATTRIB[pcb_array[pcbIndex]->terminalNum];
            screen_x[pcb_array[pcbIndex]->terminalNum]++;
            screen_y[pcb_array[pcbIndex]->terminalNum] = (screen_y[pcb_array[pcbIndex]->terminalNum] + (screen_x[pcb_array[pcbIndex]->terminalNum] / NUM_COLS));
            screen_x[pcb_array[pcbIndex]->terminalNum] %= NUM_COLS;
        }
    }


    else if(c == '\n') {
        screen_y[pcb_array[pcbIndex]->terminalNum]++;
        screen_x[pcb_array[pcbIndex]->terminalNum] = 0;
    } 

    // set backslash r to set cursor to top left of terminal
    else if ( c == '\r') {
        screen_y[pcb_array[pcbIndex]->terminalNum] = 0;
        screen_x[pcb_array[pcbIndex]->terminalNum] = 0;
    }

    // if end of tab, do nothing
    else if(c == '\t') {

    }

    // if end of autofill, do nothing
    else if(c == '\v') {

    }


    // if backspace
    else if (c == '\b') {
        // if at start of screen, do nothing
        if(screen_x[pcb_array[pcbIndex]->terminalNum] == 0 && screen_y[pcb_array[pcbIndex]->terminalNum] == 0) {
            return;
        }
        
        // go up a line to right side if at start of current line
        if(screen_x[pcb_array[pcbIndex]->terminalNum] == 0) {
            screen_y[pcb_array[pcbIndex]->terminalNum]--;
            screen_x[pcb_array[pcbIndex]->terminalNum] = NUM_COLS - 1;
            *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[pcb_array[pcbIndex]->terminalNum] + screen_x[pcb_array[pcbIndex]->terminalNum]) << 1) - 1) = ATTRIB[pcb_array[pcbIndex]->terminalNum];
            *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[pcb_array[pcbIndex]->terminalNum] + screen_x[pcb_array[pcbIndex]->terminalNum]) << 1)) = ' ';
        }
        // just put space
        else {
            *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[pcb_array[pcbIndex]->terminalNum] + screen_x[pcb_array[pcbIndex]->terminalNum]) << 1) - 1) = ATTRIB[pcb_array[pcbIndex]->terminalNum];
            screen_x[pcb_array[pcbIndex]->terminalNum]--;
            *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[pcb_array[pcbIndex]->terminalNum] + screen_x[pcb_array[pcbIndex]->terminalNum]) << 1)) = ' ';
        }
    }

    // put character on screen
    else {
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[pcb_array[pcbIndex]->terminalNum] + screen_x[pcb_array[pcbIndex]->terminalNum]) << 1)) = c;
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[pcb_array[pcbIndex]->terminalNum] + screen_x[pcb_array[pcbIndex]->terminalNum]) << 1) + 1) = ATTRIB[pcb_array[pcbIndex]->terminalNum];
        screen_x[pcb_array[pcbIndex]->terminalNum]++;
        screen_y[pcb_array[pcbIndex]->terminalNum] = (screen_y[pcb_array[pcbIndex]->terminalNum] + (screen_x[pcb_array[pcbIndex]->terminalNum] / NUM_COLS));
        screen_x[pcb_array[pcbIndex]->terminalNum] %= NUM_COLS;
    }

    // update cursor
    update_cursor(screen_x[currentTerminal], screen_y[currentTerminal]);
    *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[currentTerminal] + screen_x[currentTerminal]) << 1) + 1) = ATTRIB[pcb_array[pcbIndex]->terminalNum]; // for cursor color


    // 
    if(pcb_array[pcbIndex]->terminalNum == 0 && pcb_array[pcbIndex]->terminalNum != currentTerminal) {
        term0X = screen_x[pcb_array[pcbIndex]->terminalNum];
        term0Y = screen_y[pcb_array[pcbIndex]->terminalNum];
    }
    if(pcb_array[pcbIndex]->terminalNum == 1 && pcb_array[pcbIndex]->terminalNum != currentTerminal) {
        term1X = screen_x[pcb_array[pcbIndex]->terminalNum];
        term1Y = screen_y[pcb_array[pcbIndex]->terminalNum];
    }
    if(pcb_array[pcbIndex]->terminalNum == 2 && pcb_array[pcbIndex]->terminalNum != currentTerminal) {
        term2X = screen_x[pcb_array[pcbIndex]->terminalNum];
        term2Y = screen_y[pcb_array[pcbIndex]->terminalNum];
    }
}


/* void putc_kb(uint8_t c);
 * Inputs: uint_8* c = character to print
 * Return Value: void
 *  Function: Output a character to the console depending on current terminal*/
void putc_kb(uint8_t c) {
    if((screen_x[currentTerminal] == 0 && screen_y[currentTerminal] == NUM_ROWS) || (screen_y[currentTerminal] == NUM_ROWS - 1 && c == '\n')) {
        // Need Scolling
        int i, j;
        for(i = 0; i < NUM_ROWS - 1; i++) {
            for(j = 0; j < NUM_COLS; j++) {
                *(uint8_t *)(video_mem + ((NUM_COLS * (i) + j) << 1)) = *(uint8_t *)(video_mem + ((NUM_COLS * (i + 1) + j) << 1));
                *(uint8_t *)(video_mem + ((NUM_COLS * (i) + j) << 1) + 1) = (*(uint8_t *)(video_mem + ((NUM_COLS * (i) + j) << 1) + 1) & 0xF0) + (*(uint8_t *)(video_mem + ((NUM_COLS * (i + 1) + j) << 1) + 1)& 0x0F);

            }
        }
        for(j = 0; j < NUM_COLS; j++) {
            *(uint8_t *)(video_mem + ((NUM_COLS * (NUM_ROWS - 1) + j) << 1)) = ' ';
        }
        screen_x[currentTerminal] = 0;
        screen_y[currentTerminal] = NUM_ROWS - 1;
        if(c != '\n') {
            *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[currentTerminal] + screen_x[currentTerminal]) << 1)) = c;
            *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[currentTerminal] + screen_x[currentTerminal]) << 1) + 1) = ATTRIB[currentTerminal];
            screen_x[currentTerminal]++;
            screen_y[currentTerminal] = (screen_y[currentTerminal] + (screen_x[currentTerminal] / NUM_COLS));
            screen_x[currentTerminal] %= NUM_COLS;
        }
    }

    else if(c == '\n') {
        screen_y[currentTerminal]++;
        screen_x[currentTerminal] = 0;
    } 
    // set backslash r to set cursor to top left of terminal
    else if ( c == '\r') {
        screen_y[currentTerminal] = 0;
        screen_x[currentTerminal] = 0;
    }

    else if(c == '\t') {

    }
    else if(c == '\v') {

    }

    else if (c == '\b') {
        if(screen_x[currentTerminal] == 0 && screen_y[currentTerminal] == 0) {
            return;
        }
        
        if(screen_x[currentTerminal] == 0) {
            screen_y[currentTerminal]--;
            screen_x[currentTerminal] = NUM_COLS - 1;
            *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[currentTerminal] + screen_x[currentTerminal]) << 1) - 1) = ATTRIB[currentTerminal];
            *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[currentTerminal] + screen_x[currentTerminal]) << 1)) = ' ';
        }
        else {
            *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[currentTerminal] + screen_x[currentTerminal]) << 1) - 1) = ATTRIB[currentTerminal];
            screen_x[currentTerminal]--;
            *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[currentTerminal] + screen_x[currentTerminal]) << 1)) = ' ';
        }
    }

    else {
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[currentTerminal] + screen_x[currentTerminal]) << 1)) = c;
        *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[currentTerminal] + screen_x[currentTerminal]) << 1) + 1) = ATTRIB[currentTerminal];
        screen_x[currentTerminal]++;
        screen_y[currentTerminal] = (screen_y[currentTerminal] + (screen_x[currentTerminal] / NUM_COLS));
        screen_x[currentTerminal] %= NUM_COLS;
    }
    update_cursor(screen_x[currentTerminal], screen_y[currentTerminal]);
    *(uint8_t *)(video_mem + ((NUM_COLS * screen_y[currentTerminal] + screen_x[currentTerminal]) << 1) + 1) = ATTRIB[currentTerminal]; // for cursor color

   
}

/* int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix);
 * Inputs: uint32_t value = number to convert
 *            int8_t* buf = allocated buffer to place string in
 *          int32_t radix = base system. hex, oct, dec, etc.
 * Return Value: number of bytes written
 * Function: Convert a number to its ASCII representation, with base "radix" */
int8_t* itoa(uint32_t value, int8_t* buf, int32_t radix) {
    static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int8_t *newbuf = buf;
    int32_t i;
    uint32_t newval = value;

    /* Special case for zero */
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return buf;
    }

    /* Go through the number one place value at a time, and add the
     * correct digit to "newbuf".  We actually add characters to the
     * ASCII string from lowest place value to highest, which is the
     * opposite of how the number should be printed.  We'll reverse the
     * characters later. */
    while (newval > 0) {
        i = newval % radix;
        *newbuf = lookup[i];
        newbuf++;
        newval /= radix;
    }

    /* Add a terminating NULL */
    *newbuf = '\0';

    /* Reverse the string and return */
    return strrev(buf);
}

/* int8_t* strrev(int8_t* s);
 * Inputs: int8_t* s = string to reverse
 * Return Value: reversed string
 * Function: reverses a string s */
int8_t* strrev(int8_t* s) {
    register int8_t tmp;
    register int32_t beg = 0;
    register int32_t end = strlen(s) - 1;

    while (beg < end) {
        tmp = s[end];
        s[end] = s[beg];
        s[beg] = tmp;
        beg++;
        end--;
    }
    return s;
}

/* uint32_t strlen(const int8_t* s);
 * Inputs: const int8_t* s = string to take length of
 * Return Value: length of string s
 * Function: return length of string s */
uint32_t strlen(const int8_t* s) {
    register uint32_t len = 0;
    while (s[len] != '\0')
        len++;
    return len;
}

/* void* memset(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive bytes of pointer s to value c */
void* memset(void* s, int32_t c, uint32_t n) {
    c &= 0xFF;
    asm volatile ("                 \n\
            .memset_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memset_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memset_aligned \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memset_top     \n\
            .memset_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     stosl           \n\
            .memset_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memset_done    \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            subl    $1, %%edx       \n\
            jmp     .memset_bottom  \n\
            .memset_done:           \n\
            "
            :
            : "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_word(void* s, int32_t c, uint32_t n);
 * Description: Optimized memset_word
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set lower 16 bits of n consecutive memory locations of pointer s to value c */
void* memset_word(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosw           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memset_dword(void* s, int32_t c, uint32_t n);
 * Inputs:    void* s = pointer to memory
 *          int32_t c = value to set memory to
 *         uint32_t n = number of bytes to set
 * Return Value: new string
 * Function: set n consecutive memory locations of pointer s to value c */
void* memset_dword(void* s, int32_t c, uint32_t n) {
    asm volatile ("                 \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            cld                     \n\
            rep     stosl           \n\
            "
            :
            : "a"(c), "D"(s), "c"(n)
            : "edx", "memory", "cc"
    );
    return s;
}

/* void* memcpy(void* dest, const void* src, uint32_t n);
 * Inputs:      void* dest = destination of copy
 *         const void* src = source of copy
 *              uint32_t n = number of byets to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of src to dest */
void* memcpy(void* dest, const void* src, uint32_t n) {
    asm volatile ("                 \n\
            .memcpy_top:            \n\
            testl   %%ecx, %%ecx    \n\
            jz      .memcpy_done    \n\
            testl   $0x3, %%edi     \n\
            jz      .memcpy_aligned \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%ecx       \n\
            jmp     .memcpy_top     \n\
            .memcpy_aligned:        \n\
            movw    %%ds, %%dx      \n\
            movw    %%dx, %%es      \n\
            movl    %%ecx, %%edx    \n\
            shrl    $2, %%ecx       \n\
            andl    $0x3, %%edx     \n\
            cld                     \n\
            rep     movsl           \n\
            .memcpy_bottom:         \n\
            testl   %%edx, %%edx    \n\
            jz      .memcpy_done    \n\
            movb    (%%esi), %%al   \n\
            movb    %%al, (%%edi)   \n\
            addl    $1, %%edi       \n\
            addl    $1, %%esi       \n\
            subl    $1, %%edx       \n\
            jmp     .memcpy_bottom  \n\
            .memcpy_done:           \n\
            "
            :
            : "S"(src), "D"(dest), "c"(n)
            : "eax", "edx", "memory", "cc"
    );
    return dest;
}

/* void* memmove(void* dest, const void* src, uint32_t n);
 * Description: Optimized memmove (used for overlapping memory areas)
 * Inputs:      void* dest = destination of move
 *         const void* src = source of move
 *              uint32_t n = number of byets to move
 * Return Value: pointer to dest
 * Function: move n bytes of src to dest */
void* memmove(void* dest, const void* src, uint32_t n) {
    asm volatile ("                             \n\
            movw    %%ds, %%dx                  \n\
            movw    %%dx, %%es                  \n\
            cld                                 \n\
            cmp     %%edi, %%esi                \n\
            jae     .memmove_go                 \n\
            leal    -1(%%esi, %%ecx), %%esi     \n\
            leal    -1(%%edi, %%ecx), %%edi     \n\
            std                                 \n\
            .memmove_go:                        \n\
            rep     movsb                       \n\
            "
            :
            : "D"(dest), "S"(src), "c"(n)
            : "edx", "memory", "cc"
    );
    return dest;
}

/* int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n)
 * Inputs: const int8_t* s1 = first string to compare
 *         const int8_t* s2 = second string to compare
 *               uint32_t n = number of bytes to compare
 * Return Value: A zero value indicates that the characters compared
 *               in both strings form the same string.
 *               A value greater than zero indicates that the first
 *               character that does not match has a greater value
 *               in str1 than in str2; And a value less than zero
 *               indicates the opposite.
 * Function: compares string 1 and string 2 for equality */
int32_t strncmp(const int8_t* s1, const int8_t* s2, uint32_t n) {
    int32_t i;
    for (i = 0; i < n; i++) {
        if ((s1[i] != s2[i]) || (s1[i] == '\0') /* || s2[i] == '\0' */) {

            /* The s2[i] == '\0' is unnecessary because of the short-circuit
             * semantics of 'if' expressions in C.  If the first expression
             * (s1[i] != s2[i]) evaluates to false, that is, if s1[i] ==
             * s2[i], then we only need to test either s1[i] or s2[i] for
             * '\0', since we know they are equal. */
            return s1[i] - s2[i];
        }
    }
    return 0;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 * Return Value: pointer to dest
 * Function: copy the source string into the destination string */
int8_t* strcpy(int8_t* dest, const int8_t* src) {
    int32_t i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

/* int8_t* strcpy(int8_t* dest, const int8_t* src, uint32_t n)
 * Inputs:      int8_t* dest = destination string of copy
 *         const int8_t* src = source string of copy
 *                uint32_t n = number of bytes to copy
 * Return Value: pointer to dest
 * Function: copy n bytes of the source string into the destination string */
int8_t* strncpy(int8_t* dest, const int8_t* src, uint32_t n) {
    int32_t i = 0;
    while (src[i] != '\0' && i < n) {
        dest[i] = src[i];
        i++;
    }
    while (i < n) {
        dest[i] = '\0';
        i++;
    }
    return dest;
}

/* void test_interrupts(void)
 * Inputs: void
 * Return Value: void
 * Function: increments video memory. To be used to test rtc */
void test_interrupts(void) {
    int32_t i;
    for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
        video_mem[i << 1]++;
    }
}

// Code from https://wiki.osdev.org/Text_Mode_Cursor

/* 
 * enable_cursor
 *   DESCRIPTION: enables blinking cursor
 *   INPUTS: 
 *           start and end of where want blinker to blink (usually 14 and 15 for bottom)
 *   OUTPUTS: none
 *   RETURN VALUE: -1 (write not enables)
 *   SIDE EFFECTS: 
 */
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
    // 3d4 and 3d5 are read and write ports respecitvely for cursor
    // A = low cursor shape register
	outb(0x0A, 0x3D4);
    // C0 bit mask 1100 0000 with port 
	outb((inb(0x3D5) & 0xC0) | cursor_start, 0x3D5);
    // B = high cursor shape registe
	outb(0x0B, 0x3D4);
    // E0 bit mask 1110 0000 
	outb((inb(0x3D5) & 0xE0) | cursor_end, 0x3D5);
}

/* 
 * update_cursor
 *   DESCRIPTION: moves cursor
 *   INPUTS: 
 *           start and end of where want cursor to blink (usually 14 and 15 for bottom)
 *   OUTPUTS: none
 *   RETURN VALUE: 
 *   SIDE EFFECTS: 
 */
void update_cursor(int x, int y) {
	uint16_t pos = y * VGA_WIDTH + x;
    // 3d4 and 3d5 are read and write ports respecitvely for cursor
    
    // F register is low byte of position
	outb(0x0F, 0x3D4);
    // FF to only write low 8 bits
	outb((uint8_t) (pos & 0xFF), 0x3D5);

    // E register is high byte of position
	outb(0x0E, 0x3D4);
    // 8) & 0xFF to only write high 8 bits of position
	outb((uint8_t) ((pos >> 8) & 0xFF), 0x3D5);
}

/* 
 * terminalSwitch
 *   DESCRIPTION: enables multiple terminal windows by interchanging memory
 *   INPUTS: 
 *           base terminal, target terminal
 *   OUTPUTS: none
 *   RETURN VALUE: 
 *   SIDE EFFECTS: Switches terminal window
 */
void terminalSwitch(int initialTerm, int nextTerm)
{   cli();
    // USE NUMROWS +1  to save status bar as well
    if(initialTerm == 0)
    {
        memcpy((void*)VIDEOT1, (void*)VIDEO, (NUM_ROWS+1)*NUM_COLS*2); //now the VIDEOT1 should hold the terminals old characters 
        term0X = screen_x[initialTerm]; 
        term0Y = screen_y[initialTerm]; 
    }
    if(initialTerm == 1)
    {
        memcpy((void*)VIDEOT2, (void*)VIDEO, (NUM_ROWS+1)*NUM_COLS*2); //now the VIDEOT1 should hold the terminals old characters 
        term1X = screen_x[initialTerm]; 
        term1Y = screen_y[initialTerm];
    }
    if(initialTerm == 2)
    {
        memcpy((void*)VIDEOT3, (void*)VIDEO, (NUM_ROWS+1)*NUM_COLS*2); //now the VIDEOT1 should hold the terminals old characters 
        term2X = screen_x[initialTerm]; 
        term2Y = screen_y[initialTerm]; 
    }

    clear(); 

    if(nextTerm == 0)
    {
        screen_x[nextTerm] = term0X; 
        screen_y[nextTerm] = term0Y; 
        // pcbIndex = pcbTerm0; 
        // ATTRIB[currentTerminal] = T1_COLOR; 
        update_cursor(screen_x[nextTerm], screen_y[nextTerm]); 
        memmove((void*)VIDEO, (void*)VIDEOT1, NUM_COLS*(NUM_ROWS+1)*2); 
    }
    if(nextTerm == 1)
    {
        screen_x[nextTerm] = term1X; 
        screen_y[nextTerm] = term1Y; 
        // pcbIndex = pcbTerm1; 
        // ATTRIB[currentTerminal] = T2_COLOR; 
        update_cursor(screen_x[nextTerm], screen_y[nextTerm]); 
        memmove((void*)VIDEO, (void*)VIDEOT2, NUM_COLS*(NUM_ROWS+1)*2); 
    }
    if(nextTerm == 2)
    {
        screen_x[nextTerm] = term2X; 
        screen_y[nextTerm] = term2Y; 
        // pcbIndex = pcbTerm2; 
        // ATTRIB[currentTerminal] = T3_COLOR; 
        update_cursor(screen_x[nextTerm], screen_y[nextTerm]); 
        memmove((void*)VIDEO, (void*)VIDEOT3, NUM_COLS*(NUM_ROWS+1)*2); 
    }

    status_bar_init();
}




/* 
 *   status_bar_init
 *   DESCRIPTION: enables blinking cursor
 *   INPUTS: 
 *           start and end of where want blinker to blink (usually 14 and 15 for bottom)
 *   OUTPUTS: none
 *   RETURN VALUE: -1 (write not enables)
 *   SIDE EFFECTS: 
 */
void status_bar_init() {
    uint8_t sb_idx;

    // find pcb of current process running on terminal
    int term_pcb;
    for(term_pcb = 0; term_pcb < PCBNUM; term_pcb++) {
        if(pcb_array[term_pcb]->terminalNum == currentTerminal && !pcb_array[term_pcb]->has_child) {
            break;
        }
    }
    // if current term pcb exists
    if(term_pcb != PCBNUM) {

        // print status bar depending on terminal, process id, and name of exeutable running
        int8_t status_bar[NUM_COLS] = "Terminal    -  Process    --  Running: ";
        switch(currentTerminal){
            case 0:
                status_bar[STAT_TERM] = '1';
                break;
            case 1:
                status_bar[STAT_TERM] = '2';
                break;
            case 2:
                status_bar[STAT_TERM] = '3';
                break;
        }

        switch(pcb_array[term_pcb]->pid) {
            case 0:
                status_bar[STAT_PID] = '1';
                break;
            case 1:
                status_bar[STAT_PID] = '2';
                break;
            case 2:
                status_bar[STAT_PID] = '3';
                break;
            case 3:
                status_bar[STAT_PID] = '4';
                break;
            case 4:
                status_bar[STAT_PID] = '5';
                break;
            case 5:
                status_bar[STAT_PID] = '6';
                break;

        }
        
        // write status bar array to Video memory below the total Num rows as to not scroll with process
        for(sb_idx = 0; sb_idx < NUM_COLS; sb_idx++) {
            *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1) + 1) = (ATTRIB[currentTerminal] << 4) + (ATTRIB[currentTerminal] >> 4); // right shift by 4 to paint background color of video 
            *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = ' '; 
            if(sb_idx >= STAT_LEFT && sb_idx <= STAT_RIGHT)    
                *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = status_bar[sb_idx-STAT_LEFT]; 
            
        }
        // write name of process to status bar
        int i = 0;
        for(sb_idx = STAT_RIGHT+1; sb_idx < STAT_RIGHT+1 + strlen(pcb_array[term_pcb]->name); sb_idx++, i++) {
            *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = pcb_array[term_pcb]->name[i];
        }

    }
}

int8_t paint_name[14][15] = {{"Dark Blue\0"}, {"Dark Green\0"}, {"Light Blue\0"},{"Dark Red\0"},{"Dark Pink\0"},{"Dark Orange\0"},{"Grey\0"},{"Dark grey\0"},{"Purple-Blue\0"},{"Light Green\0"},{"Vibrant Blue\0"},{"Orange\0"},{"Pink\0"},{"Yellow\0"}};
/* STATUS BARS BASED ON GUI */

void status_bar_paint() {
    uint8_t sb_idx;
    for(sb_idx = 0; sb_idx < NUM_COLS; sb_idx++) {
         *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1) + 1) = (ATTRIB[currentTerminal] << 4) + (ATTRIB[currentTerminal] >> 4); // right shift by 4 to paint background color of video 
        *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = ' '; 
  
    }

    int i = 0;
    int8_t pingpong[80] = "PAINT | Press ESC to quit | painting: \0";
    for(sb_idx = 27; sb_idx < 27 + strlen(pingpong); sb_idx++, i++) {
        *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = pingpong[i];
    }
    
    i = 0;
    for(; sb_idx < 27 + strlen(pingpong) + strlen(paint_name[paint_color-1]); sb_idx++, i++){
        *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1) + 1) = paint_color;
         *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = paint_name[paint_color-1][i]; 
    }

    

    

    int8_t change[80] = "Right Click to Change Color\0";
    i = 0;
    for(sb_idx = 5; sb_idx < 5 + strlen(change); sb_idx++, i++) {
        *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = change[i];
    }




}


void status_bar_pingpong() {
    uint8_t sb_idx;
    for(sb_idx = 0; sb_idx < NUM_COLS; sb_idx++) {
         *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1) + 1) = (ATTRIB[currentTerminal] << 4) + (ATTRIB[currentTerminal] >> 4); // right shift by 4 to paint background color of video 
        *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = ' '; 
  
    }

    int i = 0;
    int8_t pingpong[80] = "PINGPONG | Press ESC to quit\0";
    for(sb_idx = 26; sb_idx < 26 + strlen(pingpong); sb_idx++, i++) {
        *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = pingpong[i];
    }

}

void status_bar_welcome(){
     uint8_t sb_idx;
    for(sb_idx = 0; sb_idx < NUM_COLS; sb_idx++) {
         *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1) + 1) = (ATTRIB[currentTerminal] << 4) + (ATTRIB[currentTerminal] >> 4); // right shift by 4 to paint background color of video 
        *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = ' '; 
  
    }

    int i = 0;
    int8_t pingpong[80] = "          Welcome to Manas, Jack, Will, and Stephen's Operating System!\0";
    for(sb_idx = 0; sb_idx < 0 + strlen(pingpong); sb_idx++, i++) {
        *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = pingpong[i];
    }
}

void status_bar_fish(){
     uint8_t sb_idx;
    for(sb_idx = 0; sb_idx < NUM_COLS; sb_idx++) {
         *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1) + 1) = (ATTRIB[currentTerminal] << 4) + (ATTRIB[currentTerminal] >> 4); // right shift by 4 to paint background color of video 
        *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = ' '; 
  
    }

    int i = 0;
    int8_t pingpong[80] = "FISH | Press ESC to quit\0";
    for(sb_idx = 28; sb_idx < 28 + strlen(pingpong); sb_idx++, i++) {
        *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = pingpong[i];
    }
}

void status_bar_hello(){
     uint8_t sb_idx;
    for(sb_idx = 0; sb_idx < NUM_COLS; sb_idx++) {
         *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1) + 1) = (ATTRIB[currentTerminal] << 4) + (ATTRIB[currentTerminal] >> 4); // right shift by 4 to paint background color of video 
        *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = ' '; 
  
    }

    int i = 0;
    int8_t pingpong[80] = "HELLO | Press ESC to quit";
    for(sb_idx = 28; sb_idx < 28 + strlen(pingpong); sb_idx++, i++) {
        *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = pingpong[i];
    }
}

void status_bar_counter(){
     uint8_t sb_idx;
    for(sb_idx = 0; sb_idx < NUM_COLS; sb_idx++) {
         *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1) + 1) = (ATTRIB[currentTerminal] << 4) + (ATTRIB[currentTerminal] >> 4); // right shift by 4 to paint background color of video 
        *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = ' '; 
  
    }

    int i = 0;
    int8_t pingpong[80] = "COUNTER | Press ESC to quit";
    for(sb_idx = 27; sb_idx < 27 + strlen(pingpong); sb_idx++, i++) {
        *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = pingpong[i];
    }
}

void status_bar_ls(){
     uint8_t sb_idx;
    for(sb_idx = 0; sb_idx < NUM_COLS; sb_idx++) {
         *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1) + 1) = (ATTRIB[currentTerminal] << 4) + (ATTRIB[currentTerminal] >> 4); // right shift by 4 to paint background color of video 
        *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = ' '; 
  
    }

    int i = 0;
    int8_t pingpong[80] = "LS | Press ESC to quit";
    for(sb_idx = 29; sb_idx < 29 + strlen(pingpong); sb_idx++, i++) {
        *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = pingpong[i];
    }
}

void status_bar_cat_setup() {
    uint8_t sb_idx;
    for(sb_idx = 0; sb_idx < NUM_COLS; sb_idx++) {
         *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1) + 1) = (ATTRIB[currentTerminal] << 4) + (ATTRIB[currentTerminal] >> 4); // right shift by 4 to paint background color of video 
        *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = ' '; 
  
    }

    int i = 0;
    int8_t pingpong[80] = "CAT | Choose wanted file | Press ESC to quit";
    for(sb_idx = 21; sb_idx < 21 + strlen(pingpong); sb_idx++, i++) {
        *(uint8_t *)(VIDEO + ((NUM_COLS * (NUM_ROWS) + sb_idx) << 1)) = pingpong[i];
    }
}



int32_t color_helper() {

    
    map(VIDEO, VIDEO, _4kBPAGE);
    uint8_t cur_attrib = ATTRIB[currentTerminal];
    uint8_t new_color;
    printf_kb("Colors: \n");
    ATTRIB[currentTerminal] = (ATTRIB[currentTerminal] & 0xF0) + 0;
    printf_kb("0 = black\n");
    ATTRIB[currentTerminal] = (ATTRIB[currentTerminal] & 0xF0) + 1;
    printf_kb("1 = Dark Blue\n");
    ATTRIB[currentTerminal] = (ATTRIB[currentTerminal] & 0xF0) + 2;
    printf_kb("2 = Dark Green\n");
    ATTRIB[currentTerminal] = (ATTRIB[currentTerminal] & 0xF0) + 3;
    printf_kb("3 = Light Blue\n");
    ATTRIB[currentTerminal] = (ATTRIB[currentTerminal] & 0xF0) + 4;
    printf_kb("4 = Dark Red\n");
    ATTRIB[currentTerminal] = (ATTRIB[currentTerminal] & 0xF0) + 5;
    printf_kb("5 = Dark Pink\n");
    ATTRIB[currentTerminal] = (ATTRIB[currentTerminal] & 0xF0) + 6;
    printf_kb("6 = Dark Orange\n");
    ATTRIB[currentTerminal] = (ATTRIB[currentTerminal] & 0xF0) + 7;
    printf_kb("7 = Grey\n");
    ATTRIB[currentTerminal] = (ATTRIB[currentTerminal] & 0xF0) + 8;
    printf_kb("8 = Dark grey\n");
    ATTRIB[currentTerminal] = (ATTRIB[currentTerminal] & 0xF0) + 9;
    printf_kb("9 = Purple-Blue\n");
    ATTRIB[currentTerminal] = (ATTRIB[currentTerminal] & 0xF0) + 0xA;
    printf_kb("a = Light Green\n");
    ATTRIB[currentTerminal] = (ATTRIB[currentTerminal] & 0xF0) + 0xB;
    printf_kb("b = Vibrant Blue\n");
    ATTRIB[currentTerminal] = (ATTRIB[currentTerminal] & 0xF0) + 0xC;
    printf_kb("c = Orange\n");
    ATTRIB[currentTerminal] = (ATTRIB[currentTerminal] & 0xF0) + 0xD;
    printf_kb("d = Pink\n");
    ATTRIB[currentTerminal] = (ATTRIB[currentTerminal] & 0xF0) + 0xE;
    printf_kb("e = Yellow\n");
    ATTRIB[currentTerminal] = (ATTRIB[currentTerminal] & 0xF0) + 0xF;
    printf_kb("f = White\n");
    ATTRIB[currentTerminal] = cur_attrib;
    printf_kb("Pick Your Background Color: \n");
    int8_t buf_input[2];
    terminal_read(0, buf_input, 1);
    switch((int8_t)buf_input[0]){
        case '0':   new_color = 0;
                    break;
        case '1':   new_color = 0x10;
                    break;
        case '2':   new_color = 0x20;
                    break;
        case '3':   new_color = 0x30;
                    break;
        case '4':   new_color = 0x40;
                    break;
        case '5':   new_color = 0x50;
                    break;
        case '6':   new_color = 0x60;
                    break;
        case '7':   new_color = 0x70;
                    break;
        case '8':   new_color = 0x80;
                    break;
        case '9':   new_color = 0x90;
                    break;
        case 'a':   new_color = 0xA0;
                    break;
        case 'b':   new_color = 0xB0;
                    break;
        case 'c':   new_color = 0xC0;
                    break;
        case 'd':   new_color = 0xD0;
                    break;
        case 'e':   new_color = 0xE0;
                    break;
        case 'f':   new_color = 0xF0;
                    break;
        default:    map(VIDEO, cur_vidmap, _4kBPAGE);
                    return -1;

    }
    
    ATTRIB[currentTerminal] = (ATTRIB[currentTerminal] & 0xF0);
    if(ATTRIB[currentTerminal] == 0){
        ATTRIB[currentTerminal] = ATTRIB[currentTerminal] + 0xF; 
    }
    printf_kb("Pick Your Font Color: \n");

    
    
    terminal_read(0, buf_input, 1);
    switch((int8_t)buf_input[0]){
        case '0':   ATTRIB[currentTerminal] = new_color + 0;
                    break;
        case '1':   ATTRIB[currentTerminal] = new_color + 0x1;
                    break;
        case '2':   ATTRIB[currentTerminal] = new_color + 0x2;
                    break;
        case '3':   ATTRIB[currentTerminal] = new_color + 0x3;
                    break;
        case '4':   ATTRIB[currentTerminal] = new_color + 0x4;
                    break;
        case '5':   ATTRIB[currentTerminal] = new_color + 0x5;
                    break;
        case '6':   ATTRIB[currentTerminal] = new_color + 0x6;
                    break;
        case '7':   ATTRIB[currentTerminal] = new_color + 0x7;
                    break;
        case '8':   ATTRIB[currentTerminal] = new_color + 0x8;
                    break;
        case '9':   ATTRIB[currentTerminal] = new_color + 0x9;
                    break;
        case 'a':   ATTRIB[currentTerminal] = new_color + 0xA;
                    break;
        case 'b':   ATTRIB[currentTerminal] = new_color + 0xB;
                    break;
        case 'c':   ATTRIB[currentTerminal] = new_color + 0xC;
                    break;
        case 'd':   ATTRIB[currentTerminal] = new_color + 0xD;
                    break;
        case 'e':   ATTRIB[currentTerminal] = new_color + 0xE;
                    break;
        case 'f':   ATTRIB[currentTerminal] = new_color + 0xF;
                    break;
        default:    ATTRIB[currentTerminal] = cur_attrib;
                    clear();
                    putc_kb('\r');
                    map(VIDEO, cur_vidmap, _4kBPAGE);
                    return -1;
    }

    if(((ATTRIB[currentTerminal] & 0xF0) >> 4) == (ATTRIB[currentTerminal] & 0x0F)) {
        ATTRIB[currentTerminal] = cur_attrib;
        clear();
        putc_kb('\r');
        printf_kb("Cannot have background be same color as font\n");
        map(VIDEO, cur_vidmap, _4kBPAGE);
        return 0;
    }


    clear();
    putc_kb('\r');
    status_bar_init();
    map(VIDEO, cur_vidmap, _4kBPAGE);
    return 0;
}

// sets up welcome screen
void paintScreen()
{
    map(VIDEO, VIDEO, 0); 
    
    //make the terminal 
    screen_x[0] = 10; 
    screen_y[0] = 5;
    printf_kb("Terminal");

    screen_x[0] = 35;
    printf_kb("Paint"); 

    screen_x[0] = 60;
    printf_kb("Pingpong");

    screen_x[0] = 10; 
    screen_y[0] += 5; 
    printf_kb("Fish"); 

    screen_x[0] = 35; 
    printf_kb("Hello"); 

    screen_x[0] = 60;
    printf_kb("Counter"); 

    screen_x[0] = 10; 
    screen_y[0] += 5; 
    printf_kb("ls"); 

    screen_x[0] = 35;
    printf_kb("cat"); 

    // screen_x[0] = 35;
    // printf_kb("Coming Soon"); 

}

void paintScreen_cat() {
    map(VIDEO, VIDEO, 0); 
    screen_x[0] = 10; 
    screen_y[0] = 5; 
    printf_kb("frame0.txt"); 

    screen_x[0] = 35;
    printf_kb("frame1.txt"); 

    screen_x[0] = 60; 
    printf_kb("verylong-.txt"); 


}


int8_t saved_color = 0;

void draw_cursor(int x, int y) {
    map(VIDEO, VIDEO, _4kBPAGE); 

    if(paint_flag && paint_press) {
        *(uint8_t *)(video_mem + ((NUM_COLS *y + x) << 1) + 1) = paint_color << 4;
    }
    else if(paint_flag &&  *(uint8_t *)(video_mem + ((NUM_COLS *y + x) << 1) + 1) != 0) {
        saved_color =  *(uint8_t *)(video_mem + ((NUM_COLS *y + x) << 1) + 1);
        *(uint8_t *)(video_mem + ((NUM_COLS *y + x) << 1) + 1) = (*(uint8_t *)(video_mem + ((NUM_COLS *y + x) << 1) + 1) & 0x0F);
    }
    else if((ATTRIB[currentTerminal] & 0xF0) != 0xF0)
        *(uint8_t *)(video_mem + ((NUM_COLS *y + x) << 1) + 1) = (*(uint8_t *)(video_mem + ((NUM_COLS *y + x) << 1) + 1) & 0x0F) + 0xF0;
    else
        *(uint8_t *)(video_mem + ((NUM_COLS *y + x) << 1) + 1) = (*(uint8_t *)(video_mem + ((NUM_COLS *y + x) << 1) + 1) & 0x0F);
    if(cur_vidmap != -1)
        map(VIDEO, cur_vidmap, _4kBPAGE); 
}

void undraw_cursor(int x, int y) {
    if(paint_flag &&  *(uint8_t *)(video_mem + ((NUM_COLS *y + x) << 1) + 1) == 0x00) {
        *(uint8_t *)(video_mem + ((NUM_COLS *y + x) << 1) + 1) = saved_color;
    }
    else if(!paint_flag) {
        *(uint8_t *)(video_mem + ((NUM_COLS *y + x) << 1) + 1) = (ATTRIB[currentTerminal] & 0xF0) + (*(uint8_t *)(video_mem + ((NUM_COLS *y + x) << 1) + 1) & 0x0F);
    }
}


