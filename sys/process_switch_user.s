#process_switch_user.s
#parameter 1: previous task register address
#parameter 2: new task register address
#
#--------- the push order --------
#rax +136
#rsp +128
#rbx +120
#rcx +112
#rdx +104
#rdi +96
#rsi +88
#rbp +80
#r8 +72
#r9 +64
#r10 +56
#r11 +48
#r12 +40
#r13 +32
#r14 +24
#r15 +16
#flags +8
#cr3 +0
#--------- register struct order -----------
# rsp 0
#rip 8
#rax 16
#rbx 24
#rcx 32
#rdx 40
#rdi 48
#rsi 56
#rbp 64
#r8 72
#r9 80
#r10 88
#r11 96
#r12 104
#r13 112
#r14 120
#r15 128
#cr3 136
#flags 144
#cs 152
#ds 160
#es 168
#fs 176
#gs 184
#ss 192
.text
.global process_switch_user
#reference: idea taken from here: http://wiki.osdev.org/Kernel_Multitasking
process_switch_user:
	cli
	# current registers at rdi and new registers at rsi
	#return address at +144
	pushq %rax # +136
	movq %rsp, %rax
	addq $16, %rax # 8 for rax and 8 for the return addr so the top of the stack is 2 words away
	push %rax #pushing %RSP + 128
	pushq %rbx #pushing %RBX + 120
	pushq %rcx # pushing RCX + 112
	pushq %rdx # pushing RDX + 104
	pushq %rdi # pushing RDI + 96
	pushq %rsi # pushing RSI + 88
	pushq %rbp # pushing RBP + 80
	pushq %r8 # pushing r8 + 72
	pushq %r9 # pushing r9 + 64
	pushq %r10 #pushing r10 + 56
	pushq %r11 # pushing r11 + 48
	pushq %r12 #pushing r12 + 40
	pushq %r13 # pushing r13 + 32
	pushq %r14 #pushing r 14 + 24
	pushq %r15 #pushing r15 at +16
	pushfq # pushing flags  + 8
	movq %cr3, %rax
	pushq %rax #pushing cr3 at rsp
	movq %rdi, %rax # getting the current register structure and deferencing rax gives the location of the register info
	movq %rbx, 24(%rax) # moving the value of rbx to correct location checked
	movq %rcx, 32(%rax) # moving rcx to correct location checked
	movq %rdx, 40(%rax) # moving rdx to correct location checked
	movq %rdi, 48(%rax) # moving rdi to correct location checked
	movq %rsi, 56(%rax) # moving rcx to correct location checked
	movq %rbp, 64(%rax) # moving rbp to correct location checked
	movq %r8, 72(%rax) # moving r8 to correct location checked
	movq %r9, 80(%rax) # moving r9 to correct location checked
	movq %r10, 88(%rax) # moving r10 to correct location checked
	movq %r11, 96(%rax) # moving r11 to correct location checked
	movq %r12, 104(%rax) # moving r12 to correct location checked
	movq %r13, 112(%rax) # moving r13 to correct location checked
	movq %r14, 120(%rax) # moving r14 to correct location checked
	movq %r15, 128(%rax) # moving r15 to correct location checked
	movw %cs, 152(%rax) # storing the current value of the 6 segment registers
	movw %ds, 160(%rax)
	movw %es, 168(%rax)
	movw %fs, 176(%rax)
	movw %gs, 184(%rax)
	movw %ss, 192(%rax)
	movq 136(%rsp), %rbp #using rbp to store rax since we have already stored rbp
	movq 144(%rsp), %rcx #using rcx to store rip rip is the return address that is stored on the stack when the interrupt handler is called
	movq 128(%rsp), %rdx #using rdx to store rsp that was calculated and storing it in rdx.
	movq 8(%rsp), %rdi # rdi contains flags that can be used
	movq %rbx, 16(%rax) # storing rax in he right location in register ( current process state)
	movq %rdx, (%rax) # storing rsp in the correct location in registers
	movq %rcx, 8(%rax) # rip is moved to the correct location in the registers
	movq %rdi, 144(%rax) # flags are stored in the correct location in the registers
	popq %rbx # this is getting the cr3 we stored
	movq %rbx, 136(%rax) #cr3 is stored in the correct location
	pushq %rbx # putting cr3 back where it belongs
	movq %rsi, %rax # preparing for the new register values, the new register address is passed in rsi
	movq 24(%rax), %rbx #
	movq 32(%rax), %rcx #
	movq 40(%rax), %rdx #
	movq 48(%rax), %rdi #
	movq 56(%rax), %rsi #
	movq 64(%rax), %rbp #
	movq 72(%rax), %r8 #
	movq 80(%rax), %r9 #
	movq 88(%rax), %r10 #
	movq 96(%rax), %r11 #
	movq 104(%rax), %r12 #
	movq 112(%rax), %r13 #
	movq 120(%rax), %r14 #
	movq 128(%rax), %r15 #
	movw 160(%rax), %ds #these registers need to be updated for user switch
	movw 168(%rax), %es
	movw 176(%rax), %fs
	movw 184(%rax), %gs
	pushq %rax
	push 192(%rax)
	pushq (%rax)
	movq 144(%rax), %rax
	orq $0x200,  %rax
	pushq %rax
	movq 24(%rsp), %rax
	push 152(%rax)
	push %rax
	movq 8(%rax), %rax
	xchg (%rsp), %rax
	movq 16(%rax), %rax
	iretq
