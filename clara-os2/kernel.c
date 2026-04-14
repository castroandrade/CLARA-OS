#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "multiboot.h"

/* --- DRIVER DE VÍDEO BÁSICO --- */
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
uint16_t* vga_buffer = (uint16_t*) 0xB8000;
size_t terminal_row = 0;
size_t terminal_column = 0;
uint8_t terminal_color = 15 | (0 << 4); // Branco no Preto

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        return;
    }
    size_t index = terminal_row * VGA_WIDTH + terminal_column;
    vga_buffer[index] = (uint16_t) c | (uint16_t) terminal_color << 8;
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;
    }
}

void terminal_writestring(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++) {
        terminal_putchar(data[i]);
    }
}

/* Converte um número 32-bits para Hexadecimal e imprime (Ex: 0x100000) */
void terminal_write_hex(uint32_t n) {
    terminal_writestring("0x");
    char buffer[9];
    buffer[8] = '\0';
    for (int i = 7; i >= 0; i--) {
        uint8_t nibble = (n >> (i * 4)) & 0x0F;
        buffer[7 - i] = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10);
    }
    terminal_writestring(buffer);
}

/* --- FUNÇÃO PRINCIPAL DO KERNEL --- */
/* Agora ela recebe os argumentos do boot.asm! */
void kernel_main(uint32_t magic, uint32_t addr) {
    terminal_writestring("--- CLARA OS: Mapa de Memoria RAM ---\n\n");

    /* 1. Verifica se o bootloader é confiável */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        terminal_writestring("ERRO: Bootloader invalido!\n");
        return;
    }

    /* 2. Converte o endereço para a nossa estrutura C */
    multiboot_info_t* mbi = (multiboot_info_t*) addr;

    /* 3. Verifica se a flag 6 (0x40) está ativa, indicando que o mapa existe */
    if (!(mbi->flags & (1 << 6))) {
        terminal_writestring("ERRO: Mapa de memoria nao fornecido pelo GRUB/QEMU.\n");
        return;
    }

    /* 4. Percorre o Mapa de Memória e imprime na tela */
    multiboot_memory_map_t* mmap = (multiboot_memory_map_t*) mbi->mmap_addr;
    uint32_t mmap_end = mbi->mmap_addr + mbi->mmap_length;

    terminal_writestring("ENDERECO INICIAL   | TAMANHO (BYTES)  | TIPO\n");
    terminal_writestring("--------------------------------------------------\n");

    while ((uint32_t) mmap < mmap_end) {
        terminal_write_hex(mmap->addr_low);
        terminal_writestring("         ");
        
        terminal_write_hex(mmap->len_low);
        terminal_writestring("         ");

        if (mmap->type == 1) {
            terminal_writestring("[RAM LIVRE]\n");
        } else {
            terminal_writestring("[RESERVADO]\n");
        }

        /* Avança para o próximo bloco do mapa */
        mmap = (multiboot_memory_map_t*) ((uint32_t) mmap + mmap->size + sizeof(mmap->size));
    }
    
    terminal_writestring("\nLeitura concluida! Memoria mapeada com sucesso.");
}
