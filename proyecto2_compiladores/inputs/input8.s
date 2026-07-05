.data
print_int_fmt: .string "%ld\n"
print_str_fmt: .string "%s\n"

.text
.globl __comptime__
__comptime__:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp
  movq $10, %rax
  pushq %rax
  movq $10, %rax
  movq %rax, %rcx
  popq %rax
  imulq %rcx, %rax
movq %rax, -8(%rbp)
    movq -8(%rbp), %rax
movq %rax, %rdx
leaq print_int_fmt(%rip), %rcx
movl $0, %eax
    movq %rsp, -16(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call printf
    movq -16(%rbp), %rsp
end___comptime__:
    movq $0, %rax
    leave
    ret
.globl doble
doble:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    movq %rcx, -8(%rbp)
    movq -8(%rbp), %rax
  pushq %rax
  movq $2, %rax
  movq %rax, %rcx
  popq %rax
  imulq %rcx, %rax
movq %rax, -16(%rbp)
    movq -16(%rbp), %rax
leave
ret
end_doble:
    movq $0, %rax
    leave
    ret
.globl triple
triple:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    movq %rcx, -8(%rbp)
    movq -8(%rbp), %rax
  pushq %rax
  movq $3, %rax
  movq %rax, %rcx
  popq %rax
  imulq %rcx, %rax
movq %rax, -16(%rbp)
    movq -16(%rbp), %rax
leave
ret
end_triple:
    movq $0, %rax
    leave
    ret
.globl potencia
potencia:
    pushq %rbp
    movq %rsp, %rbp
    subq $48, %rsp
    movq %rcx, -8(%rbp)
    movq %rdx, -16(%rbp)
  movq $1, %rax
movq %rax, -24(%rbp)
  movq $0, %rax
movq %rax, -32(%rbp)
while_0:
    movq -32(%rbp), %rax
  pushq %rax
    movq -16(%rbp), %rax
  movq %rax, %rcx
  popq %rax
  cmpq %rcx, %rax
  movq $0, %rax
  setl %al
  movzbq %al, %rax
  cmpq $0, %rax
  je endwhile_0
    movq -24(%rbp), %rax
  pushq %rax
    movq -8(%rbp), %rax
  movq %rax, %rcx
  popq %rax
  imulq %rcx, %rax
movq %rax, -24(%rbp)
    movq -32(%rbp), %rax
  pushq %rax
  movq $1, %rax
  movq %rax, %rcx
  popq %rax
  addq %rcx, %rax
movq %rax, -32(%rbp)
  jmp while_0
endwhile_0:
    movq -24(%rbp), %rax
leave
ret
end_potencia:
    movq $0, %rax
    leave
    ret
.globl main
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $48, %rsp
  movq $5, %rax
    pushq %rax
    popq %rcx
    movq %rsp, -48(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call doble
    movq -48(%rbp), %rsp
movq %rax, -8(%rbp)
    movq -8(%rbp), %rax
movq %rax, %rdx
leaq print_int_fmt(%rip), %rcx
movl $0, %eax
    movq %rsp, -48(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call printf
    movq -48(%rbp), %rsp
  movq $7, %rax
    pushq %rax
    popq %rcx
    movq %rsp, -48(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call triple
    movq -48(%rbp), %rsp
movq %rax, -16(%rbp)
    movq -16(%rbp), %rax
movq %rax, %rdx
leaq print_int_fmt(%rip), %rcx
movl $0, %eax
    movq %rsp, -48(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call printf
    movq -48(%rbp), %rsp
  movq $8, %rax
    pushq %rax
  movq $2, %rax
    pushq %rax
    popq %rcx
    popq %rdx
    movq %rsp, -48(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call potencia
    movq -48(%rbp), %rsp
movq %rax, -24(%rbp)
    movq -24(%rbp), %rax
movq %rax, %rdx
leaq print_int_fmt(%rip), %rcx
movl $0, %eax
    movq %rsp, -48(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call printf
    movq -48(%rbp), %rsp
  movq $4, %rax
    pushq %rax
  movq $3, %rax
    pushq %rax
    popq %rcx
    popq %rdx
    movq %rsp, -48(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call potencia
    movq -48(%rbp), %rsp
movq %rax, -32(%rbp)
    movq -32(%rbp), %rax
movq %rax, %rdx
leaq print_int_fmt(%rip), %rcx
movl $0, %eax
    movq %rsp, -48(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call printf
    movq -48(%rbp), %rsp
end_main:
    movq $0, %rax
    leave
    ret
