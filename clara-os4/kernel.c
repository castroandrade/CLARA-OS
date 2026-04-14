#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "multiboot.h"
#include "pmm.h"
#include "paging.h" /* Incluimos o modulo de paginacao */

static const size_t VGA_WIDTH = 80;
uint16_t* vga_buffer = (uint16_t*) 0xB8000;
size_t terminal_row = 0;
size_t terminal_column = 0;
uint8_t terminal_color = 15 | (0 << 4);

void terminal_putchar(char c) {
    if (c == '\n') { terminal_column = 0; terminal_row++; return; }
    size_t index = terminal_row * VGA_WIDTH + terminal_column;
    vga_buffer[index] = (uint16_t) c | (uint16_t) terminal_color << 8;
    if (++terminal_column == VGA_WIDTH) { terminal_column = 0; terminal_row++; }
}

void terminal_writestring(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++) { terminal_putchar(data[i]); }
}

void terminal_write_hex(uint32_t n) {
    terminal_writestring("0x");
    char buffer[9]; buffer[8] = '\0';
    for (int i = 7; i >= 0; i--) {
        uint8_t nibble = (n >> (i * 4)) & 0x0F;
        buffer[7 - i] = nibble < 10 ? '0' + nibble : 'A' + (nibble - 10);
    }
    terminal_writestring(buffer);
}

void kernel_main(uint32_t magic, uint32_t addr) {
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        terminal_writestring("ERRO: Bootloader invalido!\n");
        return;
    }

    terminal_writestring("--- CLARA OS: Iniciando Subsistemas ---\n\n");

    /* 1. Gerenciador de Memoria Fisica (PMM) */
    multiboot_info_t* mbi = (multiboot_info_t*) addr;
    pmm_init(mbi);
    terminal_writestring("[OK] Gerenciador de Memoria Fisica (PMM)\n");

    /* 2. Gerenciador de Memoria Virtual (Paginacao) */
    terminal_writestring("[...] Ativando Paginacao (Virtual Memory)... ");
    paging_init();
    
    /* Se a maquina nao reiniciou na linha de cima, o milagre aconteceu! */
    terminal_writestring("[OK]\n\n");

    terminal_writestring("Parabens! A Unidade de Gerenciamento de Memoria (MMU) esta ONLINE.\n");
    terminal_writestring("Agora podemos isolar a memoria de processos!\n");
}
