.text
.globl x86_64_libblaze_exit
.type x86_64_libblaze_exit, @function

x86_64_libblaze_exit:
	movq $60, %rax
	movq $0, %rdi
	syscall

.section .note.GNU-stack,"",%progbits
