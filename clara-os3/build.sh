#!/bin/bash
set -e

echo "1. Montando o Assembly..."
nasm -f elf32 boot.asm -o boot.o

echo "2. Compilando os modulos em C..."
# Compilamos o kernel.c e o pmm.c
i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
i686-elf-gcc -c pmm.c -o pmm.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

echo "3. Linkando o Binario Final..."
# Juntamos o boot.o, kernel.o e pmm.o
i686-elf-gcc -T linker.ld -o clara_os.bin -ffreestanding -O2 -nostdlib boot.o kernel.o pmm.o -lgcc

echo "4. Iniciando QEMU..."
qemu-system-i386 -kernel clara_os.bin
