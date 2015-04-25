/*filename: isr_timer.s*/
.text
.global isr_default
.global isr_timer
.global isr_keyboard
.global keyboard_init
.global trap_default
.global trap_one
.global trap_two
.global trap_three
.global trap_four
.global trap_five
.global trap_six
.global trap_seven
.global trap_eight
.global trap_nine
.global trap_ten
.global trap_eleven
.global trap_twelve
.global trap_thirteen
.global trap_fourteen
.global trap_fifteen
.global trap_sixteen
.global trap_seventeen
.global trap_eighteen
.global trap_nineteen
.global trap_twenty
.global trap_twentyone
.global trap_twentytwo
.global trap_twentythree
.global trap_twentyfour
.global trap_twentyfive
.global trap_twentysix
.global trap_twentyseven
.global trap_twentyeight
.global trap_twentynine
.global trap_thirty
.global trap_thirtyone
.global isr_syscall
.global isr_fake_preempt
.align 4

isr_default:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call isrhandler_default
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq

trap_default:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_default
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq
trap_one:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_one
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq
trap_two:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_two
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq
trap_three:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_three
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq
trap_four:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_four
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq
trap_five:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_five
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq

trap_six:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_six
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq

trap_seven:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_seven
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq


trap_eight:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_eight
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq
trap_nine:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_nine
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq
trap_ten:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_ten
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq
trap_eleven:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_eleven
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq
trap_twelve:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_twelve
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq

trap_thirteen:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_thirteen
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx
	popq %rax

	// Pop the stack top to clear the errorcode
	xchg (%rsp), %rax
	popq %rax

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq

trap_fourteen:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_fourteen
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx
	popq %rax

	// Pop the stack top to clear the errorcode
	// Need to do this for interrupts that push
	// errorcodes before call to handler, else
	// GeneralProtectionFault happens
	xchg (%rsp), %rax


	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq

trap_fifteen:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_fifteen
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq

trap_sixteen:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_sixteen
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq

trap_seventeen:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_seventeen
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq
trap_eighteen:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_eighteen
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq
trap_nineteen:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_nineteen
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq

trap_twenty:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_twenty
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq
trap_twentyone:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_twentyone
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq

trap_twentytwo:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_twentytwo
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq

trap_twentythree:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_twentythree
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq

trap_twentyfour:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_twentyfour
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq

trap_twentyfive:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_twentyfive
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq

trap_twentysix:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_twentysix
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq

trap_twentyseven:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_twentyseven
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq

trap_twentyeight:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_twentyeight
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq

trap_twentynine:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_twentynine
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq

trap_thirty:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_thirty
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq

trap_thirtyone:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_thirtyone
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	popq %rax
	sti
	iretq




.text


isr_timer:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
	movq $0, %rax
	movq %ds, %rax
	pushq %rax
	movq $0, %rax
	movq %es, %rax
	pushq %rax
	movq $0, %rax
	movq %fs, %rax
	pushq %rax
	movq $0, %rax
	movq %gs, %rax
	pushq %rax
	movq %rsp, %rdi
	call temp_print_time
	movq %rax, %rsp
	popq %rax
	movq %rax, %gs
	popq %rax
	movq %rax, %fs
	popq %rax
	movq %rax, %es
	popq %rax
	movq %rax, %ds
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb %al, $0x20
	popq %rax
	sti
	iretq



.text

isr_keyboard:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	#inb $0x60, %al
	#movq $0xb8000, %rbx
	#movb %al, (%rbx)
	call isrhandler_keyboard
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb %al, $0x20
	popq %rax
	sti
	iretq


.text

