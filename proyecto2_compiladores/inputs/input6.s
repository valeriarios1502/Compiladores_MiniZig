.data
print_int_fmt: .string "%ld\n"
print_str_fmt: .string "%s\n"
print_char_fmt: .string "%c\n"

.text
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
pushq %rax
popq %rax
leave
ret
end_sumar:
movq $0, %rax
leave
ret
.globl exprs
exprs:
pushq %rbp
movq %rsp, %rbp
subq $96, %rsp
movq $5, %rax
movq %rax, -8(%rbp)
movq -8(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq $1, %rax
movq %rax, -16(%rbp)
movq -16(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq -8(%rbp), %rax
pushq %rax
movq $6, %rax
movq %rax, %rcx
popq %rax
cmpq %rcx, %rax
movq $0, %rax
sete %al
movzbq %al, %rax
movq %rax, -24(%rbp)
movq -24(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq -8(%rbp), %rax
pushq %rax
movq $99, %rax
movq %rax, %rcx
popq %rax
cmpq %rcx, %rax
movq $0, %rax
setne %al
movzbq %al, %rax
movq %rax, -32(%rbp)
movq -32(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq -8(%rbp), %rax
pushq %rax
movq $5, %rax
movq %rax, %rcx
popq %rax
cmpq %rcx, %rax
movq $0, %rax
setg %al
movzbq %al, %rax
movq %rax, -40(%rbp)
movq -40(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq -8(%rbp), %rax
pushq %rax
movq $100, %rax
movq %rax, %rcx
popq %rax
cmpq %rcx, %rax
movq $0, %rax
setl %al
movzbq %al, %rax
movq %rax, -48(%rbp)
movq -48(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq -8(%rbp), %rax
negq %rax
movq %rax, -56(%rbp)
movq -56(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
leaq -8(%rbp), %rax
movq %rax, -64(%rbp)
movq -64(%rbp), %rax
movq (%rax), %rax
movq %rax, -72(%rbp)
movq -72(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq $999, %rax
pushq %rax
movq -64(%rbp), %rax
movq %rax, %rcx
popq %rax
movq %rax, (%rcx)
movq -8(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
    movq $24, %rdi
call malloc
movq %rax, -80(%rbp)
movq $100, %rax
pushq %rax
movq -80(%rbp), %rax
pushq %rax
movq $0, %rax
movq %rax, %rcx
popq %rax
leaq (%rax,%rcx,8), %rax
movq %rax, %rcx
popq %rax
movq %rax, (%rcx)
movq $200, %rax
pushq %rax
movq -80(%rbp), %rax
pushq %rax
movq $1, %rax
movq %rax, %rcx
popq %rax
leaq (%rax,%rcx,8), %rax
movq %rax, %rcx
popq %rax
movq %rax, (%rcx)
movq $300, %rax
pushq %rax
movq -80(%rbp), %rax
pushq %rax
movq $2, %rax
movq %rax, %rcx
popq %rax
leaq (%rax,%rcx,8), %rax
movq %rax, %rcx
popq %rax
movq %rax, (%rcx)
movq -80(%rbp), %rax
pushq %rax
movq $0, %rax
movq %rax, %rcx
popq %rax
leaq (%rax,%rcx,8), %rax
movq (%rax), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq -80(%rbp), %rax
pushq %rax
movq $1, %rax
movq %rax, %rcx
popq %rax
leaq (%rax,%rcx,8), %rax
movq (%rax), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq -80(%rbp), %rax
pushq %rax
movq $2, %rax
movq %rax, %rcx
popq %rax
leaq (%rax,%rcx,8), %rax
movq (%rax), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
movq $1, %rax
pushq %rax
movq -8(%rbp), %rax
pushq %rax
popq %rdi
popq %rsi
call sumar
movq %rax, -88(%rbp)
movq -88(%rbp), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
call printf
end_exprs:
movq $0, %rax
leave
ret
.globl main
main:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
call exprs
end_main:
movq $0, %rax
leave
ret

.section .note.GNU-stack,"",@progbits
