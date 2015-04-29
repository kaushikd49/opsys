#process_switch_cooperative.s
#parameter 1: previous process registers
#parameter 2: new process registers
#parameter 3: the stack top from the interrupt handler

#------ push order -------
#previous process - ss 184
#previous process - rsp 176
#previous process flags - rflags 168
#previous process - cs 160
#previous process - rip 152
#previous process - rax 144
#previous processs= rbx 136
#previous process - rcx 128
#previous process - rdx 120
#previous process - rdi 112
#previous process - rsi 104
#previous process - rbp 96
#previous process - r8 88
#previous process - r9 80
#previous process - r10 72
#previous process - r11 64
#previous process - r12 56
#previous process - r13 48
#previous process - r14 40(%rdx)
#previous process - r15 32at parameter 3, i.e, rdx
#previous process - ds 24
#previous process - es 16
#previous process - fs 8(%rdx)
#previous process - gs at parameter 3, i.e, rdx

#---- order in state structure:
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
#kernel_rsp 200
.text
.global process_switch_cooperative

process_switch_cooperative:
	cli
	movq 176(%rdx), %rax # storing rsp of the previous process into the from register
	movq %rax, (%rdi) # which is at offset 0 from the beginning of the state
	#####################
	movq 152(%rdx), %rax # storing rip of the previous process into the from register
	movq %rax, 8(%rdi) # which is at offset 8 from the beginning of the state
	######################
	movq 144(%rdx), %rax # rax
	movq %rax, 16(%rdi)
	######################
	movq 136(%rdx), %rax #rbx
	movq %rax, 24(%rdi)
	#####################
	movq 128(%rdx), %rax #rcx
	movq %rax, 32(%rdi)
	######################
	movq 120(%rdx), %rax #rdx
	movq %rax, 40(%rdi)
	######################
	movq 112(%rdx), %rax #rdi
	movq %rax, 48(%rdi)
	#######################
	movq 104(%rdx), %rax #rsi
	movq %rax, 56(%rdi)
	########################
	movq 96(%rdx), %rax #rbp
	movq %rax, 64(%rdi)
	######################
	movq 88(%rdx), %rax #r8
	movq %rax, 72(%rdi)
	#####################
	movq 80(%rdx), %rax #r9
	movq %rax, 80(%rdi)
	#####################
	movq 72(%rdx), %rax #r10
	movq %rax, 88(%rdi)
	######################
	movq 64(%rdx), %rax #r11
	movq %rax, 96(%rdi)
	####################
	movq 56(%rdx), %rax #r12
	movq %rax, 104(%rdi)
	#####################
	movq 48(%rdx), %rax #r13
	movq %rax, 112(%rdi)
	####################
	movq 40(%rdx), %rax #r14
	movq %rax, 120(%rdi)
	####################
	movq 32(%rdx), %rax #r15
	movq %rax, 128(%rdi)
	####################
	movq %cr3, %rax #cr3
	movq %rax, 136(%rdi)
	######################
 	movq 168(%rdx), %rax
 	movq %rax, 144(%rdi) #rflags
 	######################
	movq 24(%rdx), %rax #ds
	movq %rax, 160(%rdi)
	######################
	movq 16(%rdx), %rax #es
	movq %rax, 168(%rdi)
	#####################
	movq 8(%rdx), %rax #fs
	movq %rax, 176(%rdi)
	####################
	movq (%rdx), %rax #gs
	movq %rax, 184(%rdi)
	######################
	movq 160(%rdx), %rax #cs
	movq %rax, 152(%rdi)
	######################
	movq 184(%rdx), %rax #ss
	movq %rax, 192(%rdi)
	######################
	#######################
	movq %rdx, %rax
	addq $192, %rax
	movq %rax, 200(%rdi)
	######################
	#######################
	movq 192(%rsi), %rax
	movq %rax, 184(%rdx) # setting ss in the stack
	###################
	movq 152(%rsi), %rax
	movq %rax, 160(%rdx) # cs
	####################
	movq 184(%rsi), %rax
	movq %rax, (%rdx)  #gs
	######################
	movq 176(%rsi), %rax
	movq %rax, 8(%rdx) #fs
	######################
	movq 168(%rsi), %rax
	movq %rax, 16(%rdx) #es
	######################
	movq 160(%rsi), %rax
	movq %rax, 24(%rdx) #ds
	#######################
	movq 144(%rsi), %rax
	orq $0x200,  %rax
	movq %rax, 168(%rdx) #rflags
	#######################
	movq 136(%rsi), %rax
	movq %rax, %cr3 #cr3
	#######################
	movq 128(%rsi), %rax
	movq %rax, 32(%rdx) #r15
	########################
	movq 120(%rsi), %rax
	movq %rax, 40(%rdx) #r14
	#########################
	movq 112(%rsi), %rax
	movq %rax, 48(%rdx) #r13
	########################
	movq 104(%rsi), %rax
	movq %rax, 56(%rdx) #r12
	#########################
	movq 96(%rsi), %rax
	movq %rax, 64(%rdx) #r11
	#########################
	movq 88(%rsi), %rax
	movq %rax, 72(%rdx) #r10
	#########################
	movq 80(%rsi), %rax
	movq %rax, 80(%rdx) #r9
	########################
	movq 72(%rsi), %rax
	movq %rax, 88(%rdx) #r8
	########################
	movq 64(%rsi), %rax
	movq %rax, 96(%rdx) #rbp
	########################
	movq 56(%rsi), %rax
	movq %rax, 104(%rdx) #rsi
	########################
	movq 48(%rsi), %rax
	movq %rax, 112(%rdx) #rdi
	########################
	movq 40(%rsi), %rax
	movq %rax, 120(%rdx) #rdx
	########################
	movq 32(%rsi), %rax
	movq %rax, 128(%rdx) #rcx
	########################
	movq 24(%rsi), %rax
	movq %rax, 136(%rdx) #rbx
	########################
	movq 16(%rsi), %rax
	movq %rax, 144(%rdx) #rax
	########################
	movq 8(%rsi), %rax
	movq %rax, 152(%rdx) #rip
	########################
	movq (%rsi), %rax
	movq %rax, 176(%rdx) #rsp
	##########################
	retq
