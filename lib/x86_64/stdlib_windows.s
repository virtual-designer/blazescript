.text
.globl x86_64_libblaze_exit
.type x86_64_libblaze_exit, @function

x86_64_libblaze_exit:
	xorq %rcx, %rcx
	call ExitProcess

.section .idata
	.import ExitProcess, kernel32
