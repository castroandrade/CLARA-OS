; boot.asm - Ponto de entrada do CLARA OS
; Define constantes para o cabeçalho Multiboot
MBALIGN  equ  1 << 0            ; Alinhar módulos carregados
MEMINFO  equ  1 << 1            ; Fornecer mapa de memória
FLAGS    equ  MBALIGN | MEMINFO ; Flags do Multiboot
MAGIC    equ  0x1BADB002        ; Número mágico para o bootloader
CHECKSUM equ -(MAGIC + FLAGS)   ; Checksum obrigatório

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
    resb 16384 ; 16 KB de pilha
stack_top:

section .text
global _start:function (_start.end - _start)
extern kernel_main

_start:
    ; Configura a pilha (stack) para o C funcionar
    mov esp, stack_top

    ; Chama o kernel em C
    call kernel_main

    ; Trava o PC se o kernel retornar
    cli
.hang:
    hlt
    jmp .hang
.end:
