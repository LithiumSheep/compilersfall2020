/* 4 vregs used */
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
	subq $64, %rsp
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
	call incr_var_a             /* call incr_var_a */
	movq 24(%rsp), %r11         /* addi vr3, vr0, vr1 */
	movq 32(%rsp), %r10
	addq %r11, %r10
	movq %r10, 48(%rsp)
	movq $s_writeint_fmt, %rdi  /* writei vr3 */
	movq 48(%rsp), %rsi
	call printf
incr_var_a:
	pushq %rbp                  /* funcenter  */
	pushq %rbx
	movq $0, 40(%rsp)           /* ldci vr2, $0 */
	movq 24(%rsp), %r11         /* addi vr3, vr0, vr2 */
	movq 40(%rsp), %r10
	addq %r11, %r10
	movq %r10, 48(%rsp)
	movq 48(%rsp), %r11         /* mov vr0, vr3 */
	movq %r11, 24(%rsp)
	popq %rbp                   /* funcleave  */
	popq %rbx
	ret 
	addq $64, %rsp
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %rbx
	movl $0, %eax
	ret
