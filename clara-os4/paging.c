#include "paging.h"
#include <stdint.h>

/* O Diretório Mestre (Cobre 4 GB inteiros) */
uint32_t page_directory[1024] __attribute__((aligned(4096)));

/* A Primeira Tabela (Cobre os primeiros 4 MB da RAM) */
uint32_t first_page_table[1024] __attribute__((aligned(4096)));

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

    /* 3. Coloca a nossa Tabela no primeiro slot do Diretório Mestre */
    page_directory[0] = ((uint32_t)first_page_table) | 3;

    /* 4. Carrega o endereço do Diretório no registrador CR3 da CPU.
     * O CR3 é o registrador que a MMU lê para achar o mapa de memoria. */
    asm volatile("mov %0, %%cr3":: "r"(page_directory));

    /* 5. LIGA A PAGINAÇÃO! (Ativa o bit 31 do registrador CR0) */
    uint32_t cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; /* 0x80000000 = Bit 31 em nivel 1 */
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}
