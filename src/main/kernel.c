/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "IDT.h"
#include "devices.h"
#include "rtc.h"
#include "scheduler.h"
#include "mouse.h"

#define RUN_TESTS

/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags, bit)   ((flags) & (1 << (bit)))

/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. 
   Initializes the idt, pic, keyboard, rtc, and paging.
   Enables Interrupts and launches test cases
   */
void entry(unsigned long magic, unsigned long addr) {

    multiboot_info_t *mbi;

    /* Clear the screen. */
    ATTRIB[0] = T1_COLOR; // Set color wanted for text and background
    ATTRIB[1] = T2_COLOR;
    ATTRIB[2] = T3_COLOR;
    // bits 32-16 change background, bits 15-0 change text
        /*
        0 = black
        1 = Dark Blue
        2 = Dark Green
        3 = Light Blue
        4 = Dark Red
        5 = Dark Pink
        6 = Dark Orange
        7 = Grey
        8 = Dark grey
        9 = Purple-Blue
        A = Light Green
        B = Vibrant Blue
        C = Orange
        D = Pink
        E = Yellow
        F = White
        */

    clear();

    /* Am I booted by a Multiboot-compliant boot loader? */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        printf("Invalid magic number: 0x%#x\n", (unsigned)magic);
        return;
    }

    /* Set MBI to the address of the Multiboot information structure. */
    mbi = (multiboot_info_t *) addr;

    /* Print out the flags. */
    printf("flags = 0x%#x\n", (unsigned)mbi->flags);

    /* Are mem_* valid? */
    if (CHECK_FLAG(mbi->flags, 0))
        printf("mem_lower = %uKB, mem_upper = %uKB\n", (unsigned)mbi->mem_lower, (unsigned)mbi->mem_upper);

    /* Is boot_device valid? */
    if (CHECK_FLAG(mbi->flags, 1))
        printf("boot_device = 0x%#x\n", (unsigned)mbi->boot_device);

    /* Is the command line passed? */
    if (CHECK_FLAG(mbi->flags, 2))
        printf("cmdline = %s\n", (char *)mbi->cmdline);

    if (CHECK_FLAG(mbi->flags, 3)) {
        int mod_count = 0;
        int i;
        module_t* mod = (module_t*)mbi->mods_addr;
        while (mod_count < mbi->mods_count) {
            printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
            printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
            printf("First few bytes of module:\n");
            for (i = 0; i < 16; i++) {
                printf("0x%d ", *((char*)(mod->mod_start+i)));
            }
            printf("\n");
            mod_count++;
            mod++;
        }
        
    }
    /* Bits 4 and 5 are mutually exclusive! */
    if (CHECK_FLAG(mbi->flags, 4) && CHECK_FLAG(mbi->flags, 5)) {
        printf("Both bits 4 and 5 are set.\n");
        return;
    }

    /* Is the section header table of ELF valid? */
    if (CHECK_FLAG(mbi->flags, 5)) {
        elf_section_header_table_t *elf_sec = &(mbi->elf_sec);
        printf("elf_sec: num = %u, size = 0x%#x, addr = 0x%#x, shndx = 0x%#x\n",
                (unsigned)elf_sec->num, (unsigned)elf_sec->size,
                (unsigned)elf_sec->addr, (unsigned)elf_sec->shndx);
    }

    /* Are mmap_* valid? */
    if (CHECK_FLAG(mbi->flags, 6)) {
        memory_map_t *mmap;
        printf("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
                (unsigned)mbi->mmap_addr, (unsigned)mbi->mmap_length);
        for (mmap = (memory_map_t *)mbi->mmap_addr;
                (unsigned long)mmap < mbi->mmap_addr + mbi->mmap_length;
                mmap = (memory_map_t *)((unsigned long)mmap + mmap->size + sizeof (mmap->size)))
            printf("    size = 0x%x, base_addr = 0x%#x%#x\n    type = 0x%x,  length    = 0x%#x%#x\n",
                    (unsigned)mmap->size,
                    (unsigned)mmap->base_addr_high,
                    (unsigned)mmap->base_addr_low,
                    (unsigned)mmap->type,
                    (unsigned)mmap->length_high,
                    (unsigned)mmap->length_low);
    }

    /* Construct an LDT entry in the GDT */
    {
        seg_desc_t the_ldt_desc;
        the_ldt_desc.granularity = 0x0;
        the_ldt_desc.opsize      = 0x1;
        the_ldt_desc.reserved    = 0x0;
        the_ldt_desc.avail       = 0x0;
        the_ldt_desc.present     = 0x1;
        the_ldt_desc.dpl         = 0x0;
        the_ldt_desc.sys         = 0x0;
        the_ldt_desc.type        = 0x2;

        SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
        ldt_desc_ptr = the_ldt_desc;
        lldt(KERNEL_LDT);
    }

    /* Construct a TSS entry in the GDT */
    {
        seg_desc_t the_tss_desc;
        the_tss_desc.granularity   = 0x0;
        the_tss_desc.opsize        = 0x0;
        the_tss_desc.reserved      = 0x0;
        the_tss_desc.avail         = 0x0;
        the_tss_desc.seg_lim_19_16 = TSS_SIZE & 0x000F0000;
        the_tss_desc.present       = 0x1;
        the_tss_desc.dpl           = 0x0;
        the_tss_desc.sys           = 0x0;
        the_tss_desc.type          = 0x9;
        the_tss_desc.seg_lim_15_00 = TSS_SIZE & 0x0000FFFF;

        SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

        tss_desc_ptr = the_tss_desc;

        tss.ldt_segment_selector = KERNEL_LDT;
        tss.ss0 = KERNEL_DS;
        tss.esp0 = 0x800000 - 0x04;
        ltr(KERNEL_TSS);
    }
    clear();
    putc('\r');


    /* set up the pcbs to their correct location in memory (defined in filesys.h) */
    pcb0 = (pcb_t *) _pcb0; 
    pcb1 = (pcb_t *) _pcb1; 
    pcb2 = (pcb_t *) _pcb2; 
    pcb3 = (pcb_t *) _pcb3; 
    pcb4 = (pcb_t *) _pcb4; 
    pcb5 = (pcb_t *) _pcb5; 

    init_pcb(pcb0);
    init_pcb(pcb1);
    init_pcb(pcb2);
    init_pcb(pcb3);
    init_pcb(pcb4);
    init_pcb(pcb5);

    pcb_array[0] = pcb0;
    pcb_array[1] = pcb1;
    pcb_array[2] = pcb2;
    pcb_array[3] = pcb3;
    pcb_array[4] = pcb4;
    pcb_array[5] = pcb5;


    // set global varaibles
    pcbIndex = 0; 
    dir_read_idx = 0;
    currentTerminal = 0;  //starting in the first terminal 
    startmouse = 0;
    paint_flag = 0;
    finish_paint = 1;
    gui_flag = 0; 
    welcome = 0;
    cat_flag = 0;
    paint_press = 0;
    cur_vidmap = -1;

    MOUSE_RL_SCALE = 0.14;
    MOUSE_UD_SCALE = 0.07;



    // initialize file system
    file_sys_init((multiboot_info_t*) addr);


        



    
    // initialize the idt
    init_idt();
    /* Init the PIC */
    i8259_init();
    // init KB to allow interrupts from
    KB_init();
    // init the RTC to allow interrupts from
    RTC_init();
    // init pit interrupts
    pit_init();
     // init paging
    paging_init();
    // init  mouse
    mouse_init(); 

    terminal_open(0);


    map((uint32_t) VIDEOT1, (uint32_t) VIDEOT1, _4kBPAGE); //map the 4kb pages to be used for video memory (one to one)
    map((uint32_t) VIDEOT2, (uint32_t) VIDEOT2, _4kBPAGE); 
    map((uint32_t) VIDEOT3, (uint32_t) VIDEOT3, _4kBPAGE); 

    // set global flag variables
    terminal1_switch = 0;
    terminal2_switch = 0;
    terminal3_switch = 0;
    // set second and third terminal flags to be uninitialized
    secondTerm = 0; 
    thirdTerm = 0; 

    

    
    

    /* Enable interrupts */

    sti(); 


    

