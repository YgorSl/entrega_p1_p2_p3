#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define MEM_SIZE 256         // Tamanho total da memória (256 bytes)
#define MAX_LINES 512        // Máximo de linhas do arquivo fonte
#define MAX_LINE_LEN 256     // Tamanho máximo de cada linha
#define MAX_LABEL_LEN 32     // Tamanho máximo para rótulos
#define MAX_SYMBOLS 128      // Máximo de símbolos (variáveis)
#define MAX_CODE_LINES 256   // Máximo de linhas de código
#define MAX_TOKENS 10        // Máximo de tokens por linha

// Estrutura para armazenar os símbolos definidos na seção DATA.
typedef struct {
    char label[MAX_LABEL_LEN];
    uint8_t value;   // Valor inicial (ou 0 se não inicializado, representado por ?)
    int assigned;    // Flag para indicar se já foi atribuído um endereço
    uint8_t addr;    // Endereço final alocado na memória
} Symbol;

// Estrutura para armazenar uma linha da seção CODE.
typedef struct {
    char tokens[MAX_TOKENS][MAX_LINE_LEN];
    int numTokens;
    int lineNum;     // Número da linha para mensagens de erro
} CodeLine;

// Variáveis globais para a tabela de símbolos e código.
Symbol symbolTable[MAX_SYMBOLS];
int symbolCount = 0;
CodeLine codeLines[MAX_CODE_LINES];
int codeLineCount = 0;
int codeOrigin = 0;  // Endereço inicial do código definido por .ORG

// Tabela de instruções suportadas pelo Neander.
typedef struct {
    char mnemonic[MAX_LABEL_LEN];
    uint8_t opcode;
    int operandCount;   // 0 ou 1
} Instruction;

Instruction instrTable[] = {
    {"NOP", 0x00, 0},
    {"STA", 0x10, 1},
    {"LDA", 0x20, 1},
    {"ADD", 0x30, 1},
    {"OR",  0x40, 1},
    {"AND", 0x50, 1},
    {"NOT", 0x60, 0},
    {"JMP", 0x80, 1},
    {"JN",  0x90, 1},
    {"JZ",  0xA0, 1},
    {"HLT", 0xF0, 0},
    {"",    0x00, 0}  // Sentinela
};

// Converte uma string para maiúsculas (para padronização).
void maiusculo(char *str) {
    for (; *str; ++str)
        *str = toupper((unsigned char)*str);
}

// Remove o newline do final de uma linha.
void quebra_linha(char *line) {
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[len - 1] = '\0';
        len--;
    }
}

// Remove comentários: descarta tudo a partir de ';'
void comentario(char *line) {
    char *p = strchr(line, ';');
    if (p)
        *p = '\0';
}

// Separa uma linha em tokens usando espaço e tabulação.
int tokenize(char *line, char tokens[MAX_TOKENS][MAX_LINE_LEN]) {
    int count = 0;
    char *token = strtok(line, " \t");
    while (token && count < MAX_TOKENS) {
        strncpy(tokens[count], token, MAX_LINE_LEN-1);
        tokens[count][MAX_LINE_LEN-1] = '\0';
        count++;
        token = strtok(NULL, " \t");
    }
    return count;
}

// Procura um mnemônico na tabela de instruções.
Instruction* instrucao(const char *mnemonic) {
    for (int i = 0; instrTable[i].mnemonic[0] != '\0'; i++) {
        if (strcmp(instrTable[i].mnemonic, mnemonic) == 0)
            return &instrTable[i];
    }
    return NULL;
}

// Procura um símbolo na tabela de símbolos.
int simbolo(const char *label) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(symbolTable[i].label, label) == 0)
            return i;
    }
    return -1;
}

// Primeira passagem: processa o arquivo fonte (arquivo TXT)
// separa as seções DATA e CODE, armazenando os símbolos e linhas de código.
int processa_txt(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Erro ao abrir o arquivo fonte");
        return 0;
    }
    
    char line[MAX_LINE_LEN];
    enum { NONE, DATA, CODE } section = NONE;
    int lineNum = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        lineNum++;
        quebra_linha(line);
        comentario(line);
        if (strlen(line) == 0)
            continue;
        
        // Detecta diretivas de seção.
        if (strcasecmp(line, ".DATA") == 0) {
            section = DATA;
            continue;
        } else if (strcasecmp(line, ".CODE") == 0) {
            section = CODE;
            continue;
        }
        
        if (section == DATA) {
            // Exemplo: A DB 50   ou   X DB ?
            char tokens[MAX_TOKENS][MAX_LINE_LEN];
            int ntok = tokenize(line, tokens);
            if (ntok < 3) {
                fprintf(stderr, "Erro na linha %d (DATA): %s\n", lineNum, line);
                continue;
            }
            strncpy(symbolTable[symbolCount].label, tokens[0], MAX_LABEL_LEN-1);
            symbolTable[symbolCount].label[MAX_LABEL_LEN-1] = '\0';
            if (strcasecmp(tokens[1], "DB") != 0) {
                fprintf(stderr, "Diretiva desconhecida na linha %d: %s\n", lineNum, tokens[1]);
                continue;
            }
            if (strcmp(tokens[2], "?") == 0)
                symbolTable[symbolCount].value = 0;
            else
                symbolTable[symbolCount].value = (uint8_t)atoi(tokens[2]);
            symbolTable[symbolCount].assigned = 0;
            symbolCount++;
        } else if (section == CODE) {
            char tokens[MAX_TOKENS][MAX_LINE_LEN];
            char line_copy[MAX_LINE_LEN];
            strcpy(line_copy, line);  // Faz uma cópia da linha original
            int ntok = tokenize(line_copy, tokens);
            
            printf("Linha %d tokens:", lineNum);
            for (int i = 0; i < ntok; i++) {
                printf(" [%s]", tokens[i]);
            }
            printf("\n");
            
            if (ntok >= 2 && strcasecmp(tokens[0], ".ORG") == 0) {
                codeOrigin = atoi(tokens[1]);
                continue;
            }
            if (codeLineCount < MAX_CODE_LINES) {
                // Faz outra cópia para tokenizar e armazenar na estrutura de código
                strcpy(line_copy, line);
                codeLines[codeLineCount].numTokens = tokenize(line_copy, codeLines[codeLineCount].tokens);
                codeLines[codeLineCount].lineNum = lineNum;
                codeLineCount++;
            } else {
                fprintf(stderr, "Numero maximo de linhas de codigo excedido.\n");
            }
        }
    }
    fclose(fp);
    return 1;
}

