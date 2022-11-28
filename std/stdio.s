.text
.global write_pl_0
write_pl_0:
    pushq %rbp
    movq %rsp, %rbp

    movq %rdi, %r10

    movq %rdi, %rax  # assume value is in rax
    movq $0, %r9     # result offset

    pushq $0x0a     # push newline
    pushq $0x0d     # push carrige return
    addq $16, %r9

buffer_loop_fu4wyuuysruf:
    movq $10, %rcx   # value to divide by
    movq $0, %rdx    
    div %rcx

    addq $48, %rdx   # convert to ascii

    pushq %rdx       # store the result on the stack
    addq $8, %r9

    cmp $0, %rax
    jnz buffer_loop_fu4wyuuysruf

    test %r10, %r10
    jns add_sign_fwtnbnknmtyf828

    addq $8, %r9
    pushq $45        # add the negative sign

add_sign_fwtnbnknmtyf828:

print_loop_fhtmftu3ufwtt:
    movq $1, %rax    # system call write
    movq $1, %rdi    # file handle
    movq %rsp, %rsi  # message
    movq $1, %rdx    # number of bytes
    syscall

    addq $8, %rsp
    subq $8, %r9
    cmp $0, %r9
    jnz print_loop_fhtmftu3ufwtt

    movq %rbp, %rsp
    popq %rbp
    ret
