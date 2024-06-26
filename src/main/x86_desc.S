# x86_desc.S - Set up x86 segment descriptors, descriptor tables
# vim:ts=4 noexpandtab

#define ASM     1
#include "x86_desc.h"

.text

.globl pde, pte, pte_video
.globl ldt_size, tss_size
.globl gdt_desc, ldt_desc, tss_desc
.globl tss, tss_desc_ptr, ldt, ldt_desc_ptr
.globl gdt_ptr
.globl idt_desc_ptr, idt

.align 4


tss_size:
    .long tss_bottom - tss - 1

ldt_size:
    .long ldt_bottom - ldt - 1

    .word 0 # Padding
ldt_desc:
    .word KERNEL_LDT
    .long ldt

    .align 4
tss:
_tss:
    .rept 104
    .byte 0
    .endr
tss_bottom:

    .align  16
gdt:
_gdt:

    # First GDT entry cannot be used
    .quad 0

    # NULL entry
    .quad 0

    # Segmentation will not be used
    # CS and DS both are 0-4GB r/w segments
    #
    # The layout is (from Intel IA-32 reference manual):
    #  31        24 23  22  21  20  19   16 15  14 13 12  11   8 7          0
    # |----------------------------------------------------------------------|
    # |            |   | D |   | A |  Seg  |   |  D  |   |      |            |
    # | Base 31:24 | G | / | 0 | V | Limit | P |  P  | S | Type | Base 23:16 |
    # |            |   | B |   | L | 19:16 |   |  L  |   |      |            |
    # |----------------------------------------------------------------------|
    #
    # |----------------------------------------------------------------------|
    # |                                    |                                 |
    # | Base 15:0                          | Segment Limit 15:0              |
    # |                                    |                                 |
    # |----------------------------------------------------------------------|

gdt_ptr:
    # Set up an entry for kernel CS
    .quad 0x00CF9A000000FFFF

    # Set up an entry for kernel DS
    .quad 0x00CF92000000FFFF

    # Set up an entry for user CS
    .quad 0x00CFFA000000FFFF

    # Set up an entry for user DS
    .quad 0x00CFF2000000FFFF

    # Set up an entry for TSS
tss_desc_ptr:
    .quad 0

    # Set up one LDT
ldt_desc_ptr:
    .quad 0

gdt_bottom:

    .align 16
ldt:
    .rept 4
    .quad 0
    .endr
ldt_bottom:

.align 4
    .word 0 # Padding
# Need a GDT Descriptor structure pointer:
# Offset is bits 48-16: 32 bit address (gdt from above)
# Size is bits 15-0: The size of the table in bytes subtracted by 1. 
# This subtraction occurs because the maximum value of Size is 65535, while the GDT can be up to 65536 bytes in length (8192 entries)

# Kind of copy of idt_desc_ptr:
gdt_desc:
    .word gdt_bottom - gdt - 1
    .long gdt

idt_desc_ptr:
    .word idt_bottom - idt - 1
    .long idt


    .align  16
idt:
_idt:
    .rept NUM_VEC
    .quad 0
    .endr

idt_bottom:


# Allocate memory using pages same way as idt for all page tables (4096 b/c 4KB between entries, 1024 entries in tables)
.align 4096
pde:
_pde:
    .rept 1024
    .long 0
    .endr

pde_bottom:

.align 4096
pte:
_pte:
    .rept 1024
    .long 0
    .endr

pte_bottom:

.align 4096
pte_video:
_pte_video:
    .rept 1024
    .long 0
    .endr

pte_video_bottom:


# Based off of functions in https://wiki.osdev.org/Setting_Up_Paging

# enables paging with given page directory pointer on stack
# enables 4MB pages as well
.text
.globl load_and_enable_PD
load_and_enable_PD:
    pushl %ebp
    movl %esp, %ebp
    movl 8(%esp), %eax
    movl %eax, %cr3

    movl %cr4, %eax
    # must set the 4th bit in cr4 register, the page size extention bit to 
    # enable 32-bit paging mode in oreder to use 4 MiB huge pages in addition to 4 KiB pages.
    orl $0x00000010, %eax
    movl %eax, %cr4

    movl %cr0, %eax
    # must set the 32th bit in the CR0 register, the paging bit to 1 to enable paging
    orl $0x80000000, %eax
    movl %eax, %cr0
    movl %ebp, %esp
    popl %ebp
    ret



# flushes cr3 (use later) sets to new pde address (resets tlb)

.text
.global flush
flush:
    pushl %ebp
    movl %esp, %ebp
    movl 8(%esp), %eax
    movl %eax, %cr3
    movl %ebp, %esp
    popl %ebp
    ret


