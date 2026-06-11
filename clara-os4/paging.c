#include "paging.h"
#include <stdint.h>

/* O Diretório Mestre (Cobre 4 GB inteiros) */
uint32_t page_directory[1024] __attribute__((aligned(4096)));

/* A Primeira Tabela (Cobre os primeiros 4 MB da RAM) */
uint32_t first_page_table[1024] __attribute__((aligned(4096)));

/* A Tabela de Páginas do Heap do Kernel (Mapeia 4 MB a partir de 16 MB) */
uint32_t heap_page_table[1024] __attribute__((aligned(4096)));

extern void terminal_writestring(const char* data);

void paging_init(void) {
    /* 1. Inicializa o Page Directory inteiro como "Não Presente" */
    for (int i = 0; i < 1024; i++) {
        /* Flag 0x02 = Permite Leitura/Escrita, Flag 0x01 (Present) está DESLIGADA */
        page_directory[i] = 0x00000002;
    }

    /* 2. Identity Mapping: Mapear os primeiros 4 MB (Físico = Virtual)
     * Nosso kernel está em 1MB (0x100000) e o vídeo em 0xB8000. 
     * Ambos cabem com folga nesses primeiros 4MB! */
    for (int i = 0; i < 1024; i++) {
        /* i * 4096 = Endereço Físico do Frame.
         * | 3 = Flags (0x01 Present + 0x02 Read/Write) */
        first_page_table[i] = (i * 4096) | 3;
    }

    /* 3. Identity mapping da heap_page_table como "Não Presente" inicialmente */
    for (int i = 0; i < 1024; i++) {
        heap_page_table[i] = 0x00000002;
    }

    /* 4. Coloca a primeira Tabela no slot 0 (0 a 4 MB) */
    page_directory[0] = ((uint32_t)first_page_table) | 3;

    /* 5. Coloca a Tabela do Heap no slot 4 (16 a 20 MB) */
    page_directory[4] = ((uint32_t)heap_page_table) | 3;

    /* 6. Carrega o endereço do Diretório no registrador CR3 da CPU. */
    asm volatile("mov %0, %%cr3":: "r"(page_directory));

    /* 7. LIGA A PAGINAÇÃO! (Ativa o bit 31 do registrador CR0) */
    uint32_t cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; /* 0x80000000 = Bit 31 em nivel 1 */
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}

void paging_map_heap_page(uint32_t virtual_addr, uint32_t physical_addr) {
    if (virtual_addr < 0x01000000 || virtual_addr >= 0x01400000) {
        terminal_writestring("PANIC: Mapeamento de heap fora dos limites (16MB - 20MB)!\n");
        while (1) {
            asm volatile("hlt");
        }
    }
    uint32_t offset = virtual_addr - 0x01000000;
    uint32_t pt_idx = offset / 4096;
    
    heap_page_table[pt_idx] = physical_addr | 3; // Present + Read/Write
    
    // Invalida a entrada correspondente na cache TLB da CPU
    asm volatile("invlpg (%0)" :: "r"(virtual_addr) : "memory");
}
