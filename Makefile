CC = gcc
CFLAGS = -std=c89 -pedantic-errors -Wall -Werror -O3
AS = nasm
ASFLAGS = -f elf64
# AS = gcc
# ASFLAGS = -S -masm=intel
LD = gcc

default: build

clean:
	rm -rf out

build: out/sepia_sse.asm.o out/main.c.o out/image.c.o out/bmp.c.o
	$(LD) out/*.o -o out/lab8

out/%.asm.o: src/%.asm
	mkdir -p out
	$(AS) $(ASFLAGS) src/$*.asm -o out/$*.asm.o

out/%.c.o: src/%.c
	mkdir -p out
	$(CC) $(CFLAGS) src/$*.c -c -o out/$*.c.o
