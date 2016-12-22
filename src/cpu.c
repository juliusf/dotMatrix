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
	
	if (instructions[opcode].execute)
	{
		instructions[opcode].execute(cpu);
	}else{
		printf("Instruction 0x%x ot implemented!\n", opcode);
		exit(-1);
	}
	
	#ifdef DEBUG
    if(instructions[opcode].parLength == 2){
    	uint16_t value = get_two_byte_parameter(cpu);
    	printf( (instructions[opcode].disassembly), value);
    	printf( "\n");	
    }
     
     
    #endif /* DEBUG */

	cpu->cycles_left = instructions[opcode].cycles;

	cpu->reg_pc+= instructions[opcode].parLength + 1;
}

void opCode0x31(Cpu* cpu){
	uint16_t value = get_two_byte_parameter(cpu);
	cpu->reg_sp = value;
}

void initialize_opcodes(){
	for (int i = 0; i < 256; i++){
		instructions[i] = (Instruction){"NOT IMPLEMENTED", 0, 0, NULL};
	}

	instructions[0x31] = (Instruction){"LD SP, $%x", 2, 12, opCode0x31};
}
