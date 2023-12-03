#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "filesys.h"
#include "devices.h"
#include "rtc.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}

char deref(char * a){
	char test = *a;
	return test;
}

/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* DIV Test
 * 
 * tests divide by 0 exception
 * Inputs: None
 * Outputs: idt error
 * Side Effects: Crashes Kernel
 * Coverage: Load IDT, SET_IDT, handlers
 * Files: idt.c
 */
int div_test(){
	TEST_HEADER;
	int one = 1;
	int zero = 0;
	int check = one / zero;

	printf("will never reach %d\n", check); 
	return PASS;

}

/* NULL Test
 * 
 * dereferences null pointer
 * Inputs: None
 * Outputs: page fault on success, 0 on failure
 * Side Effects: Crashes Kernel
 * Coverage: Page Fault, init_pageing
 * Files: idt.c, paging.c
 */
/* dereferences null pointer and should display an error */ 
int test_null()
{
	 deref(NULL); 
	 return 0; 
}


/* NEG ADDRESS Test
 * 
 * dereferences negative address
 * Inputs: None
 * Outputs: page fault on success, 0 on failure
 * Side Effects: Crashes Kernel
 * Coverage: Page Fault, init_pageing
 * Files: idt.c, paging.c
 */
int test_deref_negative()
{
	// -100 just neg address
	deref((char*) -100);
	return 0; 
}


/* Deref Assigned pointer Test
 * 
 * dereferences a pointer
 * Inputs: None
 * Outputs: PASS on success, page fault on failure
 * Side Effects: Crashes Kernel
 * Coverage: Page Fault, init_pageing
 * Files: idt.c, paging.c
 */
extern int deref_pointer()
{
	int result = PASS;
	// 28 doesn't matter
	char b = 28;
	char * a = &b;
	if(b != deref(a)) {
		result = FAIL;
	}
	return result;
}


/* In page before video ADDRESS Test
 * 
 * Tries to dereference memory that is not present before vid in paging
 * Inputs: None
 * Outputs: page fault on success, 0 on failure
 * Side Effects: Crashes Kernel
 * Coverage: Page Fault, init_pageing
 * Files: idt.c, paging.c
 */
int deref_notPresentbeforevid()
{
	// 0x2 = before vid
	char * ptr = (char *) 0x2;
	deref(ptr);
	return 0; 
}

/* Deref video_mem Test
 * 
 * dereferences video_mem
 * Inputs: None
 * Outputs: PASS on success, page fault on failure
 * Side Effects: Crashes Kernel
 * Coverage: Page Fault, init_pageing
 * Files: idt.c, paging.c
 */
int deref_videoMem()
{
	char * vid_m = (char*) 0xB8000;
	deref(vid_m);
	return PASS; 
}


/* In page After video ADDRESS Test
 * 
 * Tries to dereference memory that is not present after vid in paging before kernel
 * Inputs: None
 * Outputs: page fault on success, 0 on failure
 * Side Effects: Crashes Kernel
 * Coverage: Page Fault, init_pageing
 * Files: idt.c, paging.c
 */
int deref_notPresent_aftervid()
{
	// 0xCFFFF = after vid, before kernel address
	char * ptr = (char *) 0xCFFFF;
	deref(ptr);
	return 0; 
}


/* Deref kernel_mem Test
 * 
 * dereferences a kernel mem address
 * Inputs: None
 * Outputs: PASS on success, page fault on failure
 * Side Effects: Crashes Kernel
 * Coverage: Page Fault, init_pageing
 * Files: idt.c, paging.c
 */
int deref_kernelMem()
{
	//0x400000 = start of kernel
	char * k_m = (char *) 0x400000;
	deref(k_m);
	// 0x7FFFFF end of kernel
	char * l_a = (char *) 0x7FFFFF;
	deref(l_a);
	return PASS; 
}


/* After Kernel Mem test
 * 
 * dereferences after kernel mem 
 * Inputs: None
 * Outputs: page fault on success, 0 on failure
 * Side Effects: Crashes Kernel
 * Coverage: Page Fault, init_pageing
 * Files: idt.c, paging.c
 */
int deref_kernelMemAfter()
{
	// 0x800000 = after kernel
	char * a_k = (char *) 0x800000;
	deref(a_k);
	return 0; 
}


