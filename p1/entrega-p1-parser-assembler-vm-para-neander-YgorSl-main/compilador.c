/* parser_neander_compilador.c - Optimized temporary variable usage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#define MAX_LINE 256
#define MAX_VAR 100
#define MAX_CODE_LINES 1024
#define MAX_TOKENS 20
#define MAX_TEMP_VARS 6



typedef struct {
    char nome[16];
    int valor;
    int inicializado;
} Variavel;

Variavel variaveis[MAX_VAR];
int var_count = 0;

char codigo_gerado[MAX_CODE_LINES][MAX_LINE];
int linhas_codigo = 0;
int temp_var_counter = 0;

int endereco_atual = 0;  // Contador de bytes (endereços de memória)
int tamanho_instrucao[MAX_CODE_LINES];  // Armazena o tamanho (1 ou 2) de cada linha

FILE *out;

// Tokenização (simplificada)
typedef struct {
    char tipo;  // 'n'=número, 'v'=variável, 'o'=operador, '(', ')'
    char valor[16];
} Token;

Token tokens[MAX_TOKENS];
int num_tokens = 0;
int token_atual = 0;


char* parse_fator();
void tokenizar(const char *expr);
int encontrar_variavel(const char *nome);
void declarar_variavel(const char *nome, int valor, int inicializado);
char* parse_termo();
char* parse_expressao();
void escrever_data_section();
void escrever_code_header();
void emitir(const char *linha_fmt, ...);
char* get_temp_var();
void free_temp_var(char *temp_name);
void gerar_expressao(const char *dest, const char *expr);
void processar_linha(char *linha);

void tokenizar(const char *expr) {
    num_tokens = 0;
    char temp[256];
    strcpy(temp, expr);
    char *ptr = temp;

    while (*ptr != '\0') {
        if (isspace(*ptr)) {
            ptr++;
            continue;
        }
        if (isdigit(*ptr)) {
            sscanf(ptr, "%[0-9]", tokens[num_tokens].valor);
            tokens[num_tokens].tipo = 'n';
            ptr += strlen(tokens[num_tokens].valor);
        } else if (isalpha(*ptr)) {
            sscanf(ptr, "%[a-zA-Z]", tokens[num_tokens].valor);
            tokens[num_tokens].tipo = 'v';
            ptr += strlen(tokens[num_tokens].valor);
        } else {
            tokens[num_tokens].tipo = 'o';
            tokens[num_tokens].valor[0] = *ptr;
            tokens[num_tokens].valor[1] = '\0';
            ptr++;
        }
        num_tokens++;
    }
}


int encontrar_variavel(const char *nome) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variaveis[i].nome, nome) == 0) return i;
    }
    return -1;
}

void declarar_variavel(const char *nome, int valor, int inicializado) {
    int idx = encontrar_variavel(nome);
    if (idx != -1) {
        variaveis[idx].valor = valor;
        variaveis[idx].inicializado = inicializado;
        return;
    }
    strcpy(variaveis[var_count].nome, nome);
    variaveis[var_count].valor = valor;
    variaveis[var_count].inicializado = inicializado;
    var_count++;
}



void escrever_data_section() {
    fprintf(out, ".DATA\n");
    for (int i = 0; i < var_count; i++) {
        if (variaveis[i].inicializado)
            fprintf(out, "%s DB %d\n", variaveis[i].nome, variaveis[i].valor);
        else
            fprintf(out, "%s DB ?\n", variaveis[i].nome);
    }
    // Temporary variables
    fprintf(out, "TMP1 DB 0\nTMP2 DB 0\nTMP3 DB 0\nTMP4 DB 0\nTMP5 DB 0\nTMP6 DB 0\n");
    // Constants
    fprintf(out, "UM DB 1\nZERO DB 0\nSINAL DB 0\nSUB DB 255");
}

void escrever_code_header() {
    fprintf(out, "\n.CODE\n.ORG 0\n");
}

void emitir(const char *linha_fmt, ...) {
    va_list args;
    va_start(args, linha_fmt);
    vsnprintf(codigo_gerado[linhas_codigo], MAX_LINE, linha_fmt, args);
    va_end(args);

    // Determina o tamanho da instrução (1 ou 2 bytes)
    if (strstr(codigo_gerado[linhas_codigo], "HLT") != NULL || 
        strstr(codigo_gerado[linhas_codigo], "NOT") != NULL) {
        tamanho_instrucao[linhas_codigo] = 1;  // NOT e HLT: 1 byte
    } else {
        tamanho_instrucao[linhas_codigo] = 2;  // Demais instruções: 2 bytes
    }

    endereco_atual += tamanho_instrucao[linhas_codigo];
    linhas_codigo++;
}

char* get_temp_var() {
    // Aloca memória dinamicamente para cada variável temporária
    char *temp_name = malloc(10); // TMPx\0 (4 bytes + espaço extra)
    
    temp_var_counter = (temp_var_counter % MAX_TEMP_VARS) + 1;
    sprintf(temp_name, "TMP%d", temp_var_counter);
    
    return temp_name;
}

void free_temp_var(char *temp_name) {
    free(temp_name); // Libera a memória alocada
}

// Updated parse_fator function
char* parse_fator() {
    if (token_atual >= num_tokens) return NULL;

    Token tk = tokens[token_atual];
    token_atual++;
    
    if (tk.tipo == 'n' || tk.tipo == 'v') {
        return strdup(tk.valor);
    } else if (tk.tipo == 'o' && tk.valor[0] == '(') {
        char *resultado = parse_expressao();
        if (token_atual >= num_tokens || tokens[token_atual].tipo != 'o' || tokens[token_atual].valor[0] != ')') {
            printf("Erro: ')' esperado\n");
            exit(1);
        }
        token_atual++;
        return resultado;
    } else {
        printf("Erro: Fator inválido\n");
        exit(1);
    }
}

// Updated parse_termo function
char* parse_termo() {
    char *esquerda = parse_fator();
    
    while (token_atual < num_tokens) {
        Token tk = tokens[token_atual];
        if (tk.tipo == 'o' && (tk.valor[0] == '*' || tk.valor[0] == '/')) {
            char op = tk.valor[0];
            token_atual++;
            char *direita = parse_fator();
            
            // Guardar os valores em strings para usar na função original
            char op_str[2] = {op, '\0'};
            
            // Salvar o código gerado atual e restaurar depois
            int codigo_backup = linhas_codigo;
            int endereco_backup = endereco_atual;
            
            // Gerar código para a operação
            emitir("LDA %s", esquerda);
            if (op == '*') {
                char *temp_var1 = get_temp_var();
                char *temp_var2 = get_temp_var();
                char *temp_res = get_temp_var();
                
                // Código da multiplicação (existente)
                emitir("JN %d", endereco_atual+46);
                emitir("STA %s", temp_var1);
                int validaoperador2 = endereco_atual; 
                emitir("LDA %s", direita);
                emitir("JN %d", endereco_atual+52);
                emitir("STA %s", temp_var2);
                int vaiproloop = endereco_atual;
                emitir("LDA ZERO");
                emitir("STA %s", temp_res);
                emitir("LDA %s", temp_var2);
                int loop = endereco_atual;
                emitir("JZ %d", endereco_atual+16);
                emitir("LDA %s", temp_res);
                emitir("ADD %s", temp_var1);
                emitir("STA %s", temp_res);
                emitir("LDA %s", temp_var2);
                emitir("ADD SUB");
                emitir("STA %s", temp_var2);
                emitir("JMP %d", loop);
                emitir("LDA SINAL");
                emitir("JZ %d", endereco_atual+9);
                emitir("LDA %s", temp_res);
                emitir("NOT");
                emitir("ADD UM");
                emitir("STA %s", temp_res);
                emitir("JMP %d", endereco_atual+26);
                emitir("NOT");
                emitir("ADD UM");
                emitir("STA %s", temp_var1);
                emitir("LDA SINAL");
                emitir("NOT");
                emitir("STA SINAL");
                emitir("JMP %d", validaoperador2);
                emitir("NOT");
                emitir("ADD UM");
                emitir("STA %s", temp_var2);
                emitir("LDA SINAL");
                emitir("NOT");
                emitir("STA SINAL");
                emitir("JMP %d", vaiproloop);
                emitir("LDA %s", temp_res);
                
                free_temp_var(temp_var1);
                free_temp_var(temp_var2);
                free_temp_var(temp_res);
            } else { // Divisão
                char *temp_dividend = get_temp_var();
                char *temp_divisor = get_temp_var();
                char *temp_neg_divisor = get_temp_var();
                char *temp_counter = get_temp_var();
                
                // Código da divisão (existente)
                emitir("STA %s", temp_dividend);
                emitir("LDA %s", direita);
                emitir("STA %s", temp_divisor);
                emitir("NOT");
                emitir("ADD UM");
                emitir("STA %s", temp_neg_divisor);
                emitir("LDA ZERO");
                emitir("STA %s", temp_counter);
                int inicio_loop = endereco_atual;
                emitir("LDA %s", temp_dividend);
                emitir("ADD %s", temp_neg_divisor);
                emitir("STA %s", temp_dividend);
                emitir("JN %d", endereco_atual+7);
                emitir("LDA %s", temp_counter);
                emitir("ADD UM");
                emitir("STA %s", temp_counter);
                emitir("JMP %d", inicio_loop);
                emitir("LDA %s", temp_counter);
                
                free_temp_var(temp_dividend);
                free_temp_var(temp_divisor);
                free_temp_var(temp_neg_divisor);
                free_temp_var(temp_counter);
            }
            
            // Alocar um novo temp_var para o resultado
            char *temp = get_temp_var();
            emitir("STA %s", temp);
            
            free(esquerda);
            free(direita);
            esquerda = temp;
        } else {
            break;
        }
    }
    
    return esquerda;
}

// Updated parse_expressao function
char* parse_expressao() {
    char *esquerda = parse_termo();
    
    while (token_atual < num_tokens) {
        Token tk = tokens[token_atual];
        if (tk.tipo == 'o' && (tk.valor[0] == '+' || tk.valor[0] == '-')) {
            char op = tk.valor[0];
            token_atual++;
            char *direita = parse_termo();
            char *temp = get_temp_var();
            
            // Código para soma/subtração
            emitir("LDA %s", esquerda);
            if (op == '+') {
                emitir("ADD %s", direita);
            } else { // Subtração
                emitir("STA %s", temp);
                emitir("LDA %s", direita);
                emitir("NOT");
                emitir("ADD UM");
                emitir("ADD %s", temp);
            }
            emitir("STA %s", temp);
            
            free(esquerda);
            free(direita);
            esquerda = temp;
        } else {
            break;
        }
    }
    
    return esquerda;
}

// Minimally modified gerar_expressao
void gerar_expressao(const char *dest, const char *expr) {
    // Primeiro tokenize a expressão usando a função existente
    tokenizar(expr);
    token_atual = 0;
    
    // Verifica se é expressão simples (um único token)
    if (num_tokens == 1) {
        emitir("LDA %s", tokens[0].valor);
        emitir("STA %s", dest);
        return;
    }
    
    // Se tiver mais de um token, use o parser recursivo para ordem de precedência
    char *resultado = parse_expressao();
    
    // Carrega o resultado final e armazena no destino
    emitir("LDA %s", resultado);
    emitir("STA %s", dest);
    
    // Libera a memória alocada pelo parser
    free(resultado);
}

void processar_linha(char *linha) {
    while (*linha == ' ') linha++;

    if (strncmp(linha, "VAR", 3) == 0) {
        char nome[16];
        if (sscanf(linha, "VAR %s", nome) == 1) {
            declarar_variavel(nome, 0, 0);
        }
    } else if (strchr(linha, '=') != NULL) {
        char var[16], expr[256];
        int val;
        
        if (sscanf(linha, "%s = %d", var, &val) == 2) {
            declarar_variavel(var, val, 1);
        } 
        else if (sscanf(linha, "%s = %[^\n]", var, expr) == 2) {
            gerar_expressao(var, expr);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo.lpn>\n", argv[0]);
        return 1;
    }

    FILE *in = fopen(argv[1], "r");
    out = fopen("programa.asm", "w");

    if (!in || !out) {
        printf("Erro ao abrir arquivos.\n");
        return 1;
    }

    char linha[MAX_LINE];
    int dentro_bloco = 0;

    while (fgets(linha, sizeof(linha), in)){
        linha[strcspn(linha, "\n")] = 0;
        
        if (strncmp(linha, "INICIO", 6) == 0) {
            dentro_bloco = 1;
            continue;
        }
        if (strncmp(linha, "FIM", 3) == 0) {
            escrever_data_section();
            escrever_code_header();
            for (int i = 0; i < linhas_codigo; i++) {
                fprintf(out, "%s\n", codigo_gerado[i]);
            }
            fprintf(out, "HLT\n");
            break;
        }
        if (dentro_bloco) {
            processar_linha(linha);
        }
    }

    fclose(in);
    fclose(out);
    printf("programa.asm gerado com sucesso.\n");
    return 0;
}