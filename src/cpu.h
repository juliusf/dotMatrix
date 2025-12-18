#ifndef CPU_H
#define CPU_H

#define PROGRAMSTART 0x0
#define FLAG_BIT_Z 0x7
#define FLAG_BIT_N 0x6
#define FLAG_BIT_H 0x5
#define FLAG_BIT_C 0x4
#define FLAG_NONE 0x0

#define PC_NO_JMP 0
#define PC_JMP 1

#define INSTRUCTION_BUFFER_SIZE 25

#include <stdint.h>
#include <stddef.h>
#include <pthread.h>

typedef struct InstructionTrace_t {
	char text[256];
	uint64_t instruction_count;
	uint16_t pc;
} InstructionTrace;


typedef struct Cpu_t{
	struct{
		union{
			struct{
				uint8_t reg_f;
				uint8_t reg_a;
			};
			uint16_t reg_af;
		};
	};
	struct{
		union{
			struct{
				uint8_t reg_c;
				uint8_t reg_b;
			};
			uint16_t reg_bc;
		};
	};
	struct{
		union{
			struct{
				uint8_t reg_e;
				uint8_t reg_d;
			};
			uint16_t reg_de;
		};
	};
	struct{
		union{
			struct{
				uint8_t reg_l;
				uint8_t reg_h;
			};
			uint16_t reg_hl;
		};
	};
	uint16_t reg_sp;
	uint16_t reg_pc;
	struct Interconnect_t* interconnect;
	uint8_t cycles_left;
	uint64_t instruction_count;
	volatile int should_stop;
} Cpu;

typedef struct Instruction_t {
	char *disassembly;
	int8_t parLength;
	uint8_t cycles;
	int8_t (*execute)(Cpu*);
} Instruction;

void initialize_cpu(Cpu** cpu, struct Interconnect_t* interconnect);

void run(Cpu* cpu);
void* cpu_thread_run(void* arg);
pthread_t start_cpu_thread(Cpu* cpu);
void stop_cpu(Cpu* cpu);

#ifdef DEBUG
void add_instruction_to_buffer(uint64_t instruction_count, uint16_t pc, const char* text);
void print_instruction_buffer(void);
#endif

static Instruction instructions[256];
static Instruction cb_instructions[256];

#include "cpu_inline.h"

#endif /* CPU_H */