/* System Call test
 * 
 * test int 80 
 * Inputs: None
 * Outputs: Sys call on success, 0 on failure
 * Side Effects: Crashes Kernel
 * Coverage: idt, set_idt, handler
 * Files: idt.c
 */
/*runs system call*/ 
// int sysCallTesthelper(char * buf, char * read_buf, uint8_t * exec_buf){

	/*Test Open*/
	// asm volatile("movl $5, %eax");
	// asm volatile("int $0x80");

	/*Test Close */
	// asm volatile("movl $6, %eax");
	// asm volatile("int $0x80");

	/*Test write*/
	// asm volatile("movl $4, %eax");
	// asm volatile("movl $50, %edx");
	// asm volatile("movl 8(%ebp), %ecx");
	// asm volatile("movl $1, %ebx");
	// asm volatile("int $0x80");
	// register uint32_t saved_write_eax asm("eax");
	// printf("%d\n", saved_write_eax);

	// /*Test Read*/
	// asm volatile("movl $3, %eax");
	// asm volatile("movl $1000, %edx");
	// asm volatile("movl 12(%ebp), %ecx");
	// asm volatile("movl $0, %ebx");
	// asm volatile("int $0x80");
	// register uint32_t saved_read_eax asm("eax");
	// printf("%d\n", saved_read_eax);
	// printf("Test read arg saved: \n");
	// terminal_write(1, read_buf, 128);

	// // int32_t testing = TEST_RETURN();
	// // printf("%d", testing);
	// // test arg
	// // asm volatile("movl $1, %eax");
	// // asm volatile("movl $0, %ebx");
	// // asm volatile("int $0x80");

	// asm volatile("movl $1, %eax");
	// asm volatile("movl $0x3b, %ebx");
	// asm volatile("int $0x80");
	// register uint32_t saved_eax asm("eax");
	// printf("%d\n", saved_eax);

	// asm volatile("movl 16(%ebp), %ebx");
	// asm volatile("call sys_execute");
	// register uint32_t saved_eax_e asm("eax");
	// printf("%d\n", saved_eax_e);








// 	return 0; 
// }


// int sysCallTest()
// {
// 	// x80 in idt is sys call

// 	char buf[50] = "linking syswrite actually is working eax = 4\n";
// 	char readbuf[129];
// 	int i;
// 	for(i = 0; i < 129; i++) {
// 		readbuf[i] = NULL;
// 	}
	
// 	uint8_t execbuf[50] = "shell";
// 	sysCallTesthelper(buf, readbuf, execbuf);
// 	return PASS; 
// }



/* Checkpoint 2 tests */
/* terminal test 
 * Inputs: None
 * Outputs: None
 * Side Effects: Crashes Kernel
 * Coverage: terminal
 * Files: devices.c, lib.c
 * */
int terminalTest() {
	terminal_open(0);
    char buf[129]; // in order for printf to be null terminated (will use 128 entries, so 129th must be null to print correctlty)

	// Test Terminal Write
    char*  need = "TEST_WRITE: PASS!" ;
    int test = terminal_write(1, need, 17);

	// Test terminal read
    printf("\nTEST_READ: \n");
    test = terminal_read(0, buf, 5);
    printf("%d\n", test);
    int i = 0;
    while(buf[i] != NULL) {
        i++;
    }
    if(i == test) {
        printf("PASS!\n");
    }
    terminal_write(1, buf, test);

	// Test Terminal Close
    terminal_close(0);
    // should print -1
    test = terminal_read(0, buf, 100);
    printf("%d", test);
    test = terminal_write(1, need, 7);
    printf(" %d\n", test);

	// Test while Loop
    terminal_open(0);
    char read_buf[129];
	need = "391OS> ";
    while(1) {
		terminal_write(0, need, 7);
        int n = terminal_read(0 , read_buf, 100000);
        terminal_write(0, read_buf, n);
        printf("%d\n", n);
        for(i = 0; i < n; i++) {
            read_buf[i] = NULL;
        }
    }
	
	// will never reach
	return PASS;
}

/*Test Read Directory
*
*
*
*/
// int directory_read_test(){
// 	volatile int32_t ret = 1;
// 	int8_t buf[40];
// 	uint8_t dirname[33] = ".";
// 	int32_t files = 0;
// 	while(ret != 0){
// 		ret = directory_read(dirname, 1, buf, FILENAME_SIZE);
// 		if(ret) {
// 			files++;
// 			terminal_write(0, buf, ret);
// 			printf("\n");
// 		}
// 	}
	
