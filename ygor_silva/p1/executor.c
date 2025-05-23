#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "neander.h"


#define MEM_SIZE 256

uint8_t memory[MEM_SIZE];


uint8_t AC = 0;  
uint8_t PC = 0; 
uint8_t N = 0;   
uint8_t Z = 0;   

void update_flags() {
    N = (AC >> 7) & 1; 
    Z = (AC == 0);     
}


void sta(uint8_t address) { memory[address] = AC; }
void lda(uint8_t address) { AC = memory[address]; update_flags(); }
void add(uint8_t address) { AC += memory[address]; update_flags(); }
void or(uint8_t address)  { AC |= memory[address]; update_flags(); }
void and(uint8_t address) { AC &= memory[address]; update_flags(); }
void not()               { AC = ~AC; update_flags(); }
void jmp(uint8_t address){ PC = address; }
void jmp_n(uint8_t address) { if (N) PC = address; else PC += 2; }
void jz(uint8_t address) { if (Z) PC = address; else PC += 2; }

void run() {
    while (1) {  
        uint8_t instruction = memory[PC];
        uint8_t operand = memory[PC + 1];

        switch (instruction) {
            case 0x00: 
                PC += 1;
                break;
            case 0x10: 
                sta(operand);
                PC += 2;
                break;
            case 0x20: 
                lda(operand);
                PC += 2;
                break;
            case 0x30: 
                add(operand);
                PC += 2;
                break;
            case 0x40: 
                or(operand);
                PC += 2;
                break;
            case 0x50: 
                and(operand);
                PC += 2;
                break;
            case 0x60: 
                not();
                PC += 1;
                break;
            case 0x80: 
                jmp(operand);
                break;
            case 0x90: 
                jmp_n(operand);
                break;
            case 0xA0: 
                jz(operand);
                break;
            case 0xF0: 
                printf("Execucao interrompida (HLT).\n");
                return;
            default:
                printf("Instrucao desconhecida: %02X em PC: %02X\n", instruction, PC);
                return;
        }
        printf("PC: %02X, AC: %02X, N: %d, Z: %d\n", PC, AC, N, Z);
    }
    printf("PC ultrapassou a memoria. Execucao encerrada.\n");
}


void dump_memory() {
    printf("Memoria:\n");
    for (uint8_t i = 0; i < 16; i++) {
        for (uint8_t j = 0; j < 16; j++) {
            printf("%02X ", memory[i * 16 + j]);
        }
        printf("\n");
    }
}


void load_program(uint8_t program[], uint8_t size) {
    for (uint8_t i = 0; i < size; i++) {
        memory[i] = program[i];
    }
}


void load_mem_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }
    
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (fsize == 516) {
 
        uint8_t header[4];
        if (fread(header, 1, 4, file) != 4) {
            fprintf(stderr, "Erro ao ler o cabecalho do arquivo.\n");
            fclose(file);
            exit(EXIT_FAILURE);
        }
       
        for (int i = 0; i < MEM_SIZE; i++) {
            uint8_t byte;
            if (fread(&byte, 1, 1, file) != 1) {
                fprintf(stderr, "Erro ao ler o byte da memoria.\n");
                fclose(file);
                exit(EXIT_FAILURE);
            }
            memory[i] = byte;
          
            fseek(file, 1, SEEK_CUR);
        }
    } else {
        fprintf(stderr, "Tamanho inesperado de arquivo: %ld bytes\n", fsize);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc >= 2) {
        printf("Carregando arquivo: %s\n", argv[1]);
        load_mem_file(argv[1]);
    } else {
        printf("Nenhum arquivo fornecido.\n");
        return 0;
    }

    printf("Estado inicial da memoria:\n");
    dump_memory();

    run();

    printf("Estado final da memoria:\n");
    dump_memory();

    return 0;
}