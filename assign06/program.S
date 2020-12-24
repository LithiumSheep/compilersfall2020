/* 6 vregs used */
	.section .rodata
s_readint_fmt: .string "%ld"
s_writeint_fmt: .string "%ld\n"
	.section .text
	.globl main
main:
	pushq %rbx
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
	subq $80, %rsp
	movq $2, 40(%rsp)           /* ldci vr2, $2 */
	movq 40(%rsp), %r11         /* mov vr0, vr2 */
	movq %r11, 24(%rsp)
	movq $3, 40(%rsp)           /* ldci vr2, $3 */
	movq 40(%rsp), %r11         /* mov vr1, vr2 */
	movq %r11, 32(%rsp)
	movq 24(%rsp), %r11         /* addi vr2, vr0, vr1 */
	movq 32(%rsp), %r10
	addq %r11, %r10
	movq %r10, 40(%rsp)
	movq $s_writeint_fmt, %rdi  /* writei vr2 */
	movq 40(%rsp), %rsi
	call printf
	call foo                    /* call foo */
foo:
	pushq %rbp                  /* funcenter  */
	pushq %rbx
	movq $0, 48(%rsp)           /* ldci vr3, $0 */
	movq $0, 56(%rsp)           /* ldci vr4, $0 */
	movq 48(%rsp), %r11         /* addi vr5, vr3, vr4 */
	movq 56(%rsp), %r10
	addq %r11, %r10
	movq %r10, 64(%rsp)
	popq %rbx                   /* funcleave  */
	popq %rbp
	ret 
	addq $80, %rsp
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %rbx
	movl $0, %eax
	ret