// 	printf("\nFiles: %d\n", files);
// 	if(files == (fs_info.boot_block_base->dir_count))
// 		return PASS;
// 	return FAIL;
// }

/*Read an Executable
*
*
*/
int read_file_exec(){
	dentry_t d;
	uint8_t fname[32] = "ls";
	read_dentry_by_name(fname, &d);
	printf("Filename : %s\n", d.file_name);
	printf("File type : %d\n", d.file_type);
    printf("Inode num : %d\n", d.inode_num);
    printf("File Length : %d\n", (fs_info.inode_base + d.inode_num)->length);
	uint8_t buf[14000];
    int32_t bytes = read_data(d.inode_num, 0, buf, 2000000);
    printf("bytes read: %d\n", bytes);

	terminal_write(0, buf, bytes); 
    
	if(bytes == (fs_info.inode_base + d.inode_num)->length)
		return PASS;
	return FAIL;
}

/*Read a small file
*
*
*/
int read_file_small(){
	dentry_t d;
	uint8_t fname[32] = "frame0.txt";
	read_dentry_by_name(fname, &d);
	printf("Filename : %s\n", d.file_name);
	printf("File type : %d\n", d.file_type);
    printf("Inode num : %d\n", d.inode_num);
    printf("File Length : %d\n", (fs_info.inode_base + d.inode_num)->length);
	uint8_t buf[200];
    int32_t bytes = read_data(d.inode_num, 0, buf, 2000000);
    printf("bytes read: %d\n", bytes);
	terminal_write(0, buf, bytes); 
	// printf("%d", bytes);
	if(bytes == (fs_info.inode_base + d.inode_num)->length)
		return PASS;
	return FAIL;
}
/*Read a large file
*
*
*/
int read_file_large(){
	dentry_t d;
	uint8_t fname[40] = "verylargetextwithverylongname.txt";
	read_dentry_by_name(fname, &d);
	printf("Filename : %s\n", d.file_name);
	printf("File type : %d\n", d.file_type);
    printf("Inode num : %d\n", d.inode_num);
    printf("File Length : %d\n", (fs_info.inode_base + d.inode_num)->length);
	uint8_t buf[2000000];
    int32_t bytes = read_data(d.inode_num, 0, buf, 2000000);
    printf("bytes read: %d\n", bytes);

	terminal_write(0, buf, bytes); 

	if(bytes == (fs_info.inode_base + d.inode_num)->length)
		return PASS;
	return FAIL;
}

/* File Read
*
*
*
*/
// int file_read_test(){
// 	uint8_t fname[40] = "frame1.txt";
// 	dentry_t d;
// 	read_dentry_by_name(fname, &d);
// 	printf("Filename : %s\n", d.file_name);
// 	printf("File type : %d\n", d.file_type);
//     printf("Inode num : %d\n", d.inode_num);
//     printf("File Length : %d\n", (fs_info.inode_base + d.inode_num)->length);
// 	uint8_t buf[2000000];
//     int32_t bytes = file_read(fname, 0, buf, 2000000);
//     printf("bytes read: %d\n", bytes); 

// 	terminal_write(0, buf, bytes); 

// 	if(bytes == (fs_info.inode_base + d.inode_num)->length)
// 		return PASS;
// 	return FAIL;
// }

/* RTC TESTS */

/* RTC test for open
 * 
 * test frequency = 2
 * Inputs: None
 * Outputs: PASS on success, nothing on failure
 * Side Effects: prints 1s to kernel
 * Coverage: rtc
 * Files: rtc.c
//  */
// int rtc_open_test(){
// 	rtc_open(); /* open rtc */
// 	int i; /* initialize i */
	
// 	// in the for loop just call read rtc
// 	for(i = 0; i < 8; i++){ /* print 8 times */
// 		rtc_read(); /* read functionality */
// 		printf("%d", 1); /* print 1s */
// 	}

// 	return PASS; /* return pass if passed */
// }

// /* RTC test for writing all frequencies
//  * 
//  * test frequency = multiples of 2 from 2 to 1024
//  * Inputs: None
//  * Outputs: PASS on success, FAIL on failure
//  * Side Effects: prints 1s to kernel
//  * Coverage: rtc
//  * Files: rtc.c
//  */
// int rtc_write_test_passing(){
// 	rtc_open(); /* open rtc */
// 	int i, j, out, print_amount; /* variables to use */
	