// #ifdef RUN_TESTS
//     /* Run tests */
    // launch_tests();
// #endif



    clear();
    putc('\r'); 

    // paintScreen(); 

    /*setup the shell for terminal 0 */ 
    sys_execute_setup((uint8_t *)"shell", NULL, NULL); // call the execute (set esp and edp to NULL since not going to be used)


    /* Spin (nicely, so we don't chew up cycles) */
    asm volatile (".1: hlt; jmp .1;");
}







/* init_pcb
 * 
 * Resets pcb data structure of pcb pointer passed in
 * Inputs: pab_t pointer
 * Outputs: 0 on success
 * Side Effects: clears pcb data structure for cur pointer
 */
int32_t init_pcb(pcb_t * cur_pcb) {
    cli();
    cur_pcb->active = 0; //set to inactive 
    cur_pcb->pid = 0;    //set to 0 for the 0 process
    cur_pcb->parentID = -1;  // Set to No parent
    cur_pcb->original_esp = NULL; 
    cur_pcb->original_ebp = NULL; 
    cur_pcb->saved_ebp = NULL; 
    cur_pcb->saved_esp = NULL; 
    cur_pcb->cmd_ = NULL; 
    cur_pcb->terminalNum = -1; 
    cur_pcb->mainShell = -1; 
    cur_pcb->is_waiting = 0;
    cur_pcb->has_child = 0;
    cur_pcb->name = "default";

    /*setting up stdin */
    // initialize to -1 to say empty
    cur_pcb->fd_array.fdt[STDIN_FDT].file_pos = -1;
    cur_pcb->fd_array.fdt[STDIN_FDT].flags[FILE_TYPE] = -1;
    cur_pcb->fd_array.fdt[STDIN_FDT].flags[FD_FLAG] = 1; 
    cur_pcb->fd_array.fdt[STDIN_FDT].inode_num = 0; 
    file_operations_t stdin; 
    stdin.read = (void *) terminal_read; 
    stdin.write = NULL; 
    stdin.open = NULL; 
    stdin.close = NULL; 
    cur_pcb->fd_array.fdt[STDIN_FDT].fotp = stdin; 

    /*setting up stdout */    
    cur_pcb->fd_array.fdt[STDOUT_FDT].file_pos = -1; 
    cur_pcb->fd_array.fdt[STDOUT_FDT].flags[FILE_TYPE] = -1;
    cur_pcb->fd_array.fdt[STDOUT_FDT].flags[FD_FLAG] = 1; 
    cur_pcb->fd_array.fdt[STDOUT_FDT].inode_num = 0; 
    file_operations_t stdout; 
    stdout.write = (void *) terminal_write; 
    stdout.read = NULL; 
    stdout.open = NULL; 
    stdout.close = NULL; 
    cur_pcb->fd_array.fdt[STDOUT_FDT].fotp = stdout; 

    int i;
    for(i = START_FDT_FILES; i < NUM_FDT_ENTRIES; i++) {
        cur_pcb->fd_array.fdt[i].file_pos = 0;
        cur_pcb->fd_array.fdt[i].flags[FILE_TYPE] = -1;
        cur_pcb->fd_array.fdt[i].flags[FD_FLAG] = 0;
        cur_pcb->fd_array.fdt[i].inode_num = 0;
        file_operations_t temp;
        temp.write = NULL; 
        temp.read = NULL; 
        temp.open = NULL; 
        temp.close = NULL; 
        cur_pcb->fd_array.fdt[i].fotp = temp; 

    }
    // reset read dentry index after every process
    dir_read_idx = 0;



    return 0;
}

