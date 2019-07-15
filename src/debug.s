	.file	"debug.c"
	.text
	.p2align 4,,15
	.globl	func
	.type	func, @function
func:
.LFB0:
	.cfi_startproc
	movl	$10, %eax
	ret
	.cfi_endproc
.LFE0:
	.size	func, .-func
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB3:
	.cfi_startproc
	movl	$10, %eax
	ret
	.cfi_endproc
.LFE3:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 7.4.0-1ubuntu1~18.04.1) 7.4.0"
	.section	.note.GNU-stack,"",@progbits
