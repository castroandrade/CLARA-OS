# Ferramentas do Compilador Cruzado (Disponíveis na imagem Docker)
CC = i686-elf-gcc
AS = nasm
LD = i686-elf-gcc

# Flags de Compilação
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra
ASFLAGS = -f elf32
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib -lgcc

# Objetos a serem compilados
OBJS = boot.o kernel.o pmm.o paging.o kheap.o

# Alvo principal
all: clara_os.bin

# Compilação do binário final (Linking)
clara_os.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

# Compilação dos arquivos C
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

# Montagem do boot.asm
boot.o: boot.asm
	$(AS) $(ASFLAGS) $< -o $@

# Limpeza dos objetos intermediários
clean:
	rm -f *.o clara_os.bin
