/*filename: isr_timer.s*/
.text
.global isr_default
.global isr_timer
.global isr_keyboard
.global keyboard_init
.global trap_default
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

trap_default:

	pushq %rax
	pushq %rcx
	pushq %rdx
	pushq %r8
	pushq %r9
	pushq %r10
	pushq %r11
	call traphandler_default
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
	#inb $0x60, %al
	#movq $0xb8000, %rbx
	#movb %al, (%rbx)
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
	retq