// 	/* this loops through all frequencies and prints 1 */
// 	for(j = 2; j < 1025; j = j*2){ /* increment through the possible frequencies */
// 		printf("frequency: %d       ", j); // for printing to kernel
		

// 		out = rtc_write((const void*) j); // write the frequency
// 		if(out == -1) return FAIL;  // if return value is -1, we failed test case
		

// 		/* this is all for determining how many 1s should be printed */
// 		if(j == 2){ /* frequency = 2 */
// 			print_amount = 5; /* how many 1s to print */
// 		}
// 		else if(j == 4){ /* frequency = 4 */
// 			print_amount = 10; /* how many 1s to print */
// 		}
// 		else if(j == 8){ /* frequency = 8 */
// 			print_amount = 20; /* how many 1s to print */
// 		}
// 		else if(j == 16){ /* frequency = 16 */
// 			print_amount = 40; /* how many 1s to print */
// 		}
// 		else if(j == 32){ /* frequency = 32 */
// 			print_amount = 60; /* how many 1s to print */
// 		}
// 		else if(j == 64){ /* frequency = 64 */
// 			print_amount = 120; /* how many 1s to print */
// 		}
// 		else if(j == 128){ /* frequency = 128 */
// 			print_amount = 240; /* add 25 more 1s to be printed */
// 		}
// 		else if(j == 256){ /* frequency = 256 */
// 			print_amount = 480; /* how many 1s to print */
// 		}
// 		else if(j == 512){ /* frequency = 512 */
// 			print_amount = 960; /* how many 1s to print */
// 		}
// 		else if(j == 1024){ /* frequency = 1024 */
// 			print_amount = 1920; /* how many 1s to print */
// 		}
		

// 		for(i = 0; i < print_amount; i++){ 	// print print_amount 1's
// 			rtc_read();				// read
// 			printf("%d", 1);		// print 1

// 			// if(i % 53 == 0){ /* when i % 53, go to new line */
// 			// 	if(i != 0){ /* make sure we don't do \n when i = 0 */
// 			// 		printf("\n                     "); /* for the spacing */
// 			// 	}
// 			// }

// 			/* go to the next line after 15 1s printed*/
// 		}
// 		printf("\n");				//print new line
// 	}
	
// 	return PASS; /* return PASS if it passed through */
// }
	

// int rtc_write_test_failing(){
// 	printf("\n");
// 	int frequency;
// 	out = rtc_write(120938); // write the frequency
// 	printf("This test case will fail because the frequency is not a power of 2 and isn't limited to 1024.\n The frequency is: %d%", 120938);
// 	if(out == -1) return FAIL;  // if return value is -1, we failed test case

// 	rtc_open();
// 	int i, j, out, print_amount, print_counter;
	
// 	for(i = 0; i < 5; i++){ 	// print 5 1's
// 		rtc_read();				// read
// 		printf("%d", 1);		// print 1
// 	}
// }




/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	printf("Starting Tests: \n");
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("div_test", div_test());
	// TEST_OUTPUT("test NULL", test_null()); 
	// TEST_OUTPUT("test neg address", test_deref_negative()); 
	// TEST_OUTPUT("Test dereferencing unsigned pointer", deref_pointer()); 
	// TEST_OUTPUT("deref mem not assigned in page table", deref_notPresentbeforevid()); 
	// TEST_OUTPUT("deref video memory", deref_videoMem()); 
	// TEST_OUTPUT("deref mem not assigned in page table", deref_notPresent_aftervid()); 
	// TEST_OUTPUT("deref kernel mem", deref_kernelMem()); 
	// TEST_OUTPUT("deref after kernel mem", deref_kernelMemAfter());  
	// TEST_OUTPUT("sys_call", sysCallTest());  
	// TEST_OUTPUT("directory_read_test", directory_read_test());   
	// TEST_OUTPUT("read_file_small", read_file_small());
	// TEST_OUTPUT("read_file_exec", read_file_exec());
	// TEST_OUTPUT("read_file_large", read_file_large());
	// TEST_OUTPUT("file_read", file_read_test());
	// TEST_OUTPUT("Test Terminal Driver", terminalTest());
	// TEST_OUTPUT("rtc_open_test", rtc_open_test()); 
	// TEST_OUTPUT("rtc_test", rtc_write_test_passing()); 

	printf(":End of Current Tests \n");
	 
}

