    .globl reverse
    .text
reverse:
    pushq %rbp
    movq %rsp, %rbp
    subq $112, %rsp
    movq %rdi, -8(%rbp)
    movq -8(%rbp), %r10
    movq %r10, -16(%rbp)
    movq -8(%rbp), %r10
    movq %r10, -24(%rbp)
    .Lcontinue_label.0:
    movq -24(%rbp), %rax
    movb 0(%rax), %r10b
    movb %r10b, -25(%rbp)
    movsbl -25(%rbp), %r11d
    movl %r11d, -32(%rbp)
    cmpl $0, -32(%rbp)
    movl $0, -36(%rbp)
    setne -36(%rbp)
    cmpl $0, -36(%rbp)
    je .Lbreak_label.0
    movq -24(%rbp), %r10
    movq %r10, -48(%rbp)
    movq -24(%rbp), %r10
    movq %r10, -56(%rbp)
    addq $1, -56(%rbp)
    movq -56(%rbp), %r10
    movq %r10, -24(%rbp)
    jmp .Lcontinue_label.0
    .Lbreak_label.0:
    movq -24(%rbp), %r10
    movq %r10, -64(%rbp)
    movq -24(%rbp), %r10
    movq %r10, -72(%rbp)
    subq $1, -72(%rbp)
    movq -72(%rbp), %r10
    movq %r10, -24(%rbp)
    .Lcontinue_label.1:
    movq -24(%rbp), %r10
    cmpq %r10, -16(%rbp)
    movl $0, -76(%rbp)
    setb -76(%rbp)
    cmpl $0, -76(%rbp)
    je .Lbreak_label.1
    movq -16(%rbp), %rax
    movb 0(%rax), %r10b
    movb %r10b, -77(%rbp)
    movb -77(%rbp), %r10b
    movb %r10b, -78(%rbp)
    movq -24(%rbp), %rax
    movb 0(%rax), %r10b
    movb %r10b, -79(%rbp)
    movq -16(%rbp), %rax
    movb -79(%rbp), %r10b
    movb %r10b, 0(%rax)
    movq -24(%rbp), %rax
    movb -78(%rbp), %r10b
    movb %r10b, 0(%rax)
    movq -16(%rbp), %r10
    movq %r10, -88(%rbp)
    movq -16(%rbp), %r10
    movq %r10, -96(%rbp)
    addq $1, -96(%rbp)
    movq -96(%rbp), %r10
    movq %r10, -16(%rbp)
    movq -24(%rbp), %r10
    movq %r10, -104(%rbp)
    movq -24(%rbp), %r10
    movq %r10, -112(%rbp)
    subq $1, -112(%rbp)
    movq -112(%rbp), %r10
    movq %r10, -24(%rbp)
    jmp .Lcontinue_label.1
    .Lbreak_label.1:
    movl $0, %eax
    movq %rbp, %rsp
    popq %rbp
    ret

    .globl main
    .text
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $272, %rsp
    leaq string.0(%rip), %r11
    movq %r11, -8(%rbp)
    movq -8(%rbp), %rdi
    call printf@PLT
    movl %eax, -12(%rbp)
    leaq string.1(%rip), %r11
    movq %r11, -24(%rbp)
    leaq -224(%rbp), %r11
    movq %r11, -232(%rbp)
    movq -24(%rbp), %rdi
    movq -232(%rbp), %rsi
    call scanf@PLT
    movl %eax, -236(%rbp)
    leaq -224(%rbp), %r11
    movq %r11, -248(%rbp)
    movq -248(%rbp), %rdi
    call reverse
    leaq string.2(%rip), %r11
    movq %r11, -256(%rbp)
    leaq -224(%rbp), %r11
    movq %r11, -264(%rbp)
    movq -256(%rbp), %rdi
    movq -264(%rbp), %rsi
    call printf@PLT
    movl %eax, -268(%rbp)
    movl $0, %eax
    movq %rbp, %rsp
    popq %rbp
    ret
    movl $0, %eax
    movq %rbp, %rsp
    popq %rbp
    ret

string.2:
    .ascii "Reversed: %s\n"
    .byte 0

string.1:
    .ascii "%s"
    .byte 0

string.0:
    .ascii "Enter a string: "
    .byte 0

    .section .note.GNU-stack,"",@progbits
