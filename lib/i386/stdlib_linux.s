.text
.globl i386_libblaze_exit
.type i386_libblaze_exit, @function

i386_libblaze_exit:
	movl $1, %eax
	movl $0, %ebx
	int $0x80
	
.section .note.GNU-stack,"",%progbits
