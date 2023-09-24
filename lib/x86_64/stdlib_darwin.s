.text
.globl x86_64_libblaze_exit

x86_64_libblaze_exit:
	movq $0x2000001, %rax
	movq $0, %rdi
	syscall
