.data
print_int_fmt: .string "%ld\n"
print_str_fmt: .string "%s\n"
print_char_fmt: .string "%c\n"

.text
.globl main
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    movq $24, %rdi
    call malloc
movq %rax, -8(%rbp)
  movq $10, %rax
  pushq %rax
    movq -8(%rbp), %rax
  addq $0, %rax
  movq %rax, %rcx
  popq %rax
  movq %rax, (%rcx)
  movq $20, %rax
  pushq %rax
    movq -8(%rbp), %rax
  addq $8, %rax
  movq %rax, %rcx
  popq %rax
  movq %rax, (%rcx)
  movq $30, %rax
  pushq %rax
    movq -8(%rbp), %rax
  addq $16, %rax
  movq %rax, %rcx
  popq %rax
  movq %rax, (%rcx)
    movq -8(%rbp), %rax
    movq %rax, %r10
    movq 0(%r10), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
    movq -8(%rbp), %rax
    movq %rax, %r10
    movq 8(%r10), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
    movq -8(%rbp), %rax
    movq %rax, %r10
    movq 16(%r10), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
    movq $16, %rdi
    call malloc
movq %rax, -16(%rbp)
  movq $99, %rax
  pushq %rax
    movq -16(%rbp), %rax
  addq $0, %rax
  movq %rax, %rcx
  popq %rax
  movq %rax, (%rcx)
    movq -16(%rbp), %rax
    movq %rax, %r10
    movq 0(%r10), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
    movq $24, %rdi
    call malloc
movq %rax, -24(%rbp)
  movq $255, %rax
  pushq %rax
    movq -24(%rbp), %rax
  addq $0, %rax
  movq %rax, %rcx
  popq %rax
  movq %rax, (%rcx)
  movq $128, %rax
  pushq %rax
    movq -24(%rbp), %rax
  addq $8, %rax
  movq %rax, %rcx
  popq %rax
  movq %rax, (%rcx)
  movq $0, %rax
  pushq %rax
    movq -24(%rbp), %rax
  addq $16, %rax
  movq %rax, %rcx
  popq %rax
  movq %rax, (%rcx)
    movq -24(%rbp), %rax
    movq %rax, %r10
    movq 0(%r10), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
    movq -24(%rbp), %rax
    movq %rax, %r10
    movq 8(%r10), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
    movq -24(%rbp), %rax
    movq %rax, %r10
    movq 16(%r10), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
end_main:
    movq $0, %rax
    leave
    ret

.section .note.GNU-stack,"",@progbits
