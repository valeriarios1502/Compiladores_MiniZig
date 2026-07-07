.data
print_int_fmt: .string "%ld\n"
print_str_fmt: .string "%s\n"

.text
.globl control
control:
    pushq %rbp
    movq %rsp, %rbp
    subq $64, %rsp
  movq $0, %rax
movq %rax, -8(%rbp)
    movq -8(%rbp), %rax
  pushq %rax
  movq $0, %rax
  movq %rax, %rcx
  popq %rax
  cmpq %rcx, %rax
  movq $0, %rax
  sete %al
  movzbq %al, %rax
  cmpq $0, %rax
  je else_0
.data
str_1: .string "cero"
.text
    leaq str_1(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
    call printf
  jmp endif_0
else_0:
.data
str_2: .string "no cero"
.text
    leaq str_2(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
    call printf
endif_0:
    movq $40, %rdi
    call malloc
movq %rax, -16(%rbp)
  movq $10, %rax
  pushq %rax
    movq -16(%rbp), %rax
  pushq %rax
  movq $0, %rax
  movq %rax, %rcx
  popq %rax
  leaq (%rax,%rcx,8), %rax
  movq %rax, %rcx
  popq %rax
  movq %rax, (%rcx)
  movq $20, %rax
  pushq %rax
    movq -16(%rbp), %rax
  pushq %rax
  movq $1, %rax
  movq %rax, %rcx
  popq %rax
  leaq (%rax,%rcx,8), %rax
  movq %rax, %rcx
  popq %rax
  movq %rax, (%rcx)
  movq $30, %rax
  pushq %rax
    movq -16(%rbp), %rax
  pushq %rax
  movq $2, %rax
  movq %rax, %rcx
  popq %rax
  leaq (%rax,%rcx,8), %rax
  movq %rax, %rcx
  popq %rax
  movq %rax, (%rcx)
  movq $40, %rax
  pushq %rax
    movq -16(%rbp), %rax
  pushq %rax
  movq $3, %rax
  movq %rax, %rcx
  popq %rax
  leaq (%rax,%rcx,8), %rax
  movq %rax, %rcx
  popq %rax
  movq %rax, (%rcx)
  movq $50, %rax
  pushq %rax
    movq -16(%rbp), %rax
  pushq %rax
  movq $4, %rax
  movq %rax, %rcx
  popq %rax
  leaq (%rax,%rcx,8), %rax
  movq %rax, %rcx
  popq %rax
  movq %rax, (%rcx)
  movq $0, %rax
movq %rax, -24(%rbp)
while_3:
    movq -24(%rbp), %rax
  pushq %rax
  movq $5, %rax
  movq %rax, %rcx
  popq %rax
  cmpq %rcx, %rax
  movq $0, %rax
  setl %al
  movzbq %al, %rax
  cmpq $0, %rax
  je endwhile_3
    movq -16(%rbp), %rax
pushq %rax
    movq -24(%rbp), %rax
movq %rax, %rcx
popq %rax
leaq (%rax,%rcx,8), %rax
movq (%rax), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
    movq -24(%rbp), %rax
  pushq %rax
  movq $1, %rax
  movq %rax, %rcx
  popq %rax
  addq %rcx, %rax
movq %rax, -24(%rbp)
  jmp while_3
endwhile_3:
  movq $0, %rax
movq %rax, -32(%rbp)
  movq $1, %rax
movq %rax, -40(%rbp)
while_4:
    movq -40(%rbp), %rax
  pushq %rax
  movq $10, %rax
  movq %rax, %rcx
  popq %rax
  cmpq %rcx, %rax
  movq $0, %rax
  setle %al
  movzbq %al, %rax
  cmpq $0, %rax
  je endwhile_4
    movq -32(%rbp), %rax
  pushq %rax
    movq -40(%rbp), %rax
  movq %rax, %rcx
  popq %rax
  addq %rcx, %rax
movq %rax, -32(%rbp)
    movq -40(%rbp), %rax
  pushq %rax
  movq $1, %rax
  movq %rax, %rcx
  popq %rax
  addq %rcx, %rax
movq %rax, -40(%rbp)
  jmp while_4
endwhile_4:
    movq -32(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
  movq $0, %rax
movq %rax, -48(%rbp)
while_5:
    movq -48(%rbp), %rax
  pushq %rax
  movq $10, %rax
  movq %rax, %rcx
  popq %rax
  cmpq %rcx, %rax
  movq $0, %rax
  setl %al
  movzbq %al, %rax
  cmpq $0, %rax
  je endwhile_5
    movq -48(%rbp), %rax
  pushq %rax
  movq $5, %rax
  movq %rax, %rcx
  popq %rax
  cmpq %rcx, %rax
  movq $0, %rax
  sete %al
  movzbq %al, %rax
  cmpq $0, %rax
  je else_6
  jmp endwhile_5
  jmp endif_6
else_6:
endif_6:
    movq -48(%rbp), %rax
  pushq %rax
  movq $1, %rax
  movq %rax, %rcx
  popq %rax
  addq %rcx, %rax
movq %rax, -48(%rbp)
  jmp while_5
endwhile_5:
    movq -48(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
movq $0, %rax
leave
ret
end_control:
    movq $0, %rax
    leave
    ret
.globl main
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp
    call control
end_main:
    movq $0, %rax
    leave
    ret
