.data
print_int_fmt: .string "%ld\n"
print_str_fmt: .string "%s\n"
print_char_fmt: .string "%c\n"

.text
.data
.global LIMITE
LIMITE:
    .quad 100
.text

.global clamp
clamp:
pushq %rbp
movq %rsp, %rbp
subq $24, %rsp
movq %rdi, -8(%rbp)
movq %rsi, -16(%rbp)
movq %rdx, -24(%rbp)
movq -8(%rbp), %rax
pushq %rax
movq -16(%rbp), %rax
movq %rax, %rcx
popq %rax
cmpq %rcx, %rax
movq $0, %rax
setl %al
movzbq %al, %rax
 cmpq $0, %rax
  je else_0
movq -16(%rbp), %rax
pushq %rax
popq %rax
leave
ret
  jmp endif_0
else_0:
movq -8(%rbp), %rax
pushq %rax
movq -24(%rbp), %rax
movq %rax, %rcx
popq %rax
cmpq %rcx, %rax
movq $0, %rax
setg %al
movzbq %al, %rax
 cmpq $0, %rax
  je else_1
movq -24(%rbp), %rax
pushq %rax
popq %rax
leave
ret
  jmp endif_1
else_1:
movq -8(%rbp), %rax
pushq %rax
popq %rax
leave
ret
endif_1:
endif_0:
end_clamp:
movq $0, %rax
leave
ret
.globl buscarEn
buscarEn:
pushq %rbp
movq %rsp, %rbp
subq $40, %rsp
movq %rdi, -8(%rbp)
movq %rsi, -16(%rbp)
movq %rdx, -24(%rbp)
movq $0, %rax
movq %rax, -32(%rbp)
while_2:
movq -32(%rbp), %rax
pushq %rax
movq -24(%rbp), %rax
movq %rax, %rcx
popq %rax
cmpq %rcx, %rax
movq $0, %rax
setl %al
movzbq %al, %rax
  cmpq $0, %rax
  je endwhile_2
movq -16(%rbp), %rax
pushq %rax
movq -32(%rbp), %rax
movq %rax, %rcx
popq %rax
leaq (%rax,%rcx,8), %rax
movq (%rax), %rax
movq %rax, -40(%rbp)
movq -40(%rbp), %rax
pushq %rax
movq -8(%rbp), %rax
movq %rax, %rcx
popq %rax
cmpq %rcx, %rax
movq $0, %rax
sete %al
movzbq %al, %rax
 cmpq $0, %rax
  je else_3
movq -40(%rbp), %rax
pushq %rax
popq %rax
leave
ret
  jmp endif_3
else_3:
endif_3:
movq -32(%rbp), %rax
pushq %rax
movq $1, %rax
movq %rax, %rcx
popq %rax
addq %rcx, %rax
movq %rax, -32(%rbp)
  jmp while_2
endwhile_2:
movq $1, %rax
negq %rax
pushq %rax
popq %rax
leave
ret
end_buscarEn:
movq $0, %rax
leave
ret
.globl main
main:
pushq %rbp
movq %rsp, %rbp
subq $64, %rsp
    movq $40, %rdi
call malloc
movq %rax, -8(%rbp)
movq $10, %rax
pushq %rax
movq -8(%rbp), %rax
pushq %rax
movq $0, %rax
movq %rax, %rdi
popq %rax
popq %rcx
movq %rcx, (%rax,%rdi,8)
movq $25, %rax
pushq %rax
movq -8(%rbp), %rax
pushq %rax
movq $1, %rax
movq %rax, %rdi
popq %rax
popq %rcx
movq %rcx, (%rax,%rdi,8)
movq $50, %rax
pushq %rax
movq -8(%rbp), %rax
pushq %rax
movq $2, %rax
movq %rax, %rdi
popq %rax
popq %rcx
movq %rcx, (%rax,%rdi,8)
movq $75, %rax
pushq %rax
movq -8(%rbp), %rax
pushq %rax
movq $3, %rax
movq %rax, %rdi
popq %rax
popq %rcx
movq %rcx, (%rax,%rdi,8)
movq $100, %rax
pushq %rax
movq -8(%rbp), %rax
pushq %rax
movq $4, %rax
movq %rax, %rdi
popq %rax
popq %rcx
movq %rcx, (%rax,%rdi,8)
movq $5, %rax
pushq %rax
movq -8(%rbp), %rax
pushq %rax
movq $25, %rax
pushq %rax
popq %rdi
popq %rsi
popq %rdx
call buscarEn
movq %rax, -16(%rbp)
movq -16(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq $5, %rax
pushq %rax
movq -8(%rbp), %rax
pushq %rax
movq $99, %rax
pushq %rax
popq %rdi
popq %rsi
popq %rdx
call buscarEn
movq %rax, -24(%rbp)
movq -24(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq $50, %rax
pushq %rax
movq $5, %rax
pushq %rax
movq $3, %rax
pushq %rax
popq %rdi
popq %rsi
popq %rdx
call clamp
movq %rax, -32(%rbp)
movq -32(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq $50, %rax
pushq %rax
movq $5, %rax
pushq %rax
movq $30, %rax
pushq %rax
popq %rdi
popq %rsi
popq %rdx
call clamp
movq %rax, -40(%rbp)
movq -40(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq $50, %rax
pushq %rax
movq $5, %rax
pushq %rax
movq $80, %rax
pushq %rax
popq %rdi
popq %rsi
popq %rdx
call clamp
movq %rax, -48(%rbp)
movq -48(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq -16(%rbp), %rax
  movq %rax, %r10
movq $25, %rax
  cmpq %rax, %r10
  je switch_4_case_0
  jmp switch_4_default
switch_4_case_0:
.data
str_5: .string "veinticinco"
.text
leaq str_5(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
call printf
  jmp endswitch_4
switch_4_default:
.data
str_6: .string "otro valor"
.text
leaq str_6(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
call printf
endswitch_4:
movq $0, %rax
movq %rax, -56(%rbp)
while_7:
movq -56(%rbp), %rax
pushq %rax
movq $5, %rax
movq %rax, %rcx
popq %rax
cmpq %rcx, %rax
movq $0, %rax
setl %al
movzbq %al, %rax
  cmpq $0, %rax
  je endwhile_7
movq $60, %rax
pushq %rax
movq $20, %rax
pushq %rax
movq -8(%rbp), %rax
pushq %rax
movq -56(%rbp), %rax
movq %rax, %rcx
popq %rax
leaq (%rax,%rcx,8), %rax
movq (%rax), %rax
pushq %rax
popq %rdi
popq %rsi
popq %rdx
call clamp
movq %rax, -64(%rbp)
movq -64(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq -56(%rbp), %rax
pushq %rax
movq $1, %rax
movq %rax, %rcx
popq %rax
addq %rcx, %rax
movq %rax, -56(%rbp)
  jmp while_7
endwhile_7:
movq $0, %rax
pushq %rax
movq -8(%rbp), %rax
movq %rax, %rdi
call free
popq %rax
leave
ret
end_main:
movq $0, %rax
leave
ret

.section .note.GNU-stack,"",@progbits
