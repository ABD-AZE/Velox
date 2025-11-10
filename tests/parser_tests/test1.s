
    .bss
    .align 4
string.0:
    .zero 4

    .globl main
    .text
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $0, %rsp
    movl $0, %eax
    movq %rbp, %rsp
    popq %rbp
    ret
    movl $0, %eax
    movq %rbp, %rsp
    popq %rbp
    ret

    .section .note.GNU-stack,"",@progbits