// Segunda passagem: gera o vetor de 256 bytes com o código e dados.
// A área de dados é alocada nos endereços mais altos da memória.
void second_pass(uint8_t assembled[MEM_SIZE]) {
    memset(assembled, 0, MEM_SIZE);
    
    int dataSize = symbolCount;  // Cada DB ocupa 1 byte
    int dataStart = MEM_SIZE - dataSize;
    
    // Atribui endereços aos símbolos e escreve seus valores.
    for (int i = 0; i < symbolCount; i++) {
        symbolTable[i].addr = dataStart + i;
        symbolTable[i].assigned = 1;
        assembled[symbolTable[i].addr] = symbolTable[i].value;
    }
    
    int pc = codeOrigin;
    for (int i = 0; i < codeLineCount; i++) {
        CodeLine *cl = &codeLines[i];
        if (cl->numTokens == 0)
            continue;
        maiusculo(cl->tokens[0]);
        Instruction *instr = instrucao(cl->tokens[0]);
        if (!instr) {
            fprintf(stderr, "Linha %d: Instrução desconhecida: %s\n", cl->lineNum, cl->tokens[0]);
            exit(1);
        }
        if (pc + 1 + instr->operandCount > MEM_SIZE) {
            fprintf(stderr, "Erro: codigo ultrapassa a memoria.\n");
            exit(1);
        }
        assembled[pc++] = instr->opcode;
        if (instr->operandCount == 1) {
            if (cl->numTokens < 2) {
                fprintf(stderr, "Linha %d: operando ausente para %s\n", cl->lineNum, cl->tokens[0]);
                exit(1);
            }
            int operand;
            if (isdigit(cl->tokens[1][0])) {
                operand = atoi(cl->tokens[1]);
            } else {
                char opCopy[MAX_LABEL_LEN];
                strncpy(opCopy, cl->tokens[1], MAX_LABEL_LEN-1);
                opCopy[MAX_LABEL_LEN-1] = '\0';
                maiusculo(opCopy);
                int index = simbolo(opCopy);
                if (index < 0) {
                    fprintf(stderr, "Linha %d: símbolo não definido: %s\n", cl->lineNum, cl->tokens[1]);
                    exit(1);
                }
                operand = symbolTable[index].addr;
            }
            assembled[pc++] = (uint8_t)operand;
        }
    }
}

// Grava o vetor de 256 bytes em um arquivo .mem compatível com o Neander.
// O formato gerado terá 516 bytes: 4 bytes de cabeçalho fixo e 512 bytes representando os 256 bytes
// em formato ASCII hexadecimal (2 caracteres por byte).
void write_mem_file(const char *filename, uint8_t assembled[MEM_SIZE]) {
    FILE *out = fopen(filename, "wb");
    if (!out) {
        perror("Erro ao abrir arquivo de saida");
        exit(EXIT_FAILURE);
    }
    // Cabeçalho fixo de 4 bytes (por exemplo, 0x03, 'N', 'D', 'R')
    uint8_t header[4] = { 0x03, 0x4E, 0x44, 0x52 };
    if (fwrite(header, 1, 4, out) != 4) {
        fprintf(stderr, "Erro ao escrever cabecalho.\n");
        fclose(out);
        exit(EXIT_FAILURE);
    }

    // Para cada endereço (0..255), escrevemos:
    // 1) O byte real de memória
    // 2) Um byte adicional (normalmente 0x00)
    for (int i = 0; i < MEM_SIZE; i++) {
        uint8_t val = assembled[i];
        fputc(val, out);      // byte real
        fputc(0x00, out);     // segundo byte (0x00)
    }

    fclose(out);
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo.txt>\n", argv[0]);
        return 1;
    }
    
    // Primeira passagem: processa o arquivo fonte (.txt)
    if (!processa_txt(argv[1])) {
        return 1;
    }
    
    // Debug: exibe a tabela de símbolos e o número de linhas de código lidas.
    printf("Tabela de símbolos (DATA):\n");
    for (int i = 0; i < symbolCount; i++) {
        printf("  %s, valor=%d\n", symbolTable[i].label, symbolTable[i].value);
    }
    printf("Linhas de código lidas: %d\n", codeLineCount);
    
    uint8_t assembled[MEM_SIZE];
    second_pass(assembled);
    
    // Grava o arquivo .mem compatível (516 bytes) com cabeçalho e dados em ASCII hexadecimal.
    write_mem_file("saida.mem", assembled);
    printf("Assembler finalizado com sucesso.\n");
    printf("Arquivo .mem gerado: saida.mem\n");
    
    return 0;
}
