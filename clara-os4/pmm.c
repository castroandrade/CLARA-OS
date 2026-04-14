#include "pmm.h"
#include <stddef.h>

/* Um Bitmap para gerenciar até 4 GB de RAM.
 * 4 GB / 4 KB (Page Size) = 1.048.576 frames.
 * 1.048.576 frames / 32 bits = 32.768 posicoes no array. */
uint32_t bitmap[32768];

/* Macros para manipular os bits */
#define SET_BIT(i) bitmap[(i) / 32] |= (1 << ((i) % 32))
#define CLEAR_BIT(i) bitmap[(i) / 32] &= ~(1 << ((i) % 32))
#define TEST_BIT(i) (bitmap[(i) / 32] & (1 << ((i) % 32)))

/* Importa a variavel magica que criamos no linker.ld */
extern uint32_t kernel_end;

void pmm_init(multiboot_info_t* mbi) {
    /* 1. Marca TODOS os frames como OCUPADOS por padrao (Seguranca) */
    for (int i = 0; i < 32768; i++) {
        bitmap[i] = 0xFFFFFFFF;
    }

    /* 2. Percorre o Multiboot e marca o que for 'RAM Livre' como LIVRE (0) */
    if (!(mbi->flags & (1 << 6))) return;

    multiboot_memory_map_t* mmap = (multiboot_memory_map_t*) mbi->mmap_addr;
    uint32_t mmap_end = mbi->mmap_addr + mbi->mmap_length;

    while ((uint32_t) mmap < mmap_end) {
        if (mmap->type == 1) { /* 1 significa RAM disponivel */
            uint32_t start_frame = mmap->addr_low / PAGE_SIZE;
            uint32_t end_frame = (mmap->addr_low + mmap->len_low) / PAGE_SIZE;

            for (uint32_t i = start_frame; i < end_frame; i++) {
                CLEAR_BIT(i);
            }
        }
        mmap = (multiboot_memory_map_t*) ((uint32_t) mmap + mmap->size + sizeof(mmap->size));
    }

    /* 3. Proteger o comeco da memoria!
     * A area de 0 ate onde nosso Kernel termina PRECISA ser marcada como ocupada,
     * senao o PMM vai entregar o endereco do proprio Kernel para alguem subscrever! */
    uint32_t kernel_end_frame = ((uint32_t)&kernel_end) / PAGE_SIZE + 1;
    for (uint32_t i = 0; i < kernel_end_frame; i++) {
        SET_BIT(i);
    }
}

void* pmm_alloc_frame(void) {
    /* Procura o primeiro bit que seja 0 (Livre) */
    for (uint32_t i = 0; i < 32768; i++) {
        if (bitmap[i] != 0xFFFFFFFF) { /* Se tiver pelo menos 1 bit livre neste bloco */
            for (int j = 0; j < 32; j++) {
                if (!(bitmap[i] & (1 << j))) {
                    uint32_t frame = i * 32 + j;
                    SET_BIT(frame); /* Marca como ocupado */
                    return (void*)(frame * PAGE_SIZE); /* Retorna o endereco fisico */
                }
            }
        }
    }
    return NULL; /* Acabou a RAM! */
}

void pmm_free_frame(void* addr) {
    uint32_t frame = (uint32_t)addr / PAGE_SIZE;
    CLEAR_BIT(frame); /* Marca como livre novamente */
}
