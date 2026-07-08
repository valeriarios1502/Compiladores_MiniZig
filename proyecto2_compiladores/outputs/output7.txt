.data
print_int_fmt: .string "%ld\n"
print_str_fmt: .string "%s\n"
print_char_fmt: .string "%c\n"

.text
.globl clasifica
clasifica:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movq %rdi, -8(%rbp)
movq -8(%rbp), %rax
  movq %rax, %r10
movq $1, %rax
  cmpq %rax, %r10
  je switch_0_case_0
movq $2, %rax
  cmpq %rax, %r10
  je switch_0_case_1
movq $3, %rax
  cmpq %rax, %r10
  je switch_0_case_2
  jmp switch_0_default
switch_0_case_0:
.data
str_1: .string "uno"
.text
leaq str_1(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
call printf
  jmp endswitch_0
switch_0_case_1:
.data
str_2: .string "dos"
.text
leaq str_2(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
call printf
  jmp endswitch_0
switch_0_case_2:
.data
str_3: .string "tres"
.text
leaq str_3(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
call printf
  jmp endswitch_0
switch_0_default:
.data
str_4: .string "otro"
.text
leaq str_4(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
call printf
endswitch_0:
end_clasifica:
movq $0, %rax
leave
ret
.globl diasemana
diasemana:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movq %rdi, -8(%rbp)
movq -8(%rbp), %rax
  movq %rax, %r10
movq $1, %rax
  cmpq %rax, %r10
  je switch_5_case_0
movq $2, %rax
  cmpq %rax, %r10
  je switch_5_case_1
movq $3, %rax
  cmpq %rax, %r10
  je switch_5_case_2
movq $4, %rax
  cmpq %rax, %r10
  je switch_5_case_3
movq $5, %rax
  cmpq %rax, %r10
  je switch_5_case_4
  jmp switch_5_default
switch_5_case_0:
.data
str_6: .string "lunes"
.text
leaq str_6(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
call printf
  jmp endswitch_5
switch_5_case_1:
.data
str_7: .string "martes"
.text
leaq str_7(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
call printf
  jmp endswitch_5
switch_5_case_2:
.data
str_8: .string "miercoles"
.text
leaq str_8(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
call printf
  jmp endswitch_5
switch_5_case_3:
.data
str_9: .string "jueves"
.text
leaq str_9(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
call printf
  jmp endswitch_5
switch_5_case_4:
.data
str_10: .string "viernes"
.text
leaq str_10(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
call printf
  jmp endswitch_5
switch_5_default:
.data
str_11: .string "fin de semana"
.text
leaq str_11(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
call printf
endswitch_5:
end_diasemana:
movq $0, %rax
leave
ret
.globl main
main:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movq $1, %rax
pushq %rax
popq %rdi
call clasifica
movq $2, %rax
pushq %rax
popq %rdi
call clasifica
movq $3, %rax
pushq %rax
popq %rdi
call clasifica
movq $5, %rax
pushq %rax
popq %rdi
call clasifica
movq $1, %rax
pushq %rax
popq %rdi
call diasemana
movq $3, %rax
pushq %rax
popq %rdi
call diasemana
movq $5, %rax
pushq %rax
popq %rdi
call diasemana
movq $6, %rax
pushq %rax
popq %rdi
call diasemana
movq $2, %rax
movq %rax, -8(%rbp)
movq -8(%rbp), %rax
pushq %rax
popq %rdi
call clasifica
end_main:
movq $0, %rax
leave
ret
