
.globl sys_execute, SYS_HALT_RET

# sys_execute pushed stack and cmd argument
sys_execute:
    pushl %ebp
    pushl %esp
    pushl %ebx
    call sys_execute_setup
    popl %ebx
    popl %esp
    popl %ebp
    ret

# return from parent process execute
SYS_HALT_RET:
    popl %ebx
    ret


    





 








