    .section .rodata
    .align 8
.LC0:
    .quad 4618441417868443648

    .globl main
    .text
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $176, %rsp
    leaq -8(%rbp), %r11
    movq %r11, -16(%rbp)
    movl $5, %r10d
    movslq %r10d, %r11
    movq %r11, -24(%rbp)
    movq -16(%rbp), %rax
    movq -24(%rbp), %rdx
    leaq (%rax,%rdx,8), %r11
    movq %r11, -32(%rbp)
    movq -32(%rbp), %r10
    movq %r10, -40(%rbp)
    leaq -8(%rbp), %r11
    movq %r11, -48(%rbp)
    movq -48(%rbp), %rax
    leaq -40(%rax), %r11
    movq %r11, -56(%rbp)
    movq -56(%rbp), %r10
    movq %r10, -64(%rbp)
    leaq -8(%rbp), %r11
    movq %r11, -72(%rbp)
    movl $5, %r11d
    movq %r11, -80(%rbp)
    movq -72(%rbp), %rax
    movq -80(%rbp), %rdx
    leaq (%rax,%rdx,8), %r11
    movq %r11, -88(%rbp)
    movq -88(%rbp), %r10
    movq %r10, -96(%rbp)
    leaq -8(%rbp), %r11
    movq %r11, -104(%rbp)
    movq $5, -112(%rbp)
    movq -104(%rbp), %rax
    movq -112(%rbp), %rdx
    leaq (%rax,%rdx,8), %r11
    movq %r11, -120(%rbp)
    movq -120(%rbp), %r10
    movq %r10, -128(%rbp)
    movq -64(%rbp), %r10
    cmpq %r10, -40(%rbp)
    movl $0, -132(%rbp)
    sete -132(%rbp)
    cmpl $0, -132(%rbp)
    je .Llabel.4
    movq -96(%rbp), %r10
    cmpq %r10, -40(%rbp)
    movl $0, -136(%rbp)
    sete -136(%rbp)
    cmpl $0, -136(%rbp)
    je .Llabel.4
    movl $1, -140(%rbp)
    jmp .Llabel.5
    .Llabel.4:
    movl $0, -140(%rbp)
    .Llabel.5:
    cmpl $0, -140(%rbp)
    je .Llabel.2
    movq -128(%rbp), %r10
    cmpq %r10, -40(%rbp)
    movl $0, -144(%rbp)
    sete -144(%rbp)
    cmpl $0, -144(%rbp)
    je .Llabel.2
    movl $1, -148(%rbp)
    jmp .Llabel.3
    .Llabel.2:
    movl $0, -148(%rbp)
    .Llabel.3:
    cmpl $0, -148(%rbp)
    je .Llabel.0
    movq -128(%rbp), %rax
    movsd 0(%rax), %xmm14
    movsd %xmm14, -160(%rbp)
    movsd -160(%rbp), %xmm15
    comisd .LC0(%rip), %xmm15
    movl $0, -164(%rbp)
    sete -164(%rbp)
    cmpl $0, -164(%rbp)
    je .Llabel.0
    movl $1, -168(%rbp)
    jmp .Llabel.1
    .Llabel.0:
    movl $0, -168(%rbp)
    .Llabel.1:
    cmpl $0, -168(%rbp)
    movl $0, -172(%rbp)
    sete -172(%rbp)
    movl -172(%rbp), %eax
    movq %rbp, %rsp
    popq %rbp
    ret
    movl $0, %eax
    movq %rbp, %rsp
    popq %rbp
    ret

    .section .note.GNU-stack,"",@progbits
