/*filename: isr_timer.s*/

.global isr_default
.align 4

isr_default:
	movb $0x20, %al
	outb  %al, $0x20
	pushq %rax
	pushq %rcx
	pushq %rdx
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call isrhandler_default
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rdx
	popq %rcx
	popq %rax
	iretq
