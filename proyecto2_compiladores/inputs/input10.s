.data
print_int_fmt: .string "%ld\n"
print_str_fmt: .string "%s\n"
print_char_fmt: .string "%c\n"

.text
.globl factorial
factorial:
pushq %rbp
movq %rsp, %rbp
subq $8, %rsp
movq %rdi, -8(%rbp)
movq -8(%rbp), %rax
pushq %rax
movq $1, %rax
movq %rax, %rcx
popq %rax
cmpq %rcx, %rax
movq $0, %rax
setle %al
movzbq %al, %rax
 cmpq $0, %rax
  je else_0
movq $1, %rax
pushq %rax
popq %rax
leave
ret
  jmp endif_0
else_0:
movq -8(%rbp), %rax
pushq %rax
movq -8(%rbp), %rax
pushq %rax
movq $1, %rax
movq %rax, %rcx
popq %rax
subq %rcx, %rax
pushq %rax
popq %rdi
call factorial
movq %rax, %rcx
popq %rax
imulq %rcx, %rax
pushq %rax
popq %rax
leave
ret
endif_0:
end_factorial:
movq $0, %rax
leave
ret
.globl fibonacci
fibonacci:
pushq %rbp
movq %rsp, %rbp
subq $8, %rsp
movq %rdi, -8(%rbp)
movq -8(%rbp), %rax
pushq %rax
movq $1, %rax
movq %rax, %rcx
popq %rax
cmpq %rcx, %rax
movq $0, %rax
setle %al
movzbq %al, %rax
 cmpq $0, %rax
  je else_1
movq -8(%rbp), %rax
pushq %rax
popq %rax
leave
ret
  jmp endif_1
else_1:
movq -8(%rbp), %rax
pushq %rax
movq $1, %rax
movq %rax, %rcx
popq %rax
subq %rcx, %rax
pushq %rax
popq %rdi
call fibonacci
pushq %rax
movq -8(%rbp), %rax
pushq %rax
movq $2, %rax
movq %rax, %rcx
popq %rax
subq %rcx, %rax
pushq %rax
popq %rdi
call fibonacci
movq %rax, %rcx
popq %rax
addq %rcx, %rax
pushq %rax
popq %rax
leave
ret
endif_1:
end_fibonacci:
movq $0, %rax
leave
ret
.globl area
area:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movq %rdi, -8(%rbp)
movq %rsi, -16(%rbp)
movq -8(%rbp), %rax
pushq %rax
movq -16(%rbp), %rax
movq %rax, %rcx
popq %rax
imulq %rcx, %rax
pushq %rax
popq %rax
leave
ret
end_area:
movq $0, %rax
leave
ret
.globl maximo
maximo:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movq %rdi, -8(%rbp)
movq %rsi, -16(%rbp)
movq -8(%rbp), %rax
pushq %rax
movq -16(%rbp), %rax
movq %rax, %rcx
popq %rax
cmpq %rcx, %rax
movq $0, %rax
setg %al
movzbq %al, %rax
 cmpq $0, %rax
  je else_2
movq -8(%rbp), %rax
pushq %rax
popq %rax
leave
ret
  jmp endif_2
else_2:
movq -16(%rbp), %rax
pushq %rax
popq %rax
leave
ret
endif_2:
end_maximo:
movq $0, %rax
leave
ret
.globl minimo
minimo:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movq %rdi, -8(%rbp)
movq %rsi, -16(%rbp)
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
  je else_3
movq -8(%rbp), %rax
pushq %rax
popq %rax
leave
ret
  jmp endif_3
else_3:
movq -16(%rbp), %rax
pushq %rax
popq %rax
leave
ret
endif_3:
end_minimo:
movq $0, %rax
leave
ret
.globl abs
abs:
pushq %rbp
movq %rsp, %rbp
subq $8, %rsp
movq %rdi, -8(%rbp)
movq -8(%rbp), %rax
pushq %rax
movq $0, %rax
movq %rax, %rcx
popq %rax
cmpq %rcx, %rax
movq $0, %rax
setl %al
movzbq %al, %rax
 cmpq $0, %rax
  je else_4
movq -8(%rbp), %rax
negq %rax
pushq %rax
popq %rax
leave
ret
  jmp endif_4
