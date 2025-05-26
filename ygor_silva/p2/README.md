# compilador-brainfuck

Brainfuck Toolchain

Este repositório contém duas ferramentas separadas para gerar e executar programas Brainfuck a partir de expressões aritméticas com suporte a nomes de variáveis em UTF-8.


Requisitos

Linux 64 bits (testado em Arch Linux)

GCC ou clang

Compilação



compile manualmente:

gcc -o compilador_bf compilador_bf.c
gcc -o bfexec bfexec.c

Uso

O pipeline completo roda via pipe:

echo 'preço=2+2*3' | ./compilador_bf | ./bfexec
# Saída: preço=8

Passo a passo

echo 'varão=1+1' — produz a string de entrada com nome em UTF-8.

compilador_bf — lê nome=expr de stdin, avalia a expressão e gera Brainfuck puro em stdout.

bfexec — lê o Brainfuck de stdin, interpreta e imprime o resultado diretamente (raw UTF-8).

Exemplos

$ echo 'valor=1+3*2' | ./compilador_bf | ./bfexec
valor=7

$ echo 'preço=10+5' | ./compilador_bf | ./bfexec
preço=15

Tratamento de erros

Se a expressão estiver fora do formato nome=expr, o compilador exibirá mensagem de erro em stderr.

Qualquer erro de sintaxe na expressão é relatado em stderr.

Limitações conhecidas

O parser suporta +, -, *,

Apenas inteiros positivos são aceitos.

O executor assume fita de 30 000 células.


