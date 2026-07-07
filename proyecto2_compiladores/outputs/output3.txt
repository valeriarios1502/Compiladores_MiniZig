.data
print_int_fmt: .string "%ld\n"
print_str_fmt: .string "%s\n"
print_char_fmt: .string "%c\n"

.text
.globl saludar
saludar:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp
.data
str_0: .string "hola"
.text
    leaq str_0(%rip), %rax
movq %rax, %rsi
leaq print_str_fmt(%rip), %rdi
movl $0, %eax
    call printf
end_saludar:
    movq $0, %rax
    leave
    ret
.globl sumar
sumar:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    movq %rdi, -8(%rbp)
    movq %rsi, -16(%rbp)
    movq -8(%rbp), %rax
  pushq %rax
    movq -16(%rbp), %rax
  movq %rax, %rcx
  popq %rax
  addq %rcx, %rax
leave
ret
end_sumar:
    movq $0, %rax
    leave
    ret
.globl buscar
buscar:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    movq %rdi, -8(%rbp)
    movq %rsi, -16(%rbp)
  movq $0, %rax
movq %rax, -24(%rbp)
while_1:
    movq -24(%rbp), %rax
  pushq %rax
    movq -16(%rbp), %rax
  movq %rax, %rcx
  popq %rax
  cmpq %rcx, %rax
  movq $0, %rax
  setl %al
  movzbq %al, %rax
  cmpq $0, %rax
  je endwhile_1
    movq -24(%rbp), %rax
  pushq %rax
  movq $1, %rax
  movq %rax, %rcx
  popq %rax
  addq %rcx, %rax
movq %rax, -24(%rbp)
  jmp while_1
endwhile_1:
    movq -24(%rbp), %rax
leave
ret
end_buscar:
    movq $0, %rax
    leave
    ret
.globl main
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $48, %rsp
    call saludar
  movq $4, %rax
    pushq %rax
  movq $3, %rax
    pushq %rax
    popq %rdi
    popq %rsi
    call sumar
movq %rax, -8(%rbp)
    movq -8(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
  movq $20, %rax
    pushq %rax
  movq $10, %rax
    pushq %rax
    popq %rdi
    popq %rsi
    call sumar
movq %rax, -16(%rbp)
    movq -16(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
  movq $200, %rax
    pushq %rax
  movq $100, %rax
    pushq %rax
    popq %rdi
    popq %rsi
    call sumar
movq %rax, -24(%rbp)
    movq -24(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
    movq $40, %rdi
    call malloc
movq %rax, -32(%rbp)
  movq $10, %rax
  pushq %rax
    movq -32(%rbp), %rax
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
    movq -32(%rbp), %rax
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
    movq -32(%rbp), %rax
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
    movq -32(%rbp), %rax
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
    movq -32(%rbp), %rax
  pushq %rax
  movq $4, %rax
  movq %rax, %rcx
  popq %rax
  leaq (%rax,%rcx,8), %rax
  movq %rax, %rcx
  popq %rax
  movq %rax, (%rcx)
  movq $0, %rax
movq %rax, -40(%rbp)
while_2:
    movq -40(%rbp), %rax
  pushq %rax
  movq $5, %rax
  movq %rax, %rcx
  popq %rax
  cmpq %rcx, %rax
  movq $0, %rax
  setl %al
  movzbq %al, %rax
  cmpq $0, %rax
  je endwhile_2
    movq -32(%rbp), %rax
pushq %rax
    movq -40(%rbp), %rax
movq %rax, %rcx
popq %rax
leaq (%rax,%rcx,8), %rax
movq (%rax), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
    movq -40(%rbp), %rax
  pushq %rax
  movq $1, %rax
  movq %rax, %rcx
  popq %rax
  addq %rcx, %rax
movq %rax, -40(%rbp)
  jmp while_2
endwhile_2:
end_main:
    movq $0, %rax
    leave
    ret

.section .note.GNU-stack,"",@progbits
