# CLARA OS: CLAssroom for Real Apprenticeship of Operating Systems

O **CLARA OS** é um sistema operacional didático minimalista desenvolvido do zero para a arquitetura **Intel x86 (32-bit, Modo Protegido)** seguindo a metodologia **Bare Bones** presente no OSDev Wiki. O sistema é freestanding (sem dependências de sistemas operacionais hospedeiros) e visa conectar conceitos teóricos de Sistemas Operacionais (gerência de memória, processos e escalonamento) com a prática de baixo nível (C e Assembly).

---

## 📁 Estrutura do Repositório

O projeto é desenvolvido de forma incremental, dividido em diferentes versões presentes nas seguintes pastas:

*   **`clara-os/`**: Estrutura inicial mínima, bootloader em Assembly NASM e a transição básica para C.
*   **`clara-os2/`**: Integração do driver VGA básico para escrita direta em tela e formatação de texto em modo texto.
*   **`clara-os3/`**: Implementação do Gerenciador de Memória Física (PMM) baseado em Bitmap lendo a estrutura de mapa de memória (mmap) do Multiboot.
*   **`clara-os4/` (Mais Recente)**: Versão ativa. Possui paginação de memória ativa (Identity Mapping para os primeiros 4 MB) e o **Kernel Heap Allocator** (`kmalloc`/`kfree`) com suporte a divisão de blocos (splitting), crescimento de heap virtual sob demanda e coalescência de memória.

---

## 🛠️ Arquitetura do Ambiente de Desenvolvimento

Para garantir a **replicabilidade** em qualquer máquina de desenvolvimento (especialmente Windows 10/11), o fluxo de trabalho adota um modelo híbrido:

1.  **Compilação Cruzada (Docker):** O compilador `i686-elf-gcc`, `nasm` e `make` são executados de forma isolada dentro de um container Docker. Os arquivos fonte são montados no container, que gera e exporta o binário final `clara_os.bin`.
2.  **Emulação (QEMU Nativo):** O emulador QEMU é executado localmente na máquina hospedeira para carregar o binário `clara_os.bin` de forma rápida e com suporte gráfico nativo.

---

## 🚀 Como Executar o CLARA OS (clara-os4) no Windows

### 1. Pré-requisitos
*   **Docker Desktop** (com backend WSL2 ativo e rodando).
*   **QEMU para Windows** instalado:
    *   Recomenda-se a instalação clássica via [QEMU Windows](https://www.qemu.org/download/#windows) ou via terminal **MSYS2 UCRT64** (`pacman -S mingw-w64-ucrt-x86_64-qemu`).
    *   *Nota: Caso o QEMU não esteja configurado nas variáveis de ambiente (PATH), o script de build tentará localizá-lo automaticamente nos caminhos padrão (como `C:\msys64\ucrt64\bin`).*

### 2. Compilar e Executar
Abra um terminal do PowerShell dentro da pasta da versão mais recente (`clara-os4/`) e execute o script de automação:

```powershell
cd clara-os4
.\build-run.ps1
```

O script irá:
1.  Verificar e construir a imagem docker de compilação local `clara-os-builder` (caso seja a primeira execução).
2.  Compilar os códigos Assembly e C gerando o binário `clara_os.bin`.
3.  Iniciar o emulador QEMU nativo carregando o kernel compilado.

A tela clássica do QEMU se abrirá exibindo o progresso dos testes dos subsistemas do kernel (MMU online, PMM ativo, testes de alocação de Heap passando).

---

## 📝 Próximos Passos no Desenvolvimento
O projeto se encontra no final da **Fase de Gerenciamento de Memória**. A próxima etapa compreende o desenvolvimento do subsistema de **Processos e Escalonamento Cooperativo**:
*   Definição do Bloco de Controle de Processo (`task_struct`).
*   Implementação da rotina de troca de contexto em Assembly (`switch_to`).
*   Criação de um escalonador circular (Round-Robin).
*   Implementação das funções de controle de fluxo cooperativo (`yield()`) e duplicação (`fork()`).
