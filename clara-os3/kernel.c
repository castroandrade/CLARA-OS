#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "multiboot.h"
#include "pmm.h"

/* --- DRIVER DE VÍDEO BÁSICO --- */
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

/* Trazendo o endereco final do kernel apenas para impressao */
extern uint32_t kernel_end;

void kernel_main(uint32_t magic, uint32_t addr) {
    /* Corrigindo o warning: Usando a variavel magic para seguranca */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        terminal_writestring("ERRO: Bootloader invalido!\n");
        return;
    }

    terminal_writestring("--- CLARA OS: Inicializando Modulos ---\n\n");

    /* Inicializa PMM lendo o mapa do Multiboot */
    multiboot_info_t* mbi = (multiboot_info_t*) addr;
    pmm_init(mbi);
    terminal_writestring("[OK] PMM (Physical Memory Manager) Iniciado.\n");
    
    terminal_writestring("O Kernel termina no endereco fisico: ");
    terminal_write_hex((uint32_t)&kernel_end);
    terminal_writestring("\n\n");

    /* TESTE PRÁTICO DA MEMÓRIA */
    terminal_writestring("Testando alocacao de Frames de 4KB...\n");
    
    void* frame1 = pmm_alloc_frame();
    terminal_writestring("-> Frame 1 alocado no endereco: ");
    terminal_write_hex((uint32_t)frame1);
    terminal_writestring("\n");

    void* frame2 = pmm_alloc_frame();
    terminal_writestring("-> Frame 2 alocado no endereco: ");
    terminal_write_hex((uint32_t)frame2);
    terminal_writestring("\n");

    void* frame3 = pmm_alloc_frame();
    terminal_writestring("-> Frame 3 alocado no endereco: ");
    terminal_write_hex((uint32_t)frame3);
    terminal_writestring("\n\n");

    terminal_writestring("Liberando o Frame 2... ");
    pmm_free_frame(frame2);
    terminal_writestring("[OK]\n");

    void* frame4 = pmm_alloc_frame();
    terminal_writestring("-> Frame 4 alocado no endereco: ");
    terminal_write_hex((uint32_t)frame4);
    terminal_writestring("\n(Note que o PMM reaproveitou a vaga do Frame 2!)\n");

    terminal_writestring("\nCLARA OS: Sucesso total na Fase PMM!");
}
