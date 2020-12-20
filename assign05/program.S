/* 3 vregs used */
	.section .rodata
s_readint_fmt: .string "%ld"
s_writeint_fmt: .string "%ld\n"
	.section .text
	.globl main
main:
	subq $40, %rsp
	movq $2, %r11               /* mov vr0, $2 */
	movq %r11, 16(%rsp)
	movq $3, %r11               /* mov vr1, $3 */
	movq %r11, 24(%rsp)
	movq 16(%rsp), %r11         /* addi vr2, vr0, vr1 */
	movq 24(%rsp), %r10
	addq %r11, %r10
	movq %r10, 32(%rsp)
	movq $s_writeint_fmt, %rdi  /* writei vr2 */
	movq 32(%rsp), %rsi
	call printf
	addq $40, %rsp
	movl $0, %eax
	ret
