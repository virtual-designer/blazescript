.data
stdout_fd: 
    .quad 0

.text

/**
 * Puts a character and returns. Does not set the frame pointer.
 *
 * %rdi - the character to print
 */
x86_64_libblaze_putc:
    pushq %rdi
    pushq %rsi
    pushq %rdx
    pushq %rax
    movq stdout_fd(%rip), %rax
    cmpq $0, rax
    jne .x86_64_libblaze_putc.write
    movq $2, %rax
    movq $-11, %rcx
    call GetStdHandle
    movq %rax, stdout_fd(%rip)
.x86_64_libblaze_putc.write:
    movq %rax, %rdi
    movq $0x40, %rax
    leaq -16(%rbp), %rsi
    xor %r8, %r8
    xor %rcx, %rcx
    syscall
    popq %rax
    popq %rsi
    popq %rdi
    popq %rdi
    ret

/**
 * Puts a character to stdout.
 *
 * %rdi - the character to print
 */
.globl x86_64_libblaze_putchar
.type x86_64_libblaze_putchar, @function
x86_64_libblaze_putchar:
    pushq %rbp
    movq %rsp, %rbp
    call x86_64_libblaze_putc
    movq %rbp, %rsp
    popq %rbp
    ret

/**
 * Puts a string to stdout, without a following newline.
 *
 * %rdi - pointer to the string to print
 */
.globl x86_64_libblaze_putstr
.type x86_64_libblaze_putstr, @function
x86_64_libblaze_putstr:
    pushq %rbp
    movq %rsp, %rbp
    movq %rdi, %rbx
x86_64_libblaze_putstr.loop:
    movb (%rbx), %al
    cmpb $0, %al
    je x86_64_libblaze_putstr.end
    movzx %al, %rdi
    call x86_64_libblaze_putc
    inc %rbx
    jmp x86_64_libblaze_putstr.loop
x86_64_libblaze_putstr.end:
    movq %rbp, %rsp
    popq %rbp
    ret

/**
 * Puts a string to stdout, with a following newline.
 *
 * %rdi - pointer to the string to print
 */
.globl x86_64_libblaze_puts
.type x86_64_libblaze_puts, @function
x86_64_libblaze_puts:
    pushq %rbp
    movq %rsp, %rbp
    call x86_64_libblaze_putstr
    movq $10, %rdi
    call x86_64_libblaze_putchar
    movq %rbp, %rsp
    popq %rbp
    ret

.section .idata
    .import GetStdHandle, kernel32
    .import WriteFile, kernel32
