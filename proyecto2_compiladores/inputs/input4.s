.data
print_int_fmt: .string "%ld\n"
print_str_fmt: .string "%s\n"

.text

.global maximo
maximo:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    movq %rcx, -8(%rbp)
    movq %rdx, -16(%rbp)
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
  je else_0
    movq -8(%rbp), %rax
leave
ret
  jmp endif_0
else_0:
    movq -16(%rbp), %rax
leave
ret
endif_0:
end_maximo:
    movq $0, %rax
    leave
    ret

.global swap
swap:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    movq %rcx, -8(%rbp)
    movq %rdx, -16(%rbp)
    movq -8(%rbp), %rax
    movq (%rax), %rax
movq %rax, -24(%rbp)
    movq -16(%rbp), %rax
    movq (%rax), %rax
  pushq %rax
    movq -8(%rbp), %rax
  movq %rax, %rcx
  popq %rax
  movq %rax, (%rcx)
    movq -24(%rbp), %rax
  pushq %rax
    movq -16(%rbp), %rax
  movq %rax, %rcx
  popq %rax
  movq %rax, (%rcx)
end_swap:
    movq $0, %rax
    leave
    ret
.globl main
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $64, %rsp
  movq $7, %rax
    pushq %rax
  movq $3, %rax
    pushq %rax
    popq %rcx
    popq %rdx
    movq %rsp, -64(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call maximo
    movq -64(%rbp), %rsp
movq %rax, -8(%rbp)
    movq -8(%rbp), %rax
movq %rax, %rdx
leaq print_int_fmt(%rip), %rcx
movl $0, %eax
    movq %rsp, -64(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call printf
    movq -64(%rbp), %rsp
  movq $2, %rax
    pushq %rax
  movq $10, %rax
    pushq %rax
    popq %rcx
    popq %rdx
    movq %rsp, -64(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call maximo
    movq -64(%rbp), %rsp
movq %rax, -16(%rbp)
    movq -16(%rbp), %rax
movq %rax, %rdx
leaq print_int_fmt(%rip), %rcx
movl $0, %eax
    movq %rsp, -64(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call printf
    movq -64(%rbp), %rsp
  movq $99, %rax
    pushq %rax
  movq $100, %rax
    pushq %rax
    popq %rcx
    popq %rdx
    movq %rsp, -64(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call maximo
    movq -64(%rbp), %rsp
movq %rax, -24(%rbp)
    movq -24(%rbp), %rax
movq %rax, %rdx
leaq print_int_fmt(%rip), %rcx
movl $0, %eax
    movq %rsp, -64(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call printf
    movq -64(%rbp), %rsp
  movq $5, %rax
movq %rax, -32(%rbp)
  movq $9, %rax
movq %rax, -40(%rbp)
    leaq -40(%rbp), %rax
    pushq %rax
    leaq -32(%rbp), %rax
    pushq %rax
    popq %rcx
    popq %rdx
    movq %rsp, -64(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call swap
    movq -64(%rbp), %rsp
    movq -32(%rbp), %rax
movq %rax, %rdx
leaq print_int_fmt(%rip), %rcx
movl $0, %eax
    movq %rsp, -64(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call printf
    movq -64(%rbp), %rsp
    movq -40(%rbp), %rax
movq %rax, %rdx
leaq print_int_fmt(%rip), %rcx
movl $0, %eax
    movq %rsp, -64(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call printf
    movq -64(%rbp), %rsp
  movq $42, %rax
movq %rax, -48(%rbp)
  movq $0, %rax
movq %rax, -56(%rbp)
    leaq -56(%rbp), %rax
    pushq %rax
    leaq -48(%rbp), %rax
    pushq %rax
    popq %rcx
    popq %rdx
    movq %rsp, -64(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call swap
    movq -64(%rbp), %rsp
    movq -48(%rbp), %rax
movq %rax, %rdx
leaq print_int_fmt(%rip), %rcx
movl $0, %eax
    movq %rsp, -64(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call printf
    movq -64(%rbp), %rsp
    movq -56(%rbp), %rax
movq %rax, %rdx
leaq print_int_fmt(%rip), %rcx
movl $0, %eax
    movq %rsp, -64(%rbp)
    andq $-16, %rsp
    subq $32, %rsp
    call printf
    movq -64(%rbp), %rsp
end_main:
    movq $0, %rax
    leave
    ret
