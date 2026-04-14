MBALIGN  equ  1 << 0
MEMINFO  equ  1 << 1
FLAGS    equ  MBALIGN | MEMINFO
MAGIC    equ  0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .text
global _start:function (_start.end - _start)
extern kernel_main

_start:
    mov esp, stack_top

    ; NOVIDADE: Passando os argumentos para o C
    ; O x86 passa argumentos de trás pra frente (cdecl)
    push ebx ; Argumento 2: Endereço da estrutura Multiboot info
    push eax ; Argumento 1: Número Mágico (0x2BADB002)

    call kernel_main

    cli
.hang:
    hlt
    jmp .hang
.end:
