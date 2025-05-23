

#ifndef NEANDER_H
#define NEANDER_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MEM_SIZE 256

extern uint8_t memory[MEM_SIZE];
extern uint8_t AC;   
extern uint8_t PC;   
extern uint8_t N;    
extern uint8_t Z;    

void update_flags(void);
void sta(uint8_t address);
void lda(uint8_t address);
void add(uint8_t address);
void or(uint8_t address);
void and(uint8_t address);
void not(void);
void jmp(uint8_t address);
void jmp_n(uint8_t address); 
void jz(uint8_t address);     
void run(void);
void dump_memory(void);
void load_program(uint8_t program[], uint8_t size);
void load_mem_file(const char *filename);

#endif 
