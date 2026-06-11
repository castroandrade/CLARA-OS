#include "kheap.h"
#include "pmm.h"
#include "paging.h"
#include <stddef.h>

/* Ponteiro para o início da lista de blocos do Heap */
static header_t* heap_start = (header_t*)HEAP_START_ADDR;

/* Fim da memória virtual mapeada atualmente para o Heap */
static uint32_t heap_end_addr = HEAP_START_ADDR;

/* Importa função de log para mensagens de erro/pânico */
extern void terminal_writestring(const char* data);

/* Junta blocos livres contíguos para evitar fragmentação */
static void kheap_coalesce(void) {
    header_t* curr = heap_start;
    while (curr != NULL && curr->next != NULL) {
        if (curr->is_free && curr->next->is_free) {
            // Verifica se os blocos estão fisicamente adjacentes na memória
            uint32_t expected_next = (uint32_t)curr + sizeof(header_t) + curr->size;
            if (expected_next == (uint32_t)curr->next) {
                // Incorpora o próximo bloco e seu cabeçalho
                curr->size += sizeof(header_t) + curr->next->size;
                curr->next = curr->next->next;
                // Não avança para tentar coalescer novamente com o novo vizinho
                continue;
            }
        }
        curr = curr->next;
    }
}

/* Solicita novos frames físicos ao PMM e expande o heap virtual */
static bool kheap_grow(size_t needed_size) {
    size_t total_required = needed_size + sizeof(header_t);
    // Calcula a quantidade de páginas de 4KB necessárias
    size_t num_pages = (total_required + PAGE_SIZE - 1) / PAGE_SIZE;

    // Garante que a expansão não exceda o limite virtual de 4MB do Heap (16MB a 20MB)
    if (heap_end_addr + (num_pages * PAGE_SIZE) > HEAP_MAX_ADDR) {
        return false;
    }

    uint32_t growth_start = heap_end_addr;

    // Aloca e mapeia cada nova página
    for (size_t i = 0; i < num_pages; i++) {
        void* phys_frame = pmm_alloc_frame();
        if (phys_frame == NULL) {
            terminal_writestring("ERRO: Sem memoria fisica livre (PMM) ao crescer o Heap!\n");
            return false;
        }
        paging_map_heap_page(heap_end_addr, (uint32_t)phys_frame);
        heap_end_addr += PAGE_SIZE;
    }

    // Cria o novo bloco no espaço adicionado
    header_t* new_block = (header_t*)growth_start;
    new_block->next = NULL;
    new_block->size = (num_pages * PAGE_SIZE) - sizeof(header_t);
    new_block->is_free = true;
    new_block->magic = KHEAP_MAGIC;

    // Localiza o último bloco da lista encadeada e anexa o novo bloco
    header_t* curr = heap_start;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = new_block;

    // Coalesce para fundir com o bloco anterior se este também for livre
    kheap_coalesce();

    return true;
}

void kheap_init(void) {
    // Aloca a primeira página física para inicializar o heap
    void* phys_frame = pmm_alloc_frame();
    if (phys_frame == NULL) {
        terminal_writestring("PANIC: Memoria insuficiente para inicializar o Kernel Heap!\n");
        while (1) {
            asm volatile("hlt");
        }
    }

    // Mapeia a primeira página no endereço virtual de início do Heap
    paging_map_heap_page(HEAP_START_ADDR, (uint32_t)phys_frame);
    heap_end_addr = HEAP_START_ADDR + PAGE_SIZE;

    // Configura o cabeçalho do bloco livre inicial
    heap_start->next = NULL;
    heap_start->size = PAGE_SIZE - sizeof(header_t);
    heap_start->is_free = true;
    heap_start->magic = KHEAP_MAGIC;
}

void* kmalloc(size_t size) {
    if (size == 0) return NULL;

    // Alinha o tamanho solicitado em 4 bytes (necessário para alinhamento x86)
    size = (size + 3) & ~3;

    header_t* curr = heap_start;

    while (curr != NULL) {
        // Validação simples contra corrupção do cabeçalho
        if (curr->magic != KHEAP_MAGIC) {
            terminal_writestring("PANIC: Corrupcao detectada no cabecalho do Heap!\n");
            while (1) {
                asm volatile("hlt");
            }
        }

        if (curr->is_free && curr->size >= size) {
            // Bloco encontrado! Verifica se vale a pena dividir o bloco
            // Divide se restar espaço suficiente para o cabeçalho + bloco mínimo (4 bytes)
            if (curr->size >= size + sizeof(header_t) + 4) {
                header_t* new_block = (header_t*)((uint8_t*)curr + sizeof(header_t) + size);
                
                new_block->next = curr->next;
                new_block->size = curr->size - size - sizeof(header_t);
                new_block->is_free = true;
                new_block->magic = KHEAP_MAGIC;

                curr->next = new_block;
                curr->size = size;
            }
            curr->is_free = false;
            // Retorna o endereço logo após o cabeçalho
            return (void*)((uint8_t*)curr + sizeof(header_t));
        }
        curr = curr->next;
    }

    // Se nenhum bloco servir, tenta crescer o heap
    if (kheap_grow(size)) {
        // Tenta alocar novamente após a expansão
        return kmalloc(size);
    }

    terminal_writestring("ERRO: kmalloc falhou (Sem memoria virtual ou fisica)!\n");
    return NULL;
}

void kfree(void* ptr) {
    if (ptr == NULL) return;

    // Obtém o cabeçalho recuando o tamanho do struct header
    header_t* header = (header_t*)ptr - 1;

    // Valida se o ponteiro de fato aponta para um bloco válido
    if (header->magic != KHEAP_MAGIC) {
        terminal_writestring("PANIC: Tentativa de kfree de ponteiro invalido ou corrompido!\n");
        while (1) {
            asm volatile("hlt");
        }
    }

    // Libera o bloco e junta blocos adjacentes
    header->is_free = true;
    kheap_coalesce();
}
