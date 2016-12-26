#ifndef CPU_H
#define CPU_H

#define PROGRAMSTART 0x0
#define FLAG_BIT_Z 7
#define FLAG_BIT_N 6
#define FLAG_BIT_H 5
#define FLAG_BIT_C 4

#define PC_NO_JMP 0
#define PC_JMP 1

#include <stdint.h>
#include <stddef.h>


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
	uint8_t reg_flag;
	struct Interconnect_t* interconnect;
	uint8_t cycles_left;
} Cpu;

typedef struct Instruction_t {
	char *disassembly;
	int8_t parLength;
	uint8_t cycles;
	int8_t (*execute)(Cpu*);
} Instruction;

void initialize_cpu(Cpu** cpu, struct Interconnect_t* interconnect);

void run(Cpu* cpu);

static Instruction instructions[256];
static Instruction cb_instructions[256];

#include "cpu_inline.h"

#endif /* CPU_H */
