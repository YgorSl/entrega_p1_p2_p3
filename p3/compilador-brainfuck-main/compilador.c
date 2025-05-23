// compilador_bf.c — gera Brainfuck puro em stdout a partir de nome="expr"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

const char *p;
int expr(), term(), factor();

// parser recursivo: +, -, *, / e parênteses
int expr() {
    int v = term();
    while (*p=='+'||*p=='-') {
        char op = *p++;
        int v2 = term();
        v = (op=='+') ? v+v2 : v-v2;
    }
    return v;
}

int term() {
    int v = factor();
    while (*p=='*'||*p=='/') {
        char op = *p++;
        int v2 = factor();
        v = (op=='*') ? v*v2 : v/v2;
    }
    return v;
}

int factor() {
    if (*p=='(') {
        p++;
        int v = expr();
        if (*p==')') p++;
        return v;
    }
    if (!isdigit(*p)) {
        fprintf(stderr, "Erro de sintaxe em \"%s\"\n", p);
        exit(1);
    }
    int v = 0;
    while (isdigit(*p)) {
        v = v*10 + (*p - '0');
        p++;
    }
    return v;
}

// escreve em stdout o Brainfuck para imprimir byte 'c' na célula target_cell
void bf_print_byte(int *cur_cell, int target_cell, unsigned char c) {
    int delta = target_cell - *cur_cell;
    if (delta > 0) {
        for (int i = 0; i < delta; i++) putchar('>');
    } else {
        for (int i = 0; i < -delta; i++) putchar('<');
    }
    *cur_cell = target_cell;
    // zera e seta
    fputs("[-]", stdout);
    for (int i = 0; i < c; i++) putchar('+');
    // imprime
    putchar('.');
}

// imprime string UTF-8 byte a byte
void bf_print_string(int *cur_cell, const char *s) {
    int cell = 0;
    for (size_t i = 0; i < strlen(s); ++i) {
        bf_print_byte(cur_cell, cell++, (unsigned char)s[i]);
    }
}

int main(int argc, char **argv) {
    char buffer[256];

    // lê de argv ou stdin
    if (argc == 2) {
        strncpy(buffer, argv[1], sizeof(buffer)-1);
        buffer[sizeof(buffer)-1] = '\0';
    } else {
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            return 0;
        }
        buffer[strcspn(buffer, "\r\n")] = '\0';
    }

    // separa nome e expressão
    char *eq = strchr(buffer, '=');
    if (!eq) {
        fprintf(stderr, "Formato inválido: nome=expr\n");
        return 1;
    }
    // nome da variável
    char varname[128];
    int n = eq - buffer;
    if (n >= (int)sizeof(varname)) n = sizeof(varname)-1;
    strncpy(varname, buffer, n);
    varname[n] = '\0';

    // início da expressão
    char *expr_start = eq + 1;
    if (*expr_start == '"') {
        expr_start++;
        char *endq = strrchr(expr_start, '"');
        if (endq) *endq = '\0';
    }

    // avalia
    p = expr_start;
    int result = expr();

    // monta saída texto “nome=valor”
    char outtxt[256];
    snprintf(outtxt, sizeof(outtxt), "%s=%d", varname, result);

    // gera Brainfuck puro
    int cur_cell = 0;
    bf_print_string(&cur_cell, outtxt);
    putchar('\n');

    return 0;
}
