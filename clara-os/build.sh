#!/bin/bash
set -e # Para se der erro

echo "1. Montando o Assembly..."
nasm -f elf32 boot.asm -o boot.o

echo "2. Compilando o Kernel C..."
# Certifique-se que o i686-elf-gcc esta no seu PATH
i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

echo "3. Linkando o Binario Final..."
i686-elf-gcc -T linker.ld -o clara_os.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc

echo "4. Verificando Multiboot..."
if grub-file --is-x86-multiboot clara_os.bin; then
  echo "   Sucesso: O kernel e compativel com Multiboot!"
else
  echo "   Erro: O kernel nao e compativel com Multiboot."
  exit 1
fi

echo "========================================"
echo "Tudo pronto! Iniciando QEMU..."
echo "========================================"
qemu-system-i386 -kernel clara_os.bin
