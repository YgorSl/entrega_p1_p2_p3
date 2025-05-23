Compilador Neander

Autor: Ygor Silva

Ambiente: Linux 64 bits (recomendado: GCC)

Compilação:
Rodar: make
gera os executaveis necessarios, o programa.lpn é um exemplo de codigo aceito pelo compilador
o padrao do programa deve seguir o exemplo declaração de variaveis, atribuição de valores, e por fim o calculo da expressão

No final o execitor do binario imprimira os resultados  no final da memoria disponivel



Visão Geral

Este projeto implementa um toolchain completo para a arquitetura Neander, incluindo:

Um compilador que traduz arquivos .lpn com expressões aritméticas para código assembly .asm.

Um assembler que traduz código .asm para arquivos binários .mem com cabeçalho.

Um executor que carrega e executa arquivos .mem simulando o funcionamento da arquitetura Neander

Limitações:
Divisão feita apenas com numeros positivos.
Arquivos com muitos calculos não irao funcionar devido ao limite de memoria.
