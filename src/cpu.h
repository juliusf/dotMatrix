#ifndef CPU_H
#define CPU_H

#define PROGRAMSTART 0x0

#include <stdint.h>


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
	uint8_t cycles;
	void (*execute)(Cpu*,uint16_t);
} Instruction;

void initialize_cpu(Cpu** cpu, struct Interconnect_t* interconnect);
void op_not_implemented(Cpu* cpu, uint16_t instruction);
void run(Cpu* cpu);

static Instruction instructions[256] = { {"NOT IMPLEMENTED", 0, op_not_implemented} };

#endif /* CPU_H */
