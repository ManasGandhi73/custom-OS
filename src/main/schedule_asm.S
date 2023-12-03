.global testreturn, pit_handler_setup

# pushes needed stack, calls pit handler
pit_handler_setup:
    pushl %ebp 
    pushl %esp 
    call pit_handler
    popl %esp
    popl %ebp
    ret

# skip return from pit handler, set ebp and esp to correct kernel stack values for process
testreturn:
    addl $4, %esp
    ret





