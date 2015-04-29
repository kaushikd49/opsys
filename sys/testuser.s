.text
.global _jump_usermode

_jump_usermode:
     movq $0x23, %rax
     movq %rax,%ds
     movq %rax,%es
     movq %rax,%fs
     movq %rax,%gs

     movq %rax,%rsp
     pushq $0x23
     pushq %rax
     pushf
     pushq $0x1B
     push test_user_function
     iret
