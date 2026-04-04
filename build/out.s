
.section __TEXT,__cstring,cstring_literals
.Lfmt_print_int:
    .asciz "%lld\n"
.Lfmt_read_int:
    .asciz "%lld"
.Lstr0:
    .asciz "BAD compiler demo"
.Lstr1:
    .asciz "a is greater"
.Lstr2:
    .asciz "b is greater"
.Lstr3:
    .asciz "a is less"
.Lstr4:
    .asciz "this is fucking it"

.text
.globl _main
_main:
    stp x29, x30, [sp, -16]!
    mov x29, sp
    sub sp, sp, #32
    adrp x0, .Lstr0@PAGE
    add x0, x0, .Lstr0@PAGEOFF
    bl _puts
    mov x0, 10
    sub x9, x29, #8
    str x0, [x9]
    mov x0, 3
    sub x9, x29, #16
    str x0, [x9]
    mov x0, 0
    sub x9, x29, #24
    str x0, [x9]
    sub x9, x29, #8
    ldr x0, [x9]
    str x0, [sp, -16]!
    sub x9, x29, #16
    ldr x0, [x9]
    ldr x9, [sp], 16
    add x0, x0, x9
    bl _bad_write_int
    sub x9, x29, #8
    ldr x0, [x9]
    str x0, [sp, -16]!
    sub x9, x29, #16
    ldr x0, [x9]
    ldr x9, [sp], 16
    mul x0, x0, x9
    bl _bad_write_int
    sub x9, x29, #8
    ldr x0, [x9]
    str x0, [sp, -16]!
    sub x9, x29, #16
    ldr x0, [x9]
    ldr x9, [sp], 16
    sub x0, x9, x0
    bl _bad_write_int
    sub x9, x29, #8
    ldr x0, [x9]
    str x0, [sp, -16]!
    sub x9, x29, #16
    ldr x0, [x9]
    ldr x9, [sp], 16
    sdiv x0, x9, x0
    bl _bad_write_int
    sub x9, x29, #8
    ldr x0, [x9]
    str x0, [sp, -16]!
    sub x9, x29, #16
    ldr x0, [x9]
    ldr x9, [sp], 16
    sdiv x10, x9, x0
    msub x0, x10, x0, x9
    bl _bad_write_int
    sub x9, x29, #8
    ldr x0, [x9]
    str x0, [sp, -16]!
    sub x9, x29, #16
    ldr x0, [x9]
    ldr x9, [sp], 16
    cmp x9, x0
    cset x0, gt
    cbz x0, .Lelse0
    adrp x0, .Lstr1@PAGE
    add x0, x0, .Lstr1@PAGEOFF
    bl _puts
    b .Lendif0
.Lelse0:
    adrp x0, .Lstr2@PAGE
    add x0, x0, .Lstr2@PAGEOFF
    bl _puts
.Lendif0:
.Lwhile1:
    sub x9, x29, #16
    ldr x0, [x9]
    str x0, [sp, -16]!
    mov x0, 0
    ldr x9, [sp], 16
    cmp x9, x0
    cset x0, gt
    cbz x0, .Lendwhile1
    sub x9, x29, #16
    ldr x0, [x9]
    bl _bad_write_int
    sub x9, x29, #16
    ldr x0, [x9]
    str x0, [sp, -16]!
    mov x0, 1
    ldr x9, [sp], 16
    sub x0, x9, x0
    sub x9, x29, #16
    str x0, [x9]
    b .Lwhile1
.Lendwhile1:
    sub x9, x29, #8
    ldr x0, [x9]
    str x0, [sp, -16]!
    sub x9, x29, #16
    ldr x0, [x9]
    ldr x9, [sp], 16
    cmp x9, x0
    cset x0, lt
    cbz x0, .Lelse2
    adrp x0, .Lstr3@PAGE
    add x0, x0, .Lstr3@PAGEOFF
    bl _puts
.Lelse2:
    adrp x0, .Lstr4@PAGE
    add x0, x0, .Lstr4@PAGEOFF
    bl _puts
    add sp, sp, #32
    mov w0, #0
    ldp x29, x30, [sp], 16
    ret
