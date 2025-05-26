#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_TOKEN_LEN 64

typedef enum {
    T_KEYWORD, T_IDENTIFIER, T_NUMBER,
    T_OPERATOR, T_SEPARATOR, T_EOF
} TokenType;

const char *keywords[] = { "var", "if", "else", "while", "function", "return", NULL };

typedef struct {
    TokenType type;
    char value[MAX_TOKEN_LEN];
} Token;

int is_keyword(const char *str) {
    for (int i = 0; keywords[i]; i++) {
        if (strcmp(str, keywords[i]) == 0) return 1;
    }
    return 0;
}

void next_token(FILE *fp, Token *token) {
    int c;
    // skip whitespace
    while (isspace(c = fgetc(fp)));

    if (c == EOF) {
        token->type = T_EOF;
        strcpy(token->value, "EOF");
        return;
    }

    // identifier or keyword
    if (isalpha(c)) {
        int i = 0;
        token->value[i++] = c;
        while (isalnum(c = fgetc(fp)) || c == '_') {
            if (i < MAX_TOKEN_LEN - 1) token->value[i++] = c;
        }
        token->value[i] = '\0';
        ungetc(c, fp);
        token->type = is_keyword(token->value) ? T_KEYWORD : T_IDENTIFIER;
        return;
    }

    // number
    if (isdigit(c)) {
        int i = 0;
        token->value[i++] = c;
        while (isdigit(c = fgetc(fp))) {
            if (i < MAX_TOKEN_LEN - 1) token->value[i++] = c;
        }
        token->value[i] = '\0';
        ungetc(c, fp);
        token->type = T_NUMBER;
        return;
    }

    // operators and separators
    if (strchr("=+-*/(){};,", c)) {
        token->type = strchr("(){};,", c) ? T_SEPARATOR : T_OPERATOR;
        token->value[0] = c;
        token->value[1] = '\0';
        return;
    }

    // handle multi-char operators (==, !=, >=, <=)
    if (c == '>' || c == '<' || c == '=' || c == '!') {
        int next = fgetc(fp);
        if (next == '=') {
            token->type = T_OPERATOR;
            token->value[0] = c;
            token->value[1] = '=';
            token->value[2] = '\0';
        } else {
            ungetc(next, fp);
            token->type = T_OPERATOR;
            token->value[0] = c;
            token->value[1] = '\0';
        }
        return;
    }

    // unknown character
    fprintf(stderr, "Erro: caractere inválido '%c'\n", c);
    exit(1);
}

int main(int argc, char *argv[]) {
    FILE *fp = stdin;
    if (argc == 2) {
        fp = fopen(argv[1], "r");
        if (!fp) {
            perror("Erro ao abrir arquivo");
            return 1;
        }
    }

    Token token;
    while (1) {
        next_token(fp, &token);
        if (token.type == T_EOF) break;

        printf("Token: %-12s Tipo: %d\n", token.value, token.type);
    }

    if (fp != stdin) fclose(fp);
    return 0;
}

