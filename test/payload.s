.text
.globl _start
_start:
  #save the base pointer
  pushq %rbp
  pushq %rbx
  mov %rsp,%rbp

  #write syscall = 1
  movq $1, %rax
  #print to stdout
  movq $1, %rdi
  #9 character long string
  movq $9, %rdx

  movq $0x0a, %rcx
  pushq %rcx
  movq $0x44455443454a4e49, %rcx
  pushq %rcx
  movq %rsp, %rsi

  syscall

  #remove the string
  pop %rcx
  pop %rcx

  movq $0, %rax
  movq $0, %rdi
  movq $0, %rdx

  
  pop %rbx
  pop %rbp
  ret
