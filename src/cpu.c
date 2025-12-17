#include "cpu.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>

#include "util.h"
#include "interconnect.h"
#include "cpu_opcodes.h"

void initialize_opcodes(void);
void run_instruction(Cpu* cpu);
void run_instruction_set(Cpu* cpu, Instruction instruction_set[256], uint8_t opcode);

void initialize_cpu(Cpu** cpu, Interconnect* interconnect){

	*cpu = (Cpu*) malloc(sizeof(Cpu));
	memset((*cpu), 0, sizeof(Cpu));
	(*cpu)->reg_pc = PROGRAMSTART;
	(*cpu)->reg_af = 0x01b0;
	(*cpu)->reg_bc = 0x0013;
	(*cpu)->reg_de = 0x00D8;
	(*cpu)->reg_hl = 0x014d;
	(*cpu)->instruction_count = 0;
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

	cpu->instruction_count++;
	
}

void run_instruction_set(Cpu* cpu, Instruction instruction_set[256], uint8_t opcode){
	

	
	#ifdef DEBUG
    printf("%" PRIu64 "| ", cpu->instruction_count);
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
	
	int8_t jmp_occured = 0;
	if (instruction_set[opcode].execute)
	{
		jmp_occured = instruction_set[opcode].execute(cpu);
	}else{
		if (instruction_set == cb_instructions){
			printf("0x%x: CB prefixed instruction 0x%x not implemented!\n", cpu->reg_pc, opcode);
		}else
		{
			printf("0x%x: Instruction 0x%x not implemented!\n", cpu->reg_pc, opcode);
		}
		exit(-1);
	}
	
	cpu->cycles_left = instruction_set[opcode].cycles;

	if (! jmp_occured)
		cpu->reg_pc+= instruction_set[opcode].parLength +1;

}
//--------------------------------------------------------------



void initialize_opcodes(void){
	for (int i = 0; i < 256; i++){
		instructions[i] = (Instruction){"NOT IMPLEMENTED", 0, 0, NULL};
		cb_instructions[i] = (Instruction){"NOT IMPLEMENTED", 0, 0, NULL};
	}
	instructions[0x05] = (Instruction){"DEC B", 0, 4, opCode0x05};
	instructions[0x06] = (Instruction){"LD B, 0x%x", 1, 8, opCode0x06};
	instructions[0x08] = (Instruction){"LD $%x, SP", 2, 20, opCode0x08};
	instructions[0x0c] = (Instruction){"INC C", 0, 4, opCode0x0c};
	instructions[0x0e] = (Instruction){"LD C, 0x%x", 1, 8, opCode0x0e};
	
	instructions[0x11] = (Instruction){"LD DE, $%x", 2, 12, opCode0x11};
	instructions[0x13] = (Instruction){"INC DE", 0, 8, opCode0x13};
	instructions[0x17] = (Instruction){"RLA", 0, 4, opCode0x17};
	instructions[0x1a] = (Instruction){"LD A, (DE)", 0, 8, opCode0x1a};

	instructions[0x20] = (Instruction){"JRNZ, 0x%x", 1, 8, opCode0x20};
	instructions[0x21] = (Instruction){"LD HL, $%x", 2, 12, opCode0x21};
	instructions[0x22] = (Instruction){"LDI HL, A", 0, 8, opCode0x22};
	instructions[0x23] = (Instruction){"INC HL", 0, 8, opCode0x23};
	instructions[0x28] = (Instruction){"JR Z, 0x%x", 1, 8, opCode0x28};

	instructions[0x31] = (Instruction){"LD SP, $%x", 2, 12, opCode0x31};
	instructions[0x32] = (Instruction){"LDD HL, A", 0, 8, opCode0x32};
	instructions[0x3d] = (Instruction){"DEC A", 0, 4, opCode0x3d};
	instructions[0x3e] = (Instruction){"LD A, $%x", 1, 8, opCode0x3e};

	instructions[0x4f] = (Instruction){"LD C,A", 0, 4, opCode0x4f};

	instructions[0x77] = (Instruction){"LD (HL), A", 0, 8, opCode0x77};
	instructions[0x78] = (Instruction){"LD A,B", 0, 4, opCode0x78};
	instructions[0x7b] = (Instruction){"LD A,E", 0, 4, opCode0x7b};
	instructions[0x7d] = (Instruction){"LD A,L", 0, 4, opCode0x7d};

	instructions[0x80] = (Instruction){"ADD A,B", 0, 4, opCode0x80};
	instructions[0x81] = (Instruction){"ADD A,C", 0, 4, opCode0x81};
	instructions[0x82] = (Instruction){"ADD A,D", 0, 4, opCode0x82};
	instructions[0x83] = (Instruction){"ADD A,E", 0, 4, opCode0x83};
	instructions[0x84] = (Instruction){"ADD A,H", 0, 4, opCode0x84};
	instructions[0x85] = (Instruction){"ADD A,L", 0, 4, opCode0x85};
	instructions[0x86] = (Instruction){"ADD A,(HL)", 0, 8, opCode0x86};
	instructions[0x87] = (Instruction){"ADD A,A", 0, 4, opCode0x87};

	instructions[0xaf] = (Instruction){"XOR A, A", 0, 4, opCode0xaf};

	instructions[0xbe] = (Instruction){"CP (HL)", 0, 8, opCode0xbe};

	instructions[0xc1] = (Instruction){"POP BC", 0, 12, opCode0xc1};
	instructions[0xc5] = (Instruction){"PUSH BC", 0, 16, opCode0xc5};
	instructions[0xc6] = (Instruction){"ADD A,0x%x", 1, 8, opCode0xc6};
	instructions[0xc9] = (Instruction){"RET", 0, 8, opCode0xc9};
	instructions[0xcd] = (Instruction){"CALL $%x", 2, 12, opCode0xcd};

	instructions[0xe0] = (Instruction){"LDH 0x%x, A", 1, 12, opCode0xe0};
	instructions[0xe2] = (Instruction){"LD (C),A", 0, 8, opCode0xe2};
	instructions[0xe5] = (Instruction){"PUSH HL", 0, 16, opCode0xe5};
	instructions[0xea] = (Instruction){"LD $%x, A", 2, 16, opCode0xea};

	instructions[0x2f] = (Instruction){"CPL", 0, 4, opCode0x2f};
	instructions[0xff] = (Instruction){"RST 38", 0, 32, opCode0xff};
	instructions[0xfe] = (Instruction){"CP SP", 1, 8, opCode0xfe};

	//--------------------------------------------------------------------

	cb_instructions[0x11] = (Instruction){"RL C", 0, 8, opCode0xcb11};
	cb_instructions[0x7c] = (Instruction){"BIT 7, H", 0, 8, opCode0xcb7c};

}
