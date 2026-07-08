.data
print_int_fmt: .string "%ld\n"
print_str_fmt: .string "%s\n"
print_char_fmt: .string "%c\n"

.text
.data
.global g_x
g_x:
    .quad 42
.text
.data
.global g_h
g_h:
    .quad 255
.text
.data
.global g_b
g_b:
    .quad 10
.text
.data
g_s_str: .string "hola mundo"
.text
.data
.global g_s
g_s:
    .quad g_s_str
.text
.data
.global g_ch
g_ch:
    .quad 65
.text
.data
.global g_flag
g_flag:
    .quad 1
.text
.data
.global g_nada
g_nada:
    .quad 0
.text
.data
.global g_indef
g_indef:
    .quad 0
.text
.data
.global MAX
MAX:
    .quad 100
.text
.globl main
main:
pushq %rbp
movq %rsp, %rbp
subq $16, %rsp
movq $12, %rax
movq %rax, -8(%rbp)
movq g_x(%rip), %rax
movq %rax, %rdx
leaq print_int_fmt(%rip), %rcx
movl $0, %eax
subq $32, %rsp
call printf
addq $32, %rsp
movq g_h(%rip), %rax
movq %rax, %rdx
leaq print_int_fmt(%rip), %rcx
movl $0, %eax
subq $32, %rsp
call printf
addq $32, %rsp
movq g_b(%rip), %rax
movq %rax, %rdx
leaq print_int_fmt(%rip), %rcx
movl $0, %eax
subq $32, %rsp
call printf
addq $32, %rsp
movq g_s(%rip), %rax
movq %rax, %rdx
leaq print_str_fmt(%rip), %rcx
movl $0, %eax
subq $32, %rsp
call printf
addq $32, %rsp
movq g_ch(%rip), %rax
movq %rax, %rdx
leaq print_char_fmt(%rip), %rcx
movl $0, %eax
subq $32, %rsp
call printf
addq $32, %rsp
movq g_flag(%rip), %rax
movq %rax, %rdx
leaq print_int_fmt(%rip), %rcx
movl $0, %eax
subq $32, %rsp
call printf
addq $32, %rsp
movq MAX(%rip), %rax
movq %rax, %rdx
leaq print_int_fmt(%rip), %rcx
movl $0, %eax
subq $32, %rsp
call printf
addq $32, %rsp
end_main:
movq $0, %rax
leave
ret
