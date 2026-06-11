#ifndef KHEAP_H
#define KHEAP_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define HEAP_START_ADDR 0x01000000
#define HEAP_MAX_ADDR   0x01400000 // Tamanho máximo de 4MB (1 Tabela de Páginas)
#define KHEAP_MAGIC     0x12345678

/* Estrutura do cabeçalho de metadados para cada bloco no Heap */
typedef struct header {
    struct header* next;
    size_t size;          // Tamanho utilizável pelo usuário (exclui este cabeçalho)
    bool is_free;
    uint32_t magic;       // Assinatura para validação e segurança contra corrupções
} header_t;

/* Inicializa o heap do kernel */
void kheap_init(void);

/* Aloca um bloco de memória dinâmica de tamanho 'size' */
void* kmalloc(size_t size);

/* Libera o bloco de memória apontado por 'ptr' */
void kfree(void* ptr);

#endif