isr_syscall:
	cli
	pushq %rax #144
	pushq %rbx #136
	pushq %rcx #128
	pushq %rdx #120
	pushq %rdi #112
	pushq %rsi #104
	pushq %rbp #96
	pushq %r8 #88
	pushq %r9 #80
	pushq %r10 #72
	pushq %r11 #64
	pushq %r12 #56
	pushq %r13 #48
	pushq %r14 #40
	pushq %r15 #32
	movq $0, %rax
	movq %ds, %rax
	pushq %rax #24
	movq $0, %rax
	movq %es, %rax
	pushq %rax #16
	movq $0, %rax
	movq %fs, %rax
	pushq %rax  #8
	movq $0, %rax
	movq %gs, %rax
	pushq %rax  ##pushed all registers here
    movq $60, %rbx
    movq 144(%rsp), %rax
	cmp %rbx, %rax
	jne q2d
	call handle_exit
	movq %rax, %rsp
	jmp q3
q2d:movq $0, %rbx
	movq 144(%rsp), %rax
	cmp %rbx, %rax
	jne q2e
	call handle_read
	movq %rax, %rsp
	jmp q3
q2e:movq $61, %rbx
	movq 144(%rsp), %rax
	cmp %rbx, %rax
	jne q2f
	call handle_wait
	movq %rax, %rsp
	jmp q3
q2f:movq $35, %rbx
	movq 144(%rsp), %rax
	cmp %rbx, %rax
	jne q2
	call handle_nanosleep
	movq %rax, %rsp
	jmp q3
 q2:call handle_syscall
 q3:movq %rax, %rbx
 	popq %rax #add more syscalls here
	movq %rax, %gs
	popq %rax
	movq %rax, %fs
	popq %rax
	movq %rax, %es
	popq %rax
	movq %rax, %ds
	popq %r15
	popq %r14
	popq %r13
	popq %r12
 	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rax#actually rbx
	xchgq %rbx, %rax
	add $8, %rsp
	pushq %rax
	#popq %rax
	movb $0x20, %al
	outb %al, $0x20
	popq %rax
	sti
	iretq
#init steps mentioned in http://wiki.osdev.org/%228042%22_PS/2_Controller#Translation
keyboard_init:
	cli
	movb $0xad, %al
	outb %al, $0x64
	movb $0xa7, %ah
	outb %al, $0x64  #step 2:
	inb $0x60, %al   #step 3: flushing buffer
	movb $0x20, %al
	outb %al, $0x64   #step 4: read confg byte
	inb $0x60,%al
	andb $0xbc,%al   #5:set config byte, might be useful to check if bit
	movb %al,%bl
 	movb $0x60,%al
	outb %al, $0x64
	movb %bl, %al
	outb %al, $0x60
	#controller work check
	movb $0xaa, %al   #6:controller self test
	outb %al, $0x64
	inb $0x60, %al
	#movq $0xb8000, %rbx
	#movb %al, (%rbx)
	#port 1 check
	movb $0xab, %al   #6:controller self test
	outb %al, $0x64
	inb $0x60, %al
	movb $0x00, %bl
	cmp %al,%bl
	jne s9
	#movq $0xb8008, %rbx
	#movb $0x65, (%rbx)
 s9:movb $0xae, %al   #6:controller self test
	outb %al, $0x64
	movb $0x60, %al
	outb %al, $0x64
	movb $0x01, %al
	outb %al, $0x60
	movb $0xff, %al
	outb %al, $0x60
	inb $0x60, %al
	movb $0xFA, %bl
	cmp %al, %bl
	jne exit
	#movq $0xb8016, %rbx
	#movb $0x75, (%rbx)
exit:
	sti
	retq


isr_fake_preempt:
	cli
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %rsi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
	movq $0, %rax
	movq %ds, %rax
	pushq %rax
	movq $0, %rax
	movq %es, %rax
	pushq %rax
	movq $0, %rax
	movq %fs, %rax
	pushq %rax
	movq $0, %rax
	movq %gs, %rax
	pushq %rax
	movq %rsp, %rdi
	call handle_fake_preempt
	movq %rax, %rsp
	popq %rax
	movq %rax, %gs
	popq %rax
	movq %rax, %fs
	popq %rax
	movq %rax, %es
	popq %rax
	movq %rax, %ds
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rbp
	popq %rsi
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rbx

	movb $0x20, %al
	outb %al, $0x20
	popq %rax
	sti
	iretq
