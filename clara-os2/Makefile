CC = i686-elf-gcc
AS = nasm
LD = i686-elf-gcc

CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra
ASFLAGS = -f elf32
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib -lgcc

OBJS = boot.o kernel.o

all: clara_os.bin

clara_os.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

boot.o: boot.asm
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f *.o clara_os.bin
