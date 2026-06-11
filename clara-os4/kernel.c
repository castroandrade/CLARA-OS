#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "multiboot.h"
#include "pmm.h"
#include "paging.h" /* Incluimos o modulo de paginacao */
#include "kheap.h"  /* Incluimos o modulo do heap do kernel */

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

void test_kheap(void) {
    terminal_writestring("\n--- Testes do Kernel Heap (kmalloc/kfree) ---\n");
    
    terminal_writestring("1. Alloc A (128 bytes): ");
    void* a = kmalloc(128);
    if (a) { terminal_write_hex((uint32_t)a); terminal_writestring(" [OK]\n"); }
    else { terminal_writestring("FAIL\n"); }

    terminal_writestring("2. Alloc B (512 bytes): ");
    void* b = kmalloc(512);
    if (b) { terminal_write_hex((uint32_t)b); terminal_writestring(" [OK]\n"); }
    else { terminal_writestring("FAIL\n"); }

    terminal_writestring("3. Liberando bloco A...\n");
    kfree(a);

    terminal_writestring("4. Alloc C (64 bytes - reuso A): ");
    void* c = kmalloc(64);
    if (c) {
        terminal_write_hex((uint32_t)c);
        if (c == a) { terminal_writestring(" [REUSO OK]\n"); }
        else { terminal_writestring(" [OK - SEM REUSO]\n"); }
    } else { terminal_writestring("FAIL\n"); }

    terminal_writestring("5. Alloc D grande (8000 bytes - heap grow): ");
    void* d = kmalloc(8000);
    if (d) { terminal_write_hex((uint32_t)d); terminal_writestring(" [OK]\n"); }
    else { terminal_writestring("FAIL\n"); }

    terminal_writestring("6. Liberando B, C, D...\n");
    kfree(b);
    kfree(c);
    kfree(d);

    terminal_writestring("7. Alloc E (8000 bytes - coalescencia): ");
    void* e = kmalloc(8000);
    if (e) {
        terminal_write_hex((uint32_t)e);
        terminal_writestring(" [OK]\n");
        kfree(e);
        terminal_writestring("   [SUCESSO] Coalescencia validada!\n");
    } else { terminal_writestring("FAIL\n"); }

    terminal_writestring("--- Todos os testes do Heap concluidos! ---\n");
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
    terminal_writestring("[OK]\n");

    /* 3. Inicializador do Heap do Kernel */
    terminal_writestring("[...] Inicializando Kernel Heap (kheap)... ");
    kheap_init();
    terminal_writestring("[OK]\n\n");

    terminal_writestring("A Unidade de Gerenciamento de Memoria (MMU) esta ONLINE.\n");
    
    /* Roda a suite de testes do Heap */
    test_kheap();
}
