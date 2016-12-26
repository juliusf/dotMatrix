#include "cpu.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "util.h"
#include "interconnect.h"

void initialize_opcodes();
void run_instruction(Cpu* cpu);
void run_instruction_set(Cpu* cpu, Instruction instruction_set[256], uint8_t opcode);

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

	if (opcode == 0xcb){
		cpu->reg_pc++;
		opcode = read_from_ram(cpu->interconnect, cpu->reg_pc);		
		run_instruction_set(cpu, cb_instructions, opcode);
	}else{
		run_instruction_set(cpu, instructions, opcode);
	}
	
	
}

void run_instruction_set(Cpu* cpu, Instruction instruction_set[256], uint8_t opcode){
	
	int8_t jmp_occured = 0;
	if (instruction_set[opcode].execute)
	{
		jmp_occured = instruction_set[opcode].execute(cpu);
	}else{
		if (instruction_set == cb_instructions){
			printf("0x%x: CB prefixed instruction 0x%x ot implemented!\n", cpu->reg_pc, opcode);
		}else
		{
			printf("0x%x: Instruction 0x%x ot implemented!\n", cpu->reg_pc, opcode);
		}
		exit(-1);
	}
	
	#ifdef DEBUG
    
    switch(instruction_set[opcode].parLength){
    	case 0:{
    		printf("0x%x: %s \n", cpu->reg_pc, instruction_set[opcode].disassembly);
    	}
    	break;
    	case 1:{
    		uint8_t value = get_one_byte_parameter(cpu);
    		printf("0x%x: ", cpu->reg_pc);
	    	printf( (instruction_set[opcode].disassembly), value);
	    	printf( "\n");
    	}
    	break;
    	case 2:{
	    	uint16_t value = get_two_byte_parameter(cpu);
	    	printf("0x%x: ", cpu->reg_pc);
	    	printf( (instruction_set[opcode].disassembly), value);
	    	printf( "\n");
    	}
    	break;
    	default:
    		printf("Unsupported debug number of parameters!\n");
    		exit(-1);
    };
     
    #endif /* DEBUG */

	cpu->cycles_left = instruction_set[opcode].cycles;

	if (! jmp_occured)
		cpu->reg_pc+= instruction_set[opcode].parLength +1;

}
//--------------------------------------------------------------

int8_t opCode0x20(Cpu* cpu){ // JRNZ
	int8_t addr_offset = get_one_byte_parameter(cpu); 
	if (! get_bit( &(cpu->reg_flag), FLAG_BIT_Z)){
		cpu->reg_pc += addr_offset;
		return PC_JMP;
	}
	return PC_NO_JMP;
	
}

int8_t opCode0x21(Cpu* cpu){ // LD HL, $nn
	uint16_t value = get_two_byte_parameter(cpu);
	cpu->reg_hl = value;
	return PC_NO_JMP;
}

int8_t opCode0x31(Cpu* cpu){ // LD SP, $nn
	uint16_t value = get_two_byte_parameter(cpu);
	cpu->reg_sp = value;
	return PC_NO_JMP;
}

int8_t opCode0x32(Cpu* cpu){ // LDD (HL), A
	write_to_ram(cpu->interconnect, cpu->reg_hl, cpu->reg_a);
	cpu->reg_hl--;
	return PC_NO_JMP;
}

int8_t opCode0xaf(Cpu* cpu){// XOR A, A
	uint8_t result = cpu->reg_a ^ cpu->reg_a;
	cpu->reg_a = result;
	return PC_NO_JMP;
}

//---------------------------------------------------------------

int8_t opCode0xcb7c(Cpu* cpu){ // BIT 7h
	if ( ! get_bit(&cpu->reg_h, 7) ){
		set_bit(&cpu->reg_flag, FLAG_BIT_Z);	
	}else{
		clear_bit(&cpu->reg_flag, FLAG_BIT_Z);
	}

	clear_bit(&cpu->reg_flag, FLAG_BIT_N);
	set_bit(&cpu->reg_flag, FLAG_BIT_H);
	return PC_NO_JMP;
}

void initialize_opcodes(){
	for (int i = 0; i < 256; i++){
		instructions[i] = (Instruction){"NOT IMPLEMENTED", 0, 0, NULL};
		cb_instructions[i] = (Instruction){"NOT IMPLEMENTED", 0, 0, NULL};
	}

	instructions[0x20] = (Instruction){"JRNZ, $%x", 1, 8, opCode0x20};
	instructions[0x21] = (Instruction){"LD HL, $%x", 2, 12, opCode0x21};
	instructions[0x31] = (Instruction){"LD SP, $%x", 2, 12, opCode0x31};
	instructions[0x32] = (Instruction){"LDD HL, A", 0, 8, opCode0x32};
	instructions[0xaf] = (Instruction){"XOR A, A", 0, 4, opCode0xaf};


	cb_instructions[0x7c] = (Instruction){"BIT 7, H", 0, 8, opCode0xaf};
}
