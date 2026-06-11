#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

/* Configura as tabelas e liga a MMU (Virtual Memory) */
void paging_init(void);

/* Mapeia uma pagina virtual no heap do kernel para um frame fisico */
void paging_map_heap_page(uint32_t virtual_addr, uint32_t physical_addr);

#endif
