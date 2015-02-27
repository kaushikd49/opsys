/*filename: isr_timer.s*/
.text
.global isr_default
.global isr_timer
.global isr_keyboard
.align 4

isr_default:

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
	movb $0x20, %al
	outb  %al, $0x20  # see whether this has to be in the beginning or the end.
	iretq





.text


isr_timer:
	pushq %rax
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call print_time
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rax
	movb $0x20, %al
	outb %al, $0x20
	iretq


.text

isr_keyboard:
	pushq %rax
	pushq %rcx
	pushq %rdx
	pushq %rdi
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call isrhandler_keyboard
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rdi
	popq %rdx
	popq %rcx
	popq %rax
	movb $0x20, %al
	outb %al, $0x20
	iretq

#init steps mentioned in http://wiki.osdev.org/%228042%22_PS/2_Controller#Translation
keyboard_init:
	movb 0xad, %al
	outb %al, $0x64
	movb $0xa7, %ah
	outb %al, $0x64  #step 2:
	inb $0x60, %al   #step 3: flushing buffer
	movb $0x20, %al
	outb %al, $0x64   #step 4: read confg byte
	inb $0x60,%al
	andb $0xbc,%al   #5:set config byte, might be useful to check if bit
