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
movq %rax, %rcx
movq -8(%rbp), %rax
movq $0, %rdi
movq %rcx, (%rax,%rdi,8)
movq $20, %rax
movq %rax, %rcx
movq -8(%rbp), %rax
movq $1, %rdi
movq %rcx, (%rax,%rdi,8)
movq $30, %rax
movq %rax, %rcx
movq -8(%rbp), %rax
movq $2, %rdi
movq %rcx, (%rax,%rdi,8)
movq -8(%rbp), %rax
movq %rax, %r10
movq $0, %rcx
movq (%r10,%rcx,8), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq -8(%rbp), %rax
movq %rax, %r10
movq $1, %rcx
movq (%r10,%rcx,8), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq -8(%rbp), %rax
movq %rax, %r10
movq $2, %rcx
movq (%r10,%rcx,8), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq $16, %rdi
call malloc
movq %rax, -16(%rbp)
movq $99, %rax
movq %rax, %rcx
movq -16(%rbp), %rax
movq $0, %rdi
movq %rcx, (%rax,%rdi,8)
movq -16(%rbp), %rax
movq %rax, %r10
movq $0, %rcx
movq (%r10,%rcx,8), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq $24, %rdi
call malloc
movq %rax, -24(%rbp)
movq $255, %rax
movq %rax, %rcx
movq -24(%rbp), %rax
movq $0, %rdi
movq %rcx, (%rax,%rdi,8)
movq $128, %rax
movq %rax, %rcx
movq -24(%rbp), %rax
movq $1, %rdi
movq %rcx, (%rax,%rdi,8)
movq $0, %rax
movq %rax, %rcx
movq -24(%rbp), %rax
movq $2, %rdi
movq %rcx, (%rax,%rdi,8)
movq -24(%rbp), %rax
movq %rax, %r10
movq $0, %rcx
movq (%r10,%rcx,8), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq -24(%rbp), %rax
movq %rax, %r10
movq $1, %rcx
movq (%r10,%rcx,8), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq -24(%rbp), %rax
movq %rax, %r10
movq $2, %rcx
movq (%r10,%rcx,8), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
end_main:
movq $0, %rax
leave
ret
