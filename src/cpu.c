#include "cpu.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "util.h"
#include "interconnect.h"

void initialize_opcodes();
void run_instruction(Cpu* cpu);

void initialize_cpu(Cpu** cpu, Interconnect* interconnect){

	*cpu = (Cpu*) malloc(sizeof(Cpu));
	(*cpu)->reg_pc = PROGRAMSTART;
	(*cpu)->interconnect = interconnect;
	initialize_opcodes();
}

void initialize_opcodes(){
	for (int i = 0; i < 256; i++){
		instructions[i] = (Instruction){"NOT IMPLEMENTED", 0, op_not_implemented};
	}
}

void run(Cpu* cpu){
	debug_print("starting execution%s", "\n");
	while(1){
		
		if (cpu->cycles_left == 0){
			run_instruction(cpu);
			assert(cpu->cycles_left > 0);
		}
		cpu->cycles_left--;

	}
}

void run_instruction(Cpu* cpu){

	uint8_t opcode = read_from_ram(cpu->interconnect, cpu->reg_pc);
	instructions[opcode].execute(cpu, opcode);
	cpu->cycles_left = instructions[opcode].cycles;

	cpu->reg_pc++;
}

void op_not_implemented(Cpu* cpu, uint16_t instruction){
	printf("Instruction 0x%x not implemented!\n", instruction);
	exit(-1);
}


