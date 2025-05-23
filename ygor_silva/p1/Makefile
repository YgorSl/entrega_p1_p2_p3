# Makefile para Compilador Neander

CC = gcc
CFLAGS = -Wall -Wextra -O2

# Alvos
all: compilador assembler executor

compilador: compilador.c
	$(CC) $(CFLAGS) -o compilador compilador.c

assembler: assembler.c
	$(CC) $(CFLAGS) -o assembler assembler.c

executor: executor.c
	$(CC) $(CFLAGS) -o executor executor.c

clean:
	rm -f compilador assembler executor programa.asm saida.mem

.PHONY: all clean
