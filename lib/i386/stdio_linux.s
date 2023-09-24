.text

/**
 * Puts a character and returns. Does not set the frame pointer.
 *
 * %edx - the character to print
 */
i386_libblaze_putc:
    push %eax
    push %ebx
    push %ecx
    push %edx
    movl $4, %eax
    movl $1, %ebx
    leal (%esp), %ecx
    movl $1, %edx
    int $0x80
    pop %edx
    pop %ecx
    pop %ebx
    pop %eax
    ret

/**
 * Puts a character to stdout.
 *
 * stack[0] - the character to print
 */
.globl i386_libblaze_putchar
.type i386_libblaze_putchar, @function
i386_libblaze_putchar:
    push %ebp
    mov %esp, %ebp
    mov 8(%ebp), %edx
    call i386_libblaze_putc
    mov %ebp, %esp
    pop %ebp
    ret

/**
 * Puts a string to stdout, without a following newline.
 *
 * stack[0] - pointer to the string to print
 */
.globl i386_libblaze_putstr
.type i386_libblaze_putstr, @function
i386_libblaze_putstr:
    push %ebp
    mov %esp, %ebp
    push %eax
    push %ebx
    mov 8(%ebp), %ebx
i386_libblaze_putstr.loop:
    movb (%ebx), %al
    cmpb $0, %al
    je i386_libblaze_putstr.end
    movzx %al, %edx
    call i386_libblaze_putc
    inc %ebx
    jmp i386_libblaze_putstr.loop
i386_libblaze_putstr.end:
    pop %ebx
    pop %eax
    mov %ebp, %esp
    pop %ebp
    ret

/**
 * Puts a string to stdout, with a following newline.
 *
 * stack[0] - pointer to the string to print
 */
.globl i386_libblaze_puts
.type i386_libblaze_puts, @function
i386_libblaze_puts:
    push %ebp
    mov %esp, %ebp
    pushl 8(%ebp)
    call i386_libblaze_putstr
    add $4, %esp
    mov $10, %edx
    call i386_libblaze_putc
    mov %ebp, %esp
    pop %ebp
    ret

.section .note.GNU-stack,"",%progbits
