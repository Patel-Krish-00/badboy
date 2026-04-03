.intel_syntax noprefix

.section .rodata
.Lstr0:
    .asciz "--- Arithmetic ---"
.Lstr1:
    .asciz "--- If/Else ---"
.Lstr2:
    .asciz "x is greater"
.Lstr3:
    .asciz "y is greater or equal"
.Lstr4:
    .asciz "--- Count 1 to 5 ---"
.Lstr5:
    .asciz "--- Fibonacci ---"
.Lstr6:
    .asciz "--- Even numbers 2..10 ---"

.section .text
.global _start

bad_print_int:
    push rbp
    mov rbp, rsp
    sub rsp, 32
    mov rax, rdi
    test rax, rax
    jnz .Lpi_nonzero
    mov byte ptr [rbp-2], 0x30
    mov byte ptr [rbp-1], 0x0a
    lea rsi, [rbp-2]
    mov rax, 1
    mov rdi, 1
    mov rdx, 2
    syscall
    leave
    ret
.Lpi_nonzero:
    xor r8d, r8d
    test rax, rax
    jns .Lpi_positive
    neg rax
    mov r8d, 1
.Lpi_positive:
    mov byte ptr [rbp-1], 0x0a
    lea rcx, [rbp-2]
    mov r9, 10
.Lpi_loop:
    xor rdx, rdx
    div r9
    add dl, 0x30
    mov byte ptr [rcx], dl
    dec rcx
    test rax, rax
    jnz .Lpi_loop
    test r8d, r8d
    jz .Lpi_no_minus
    mov byte ptr [rcx], 0x2d
    dec rcx
.Lpi_no_minus:
    inc rcx
    lea rdx, [rbp-1]
    sub rdx, rcx
    inc rdx
    mov rsi, rcx
    mov rdi, 1
    mov rax, 1
    syscall
    leave
    ret

bad_print_str:
    push rbp
    mov rbp, rsp
    sub rsp, 16
    mov rsi, rdi
    xor rdx, rdx
.Lps_len:
    cmp byte ptr [rdi + rdx], 0
    je .Lps_write
    inc rdx
    jmp .Lps_len
.Lps_write:
    mov rdi, 1
    mov rax, 1
    syscall
    mov byte ptr [rbp-1], 0x0a
    lea rsi, [rbp-1]
    mov rdx, 1
    mov rdi, 1
    mov rax, 1
    syscall
    leave
    ret

bad_read_int:
    push rbp
    mov rbp, rsp
    sub rsp, 48
    lea rsi, [rbp-40]
    mov rax, 0
    mov rdi, 0
    mov rdx, 32
    syscall
    lea rsi, [rbp-40]
    xor rax, rax
    xor r8d, r8d
.Lri_skip:
    movzx rcx, byte ptr [rsi]
    cmp rcx, 0x20
    je .Lri_skip_adv
    cmp rcx, 0x09
    je .Lri_skip_adv
    cmp rcx, 0x0a
    je .Lri_skip_adv
    cmp rcx, 0x0d
    je .Lri_skip_adv
    jmp .Lri_skip_done
.Lri_skip_adv:
    inc rsi
    jmp .Lri_skip
.Lri_skip_done:
    cmp byte ptr [rsi], 0x2d
    jne .Lri_parse
    mov r8d, 1
    inc rsi
.Lri_parse:
    movzx rcx, byte ptr [rsi]
    cmp rcx, 0x30
    jl .Lri_done
    cmp rcx, 0x39
    jg .Lri_done
    sub rcx, 0x30
    imul rax, rax, 10
    add rax, rcx
    inc rsi
    jmp .Lri_parse
.Lri_done:
    test r8d, r8d
    jz .Lri_ret
    neg rax
.Lri_ret:
    leave
    ret

