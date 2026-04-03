.section __TEXT,__text
.global _main
.extern _printf

_main:
    adrp x0, fmt@PAGE
    add x0, x0, fmt@PAGEOFF
    adrp x1, msg@PAGE
    add x1, x1, msg@PAGEOFF
    bl _printf
    mov x0, #0
    ret

.section __TEXT,__cstring
fmt:
    .asciz "%s\n"
msg:
    .asciz ""
