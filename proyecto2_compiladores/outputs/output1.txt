.data
print_int_fmt: .string "%ld\n"
print_str_fmt: .string "%s\n"

.text
.data
.global g_x
g_x:
    .quad 42
.text
.data
.global g_y
g_y:
    .quad 0
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
.global g_s
g_s:
    .quad 0
.text
.data
.global g_ch
g_ch:
    .quad 0
.text
.data
.global g_flag
g_flag:
    .quad 0
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
.global PI
PI:
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
    movq g_x(%rip), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
    movq g_y(%rip), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
    movq g_h(%rip), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
    movq g_b(%rip), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
    movq g_s(%rip), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
    movq g_ch(%rip), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
    movq g_flag(%rip), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
    movq PI(%rip), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
    movq MAX(%rip), %rax
movq %rax, %rsi
leaq print_int_fmt(%rip), %rdi
movl $0, %eax
    call printf
end_main:
    movq $0, %rax
    leave
    ret
