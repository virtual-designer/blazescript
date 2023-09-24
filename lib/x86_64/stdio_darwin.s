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
    movq $0x2000004, %rax
    movq $1, %rdi
    leaq -16(%rbp), %rsi
    movq $1, %rdx
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
x86_64_libblaze_puts:
    pushq %rbp
    movq %rsp, %rbp
    call x86_64_libblaze_putstr
    movq $10, %rdi
    call x86_64_libblaze_putchar
    movq %rbp, %rsp
    popq %rbp
    ret
