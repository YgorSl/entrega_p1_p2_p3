// bfexec.c — Executor Brainfuck que lê de stdin se não receber arquivo

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
  #include <windows.h>
#endif

#define TAPE_SIZE 30000
#define PROG_SIZE 131072

int main(int argc, char *argv[]) {
    #ifdef _WIN32
      SetConsoleOutputCP(CP_UTF8);
    #endif

    FILE *f;
    if (argc == 2) {
        f = fopen(argv[1], "r");
        if (!f) {
            perror("Erro ao abrir arquivo .bf");
            return 1;
        }
    } else {
        // lê de stdin
        f = stdin;
    }

    // carrega todo o código BF
    char *prog = malloc(PROG_SIZE);
    if (!prog) {
        fprintf(stderr, "Memória insuficiente\n");
        if (argc == 2) fclose(f);
        return 1;
    }
    size_t len = fread(prog, 1, PROG_SIZE-1, f);
    prog[len] = '\0';
    if (argc == 2) fclose(f);

    // inicializa fita e pilha de loops
    unsigned char *tape = calloc(TAPE_SIZE, 1);
    size_t *stack = malloc(PROG_SIZE * sizeof(size_t));
    if (!tape || !stack) {
        fprintf(stderr, "Memória insuficiente\n");
        free(prog);
        return 1;
    }

    size_t pc = 0, dp = 0, sp = 0;
    while (pc < len) {
        switch (prog[pc]) {
            case '>': dp = (dp + 1) % TAPE_SIZE; break;
            case '<': dp = (dp == 0 ? TAPE_SIZE - 1 : dp - 1); break;
            case '+': tape[dp]++; break;
            case '-': tape[dp]--; break;
            case '.': putchar(tape[dp]); break;
            case ',': {
                int c = getchar();
                tape[dp] = (c == EOF ? 0 : (unsigned char)c);
                break;
            }
            case '[':
                if (tape[dp]) {
                    stack[sp++] = pc;
                } else {
                    int nest = 1;
                    while (nest && ++pc < len) {
                        if (prog[pc] == '[') nest++;
                        else if (prog[pc] == ']') nest--;
                    }
                }
                break;
            case ']':
                if (sp == 0) {
                    fprintf(stderr, "']' sem '[' correspondente em pc=%zu\n", pc);
                    goto cleanup;
                }
                if (tape[dp]) {
                    pc = stack[sp - 1];
                } else {
                    sp--;
                }
                break;
            default:
                // ignora qualquer outro caractere
                break;
        }
        pc++;
    }

cleanup:
    free(prog);
    free(tape);
    free(stack);
    return 0;
}
