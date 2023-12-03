
# function that save register and flags, calls handler function, the returns from interrupt with registers and flags intact

#define IDT_LINK(name, func)     \
    .global name                 ;\
    name:                       ;\
        pushal                  ;\
        pushfl                  ;\
        call func               ;\
        popfl                   ;\
        popal                   ;\
        iret


IDT_LINK(RTC_handler_linkage, RTC_handler);
IDT_LINK(KB_handler_linkage, KB_handler);                
IDT_LINK(pit_handler_linkage, pit_handler_setup);
IDT_LINK(mouse_handler_linkage, mouse_handler);


# System linkage, check if eax is correct syscall value (1->10)
#define SYS_LINK(name)           \
    .global name                 ;\
    name:                       ;\
        pushl %ebx              ;\
        pushl %ecx              ;\
        pushl %edx              ;\
        pushl %esi              ;\
        pushl %edi              ;\
        pushl %ebp              ;\
        pushl %esp              ;\
        pushfl                  ;\
        cmpl $1, %eax           ;\
        jb eax_error            ;\
        cmpl $10, %eax          ;\
        ja eax_error            ;\
        call *sys_jump_table(,%eax,4) ;\
        popfl                   ;\
        popl %esp               ;\
        popl %ebp               ;\
        popl %edi               ;\
        popl %esi               ;\
        popl %edx               ;\
        popl %ecx               ;\
        popl %ebx               ;\
        iret                    ;\
                                ;\
    eax_error:                  ;\
        popfl                   ;\
        popl %esp               ;\
        popl %ebp               ;\
        popl %edi               ;\
        popl %esi               ;\
        popl %edx               ;\
        popl %ecx               ;\
        popl %ebx               ;\
        movl $-1, %eax          ;\
        iret
SYS_LINK(sys_call_linkage);

# jump table for syscalls, 0 is never done
 .global sys_jump_table
 sys_jump_table:
   .long never, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

never:
    ret

# set up and execute the halt system call -- ebx value needs to be preserved before returning
halt:
    pushl %ebx
    call sys_halt
    popl %ebx
    ret

# set up and execute the execute system call -- ebx value needs to be preserved before returning
execute:    
    pushl %ebx
    call sys_execute
    popl %ebx
    ret

# set up and execute the read system call -- save registers 
read:
    pushl %edx
    pushl %ecx
    pushl %ebx
    sti             # should be atomic to avoid interrupts occuring while system reads 
    call sys_read
    cli
    popl %ebx
    popl %ecx
    popl %edx
    ret

# set up and execute the write system call -- save registers 
write:
    pushl %edx
    pushl %ecx
    pushl %ebx
    call sys_write
    popl %ebx
    popl %ecx
    popl %edx
    ret

# set up and execute the open system call -- ebx value needs to be preserved before returning 
open:
    pushl %ebx
    call sys_open
    popl %ebx
    ret

# set up and execute the open system call -- ebx value needs to be preserved before returning 
close:
    pushl %ebx
    call sys_close
    popl %ebx
    ret

# set up and execute the open system call -- ebx and ecx value needs to be preserved before returning 
getargs:
    pushl %ecx 
    pushl %ebx
    call sys_getargs
    popl %ebx
    popl %ecx 
    ret

# set up and execute vidmap -- ebx value needs to be preserved before returning 
vidmap:
    pushl %ebx
    call sys_vidmap
    popl %ebx
    ret

# just return
set_handler:
    ret

# just return
sigreturn:
    ret
    