_start:
    push rbp
    mov rbp, rsp
    sub rsp, 88
    mov rax, 10
    mov [rbp-8], rax
    mov rax, 7
    mov [rbp-16], rax
    mov rax, [rbp-8]
    push rax
    mov rax, [rbp-16]
    pop rbx
    add rax, rbx
    mov [rbp-24], rax
    mov rax, [rbp-8]
    push rax
    mov rax, [rbp-16]
    pop rbx
    sub rbx, rax
    mov rax, rbx
    mov [rbp-32], rax
    mov rax, [rbp-8]
    push rax
    mov rax, [rbp-16]
    pop rbx
    imul rax, rbx
    mov [rbp-40], rax
    lea rdi, [rip + .Lstr0]
    call bad_print_str
    mov rax, [rbp-24]
    mov rdi, rax
    call bad_print_int
    mov rax, [rbp-32]
    mov rdi, rax
    call bad_print_int
    mov rax, [rbp-40]
    mov rdi, rax
    call bad_print_int
    lea rdi, [rip + .Lstr1]
    call bad_print_str
    mov rax, [rbp-8]
    push rax
    mov rax, [rbp-16]
    pop rbx
    cmp rbx, rax
    setg al
    movzx rax, al
    test rax, rax
    jz .Lelse0
    lea rdi, [rip + .Lstr2]
    call bad_print_str
    jmp .Lendif0
.Lelse0:
    lea rdi, [rip + .Lstr3]
    call bad_print_str
.Lendif0:
    lea rdi, [rip + .Lstr4]
    call bad_print_str
    mov rax, 1
    mov [rbp-48], rax
.Lwhile1:
    mov rax, [rbp-48]
    push rax
    mov rax, 5
    pop rbx
    cmp rbx, rax
    setle al
    movzx rax, al
    test rax, rax
    jz .Lendwhile1
    mov rax, [rbp-48]
    mov rdi, rax
    call bad_print_int
    mov rax, [rbp-48]
    push rax
    mov rax, 1
    pop rbx
    add rax, rbx
    mov [rbp-48], rax
    jmp .Lwhile1
.Lendwhile1:
    lea rdi, [rip + .Lstr5]
    call bad_print_str
    mov rax, 0
    mov [rbp-56], rax
    mov rax, 1
    mov [rbp-64], rax
    mov rax, 0
    mov [rbp-72], rax
.Lwhile2:
    mov rax, [rbp-72]
    push rax
    mov rax, 8
    pop rbx
    cmp rbx, rax
    setl al
    movzx rax, al
    test rax, rax
    jz .Lendwhile2
    mov rax, [rbp-56]
    mov rdi, rax
    call bad_print_int
    mov rax, [rbp-56]
    push rax
    mov rax, [rbp-64]
    pop rbx
    add rax, rbx
    mov [rbp-80], rax
    mov rax, [rbp-64]
    mov [rbp-56], rax
    mov rax, [rbp-80]
    mov [rbp-64], rax
    mov rax, [rbp-72]
    push rax
    mov rax, 1
    pop rbx
    add rax, rbx
    mov [rbp-72], rax
    jmp .Lwhile2
.Lendwhile2:
    lea rdi, [rip + .Lstr6]
    call bad_print_str
    mov rax, 2
    mov [rbp-88], rax
.Lwhile3:
    mov rax, [rbp-88]
    push rax
    mov rax, 10
    pop rbx
    cmp rbx, rax
    setle al
    movzx rax, al
    test rax, rax
    jz .Lendwhile3
    mov rax, [rbp-88]
    push rax
    mov rax, 2
    pop rbx
    mov rcx, rax
    mov rax, rbx
    cqo
    idiv rcx
    mov rax, rdx
    push rax
    mov rax, 0
    pop rbx
    cmp rbx, rax
    sete al
    movzx rax, al
    test rax, rax
    jz .Lendif4
    mov rax, [rbp-88]
    mov rdi, rax
    call bad_print_int
.Lendif4:
    mov rax, [rbp-88]
    push rax
    mov rax, 1
    pop rbx
    add rax, rbx
    mov [rbp-88], rax
    jmp .Lwhile3
.Lendwhile3:
    mov rax, 60
    xor rdi, rdi
    syscall
