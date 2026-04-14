#ifndef MULTIBOOT_H
#define MULTIBOOT_H
#include <stdint.h>

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

/* A estrutura principal que o Bootloader nos entrega */
typedef struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length; /* Tamanho do mapa de memória */
    uint32_t mmap_addr;   /* Endereço do mapa de memória */
} multiboot_info_t;

/* A estrutura de cada bloco de memória RAM */
typedef struct multiboot_memory_map {
    uint32_t size;
    uint32_t addr_low;
    uint32_t addr_high;
    uint32_t len_low;
    uint32_t len_high;
    uint32_t type; /* type 1 = RAM Livre, type 2 = RAM Reservada/Ocupada */
} __attribute__((packed)) multiboot_memory_map_t;

#endif
