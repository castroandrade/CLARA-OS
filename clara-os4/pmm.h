#ifndef PMM_H
#define PMM_H
#include <stdint.h>
#include <stdbool.h>
#include "multiboot.h"

#define PAGE_SIZE 4096

/* Inicializa o gerenciador lendo o Multiboot */
void pmm_init(multiboot_info_t* mbi);

/* Pede um frame de 4KB (Retorna o endereco de memoria) */
void* pmm_alloc_frame(void);

/* Devolve um frame para ficar livre novamente */
void pmm_free_frame(void* addr);

#endif