else_4:
movq -8(%rbp), %rax
pushq %rax
popq %rax
leave
ret
endif_4:
end_abs:
movq $0, %rax
leave
ret
.globl main
main:
pushq %rbp
movq %rsp, %rbp
subq $96, %rsp
movq $5, %rax
pushq %rax
popq %rdi
call factorial
movq %rax, -8(%rbp)
movq -8(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq $10, %rax
pushq %rax
popq %rdi
call factorial
movq %rax, -16(%rbp)
movq -16(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq $7, %rax
pushq %rax
popq %rdi
call fibonacci
movq %rax, -24(%rbp)
movq -24(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq $10, %rax
pushq %rax
popq %rdi
call fibonacci
movq %rax, -32(%rbp)
movq -32(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq $16, %rdi
call malloc
movq %rax, -40(%rbp)
movq $3, %rax
movq %rax, %rcx
movq -40(%rbp), %rax
movq $0, %rdi
movq %rcx, (%rax,%rdi,8)
movq $4, %rax
movq %rax, %rcx
movq -40(%rbp), %rax
movq $1, %rdi
movq %rcx, (%rax,%rdi,8)
movq -40(%rbp), %rax
movq %rax, %r10
movq $0, %rcx
movq (%r10,%rcx,8), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq -40(%rbp), %rax
movq %rax, %r10
movq $1, %rcx
movq (%r10,%rcx,8), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq $32, %rdi
call malloc
movq %rax, -48(%rbp)
movq $0, %rax
movq %rax, %rcx
movq -48(%rbp), %rax
movq $0, %rdi
movq %rcx, (%rax,%rdi,8)
movq $0, %rax
movq %rax, %rcx
movq -48(%rbp), %rax
movq $1, %rdi
movq %rcx, (%rax,%rdi,8)
movq $10, %rax
movq %rax, %rcx
movq -48(%rbp), %rax
movq $2, %rdi
movq %rcx, (%rax,%rdi,8)
movq $5, %rax
movq %rax, %rcx
movq -48(%rbp), %rax
movq $3, %rdi
movq %rcx, (%rax,%rdi,8)
movq -48(%rbp), %rax
movq %rax, %r10
movq $3, %rcx
movq (%r10,%rcx,8), %rax
pushq %rax
movq -48(%rbp), %rax
movq %rax, %r10
movq $2, %rcx
movq (%r10,%rcx,8), %rax
pushq %rax
popq %rdi
popq %rsi
call area
movq %rax, -56(%rbp)
movq -56(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq -40(%rbp), %rax
movq %rax, %r10
movq $1, %rcx
movq (%r10,%rcx,8), %rax
pushq %rax
movq -40(%rbp), %rax
movq %rax, %r10
movq $0, %rcx
movq (%r10,%rcx,8), %rax
pushq %rax
popq %rdi
popq %rsi
call maximo
movq %rax, -64(%rbp)
movq -64(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq -40(%rbp), %rax
movq %rax, %r10
movq $1, %rcx
movq (%r10,%rcx,8), %rax
pushq %rax
movq -40(%rbp), %rax
movq %rax, %r10
movq $0, %rcx
movq (%r10,%rcx,8), %rax
pushq %rax
popq %rdi
popq %rsi
call minimo
movq %rax, -72(%rbp)
movq -72(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq $42, %rax
negq %rax
pushq %rax
popq %rdi
call abs
movq %rax, -80(%rbp)
movq -80(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
    movq $40, %rdi
call malloc
movq %rax, -88(%rbp)
movq $5, %rax
pushq %rax
movq -88(%rbp), %rax
pushq %rax
movq $0, %rax
movq %rax, %rdi
popq %rax
popq %rcx
movq %rcx, (%rax,%rdi,8)
movq $3, %rax
pushq %rax
movq -88(%rbp), %rax
pushq %rax
movq $1, %rax
movq %rax, %rdi
popq %rax
popq %rcx
movq %rcx, (%rax,%rdi,8)
movq $8, %rax
pushq %rax
movq -88(%rbp), %rax
pushq %rax
movq $2, %rax
movq %rax, %rdi
popq %rax
popq %rcx
movq %rcx, (%rax,%rdi,8)
movq $1, %rax
pushq %rax
movq -88(%rbp), %rax
pushq %rax
movq $3, %rax
movq %rax, %rdi
popq %rax
popq %rcx
movq %rcx, (%rax,%rdi,8)
movq $9, %rax
pushq %rax
movq -88(%rbp), %rax
pushq %rax
movq $4, %rax
movq %rax, %rdi
popq %rax
popq %rcx
movq %rcx, (%rax,%rdi,8)
movq $0, %rax
movq %rax, -96(%rbp)
while_5:
movq -96(%rbp), %rax
pushq %rax
movq $5, %rax
movq %rax, %rcx
popq %rax
cmpq %rcx, %rax
movq $0, %rax
setl %al
movzbq %al, %rax
  cmpq $0, %rax
  je endwhile_5
movq -88(%rbp), %rax
pushq %rax
movq -96(%rbp), %rax
movq %rax, %rcx
popq %rax
leaq (%rax,%rcx,8), %rax
movq (%rax), %rax
  movq %rax, %r10
movq $1, %rax
  cmpq %rax, %r10
  je switch_6_case_0
movq $3, %rax
  cmpq %rax, %r10
  je switch_6_case_1
movq $5, %rax
  cmpq %rax, %r10
  je switch_6_case_2
movq $8, %rax
  cmpq %rax, %r10
  je switch_6_case_3
movq $9, %rax
  cmpq %rax, %r10
  je switch_6_case_4
  jmp switch_6_default
switch_6_case_0:
.data
str_7: .string "uno"
.text
leaq str_7(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
call printf
  jmp endswitch_6
switch_6_case_1:
.data
str_8: .string "tres"
.text
leaq str_8(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
call printf
  jmp endswitch_6
switch_6_case_2:
.data
str_9: .string "cinco"
.text
leaq str_9(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
call printf
  jmp endswitch_6
switch_6_case_3:
.data
str_10: .string "ocho"
.text
leaq str_10(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
call printf
  jmp endswitch_6
switch_6_case_4:
.data
str_11: .string "nueve"
.text
leaq str_11(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
call printf
  jmp endswitch_6
switch_6_default:
.data
str_12: .string "otro"
.text
leaq str_12(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
call printf
endswitch_6:
movq -96(%rbp), %rax
pushq %rax
movq $1, %rax
movq %rax, %rcx
popq %rax
addq %rcx, %rax
movq %rax, -96(%rbp)
  jmp while_5
endwhile_5:
movq $0, %rax
pushq %rax
movq -88(%rbp), %rax
movq %rax, %rdi
call free
movq -48(%rbp), %rax
movq %rax, %rdi
call free
movq -40(%rbp), %rax
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
