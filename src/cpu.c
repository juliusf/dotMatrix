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

#ifdef DEBUG
static InstructionTrace instruction_buffer[INSTRUCTION_BUFFER_SIZE];
static int buffer_index = 0;
static int buffer_filled = 0;

void add_instruction_to_buffer(uint64_t instruction_count, uint16_t pc, const char* text){
	instruction_buffer[buffer_index].instruction_count = instruction_count;
	instruction_buffer[buffer_index].pc = pc;
	strncpy(instruction_buffer[buffer_index].text, text, sizeof(instruction_buffer[buffer_index].text) - 1);
	instruction_buffer[buffer_index].text[sizeof(instruction_buffer[buffer_index].text) - 1] = '\0';

	buffer_index = (buffer_index + 1) % INSTRUCTION_BUFFER_SIZE;
	if (!buffer_filled && buffer_index == 0) {
		buffer_filled = 1;
	}
}

void print_instruction_buffer(void){
	int count = buffer_filled ? INSTRUCTION_BUFFER_SIZE : buffer_index;
	int start = buffer_filled ? buffer_index : 0;

	fprintf(stderr, "\n=== Last %d executed instructions ===\n", count);
	for (int i = 0; i < count; i++){
		int idx = (start + i) % INSTRUCTION_BUFFER_SIZE;
		fprintf(stderr, "%" PRIu64 "| 0x%x: %s\n",
			instruction_buffer[idx].instruction_count,
			instruction_buffer[idx].pc,
			instruction_buffer[idx].text);
	}
	fprintf(stderr, "=====================================\n");
}
#endif

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
	(*cpu)->should_stop = 0;
	initialize_opcodes();
}

void run(Cpu* cpu){
	debug_print("starting execution%s", "\n");
	while(!cpu->should_stop){

		if (cpu->cycles_left == 0){
			run_instruction(cpu);
			assert(cpu->cycles_left > 0);
		}

		// Step PPU (1 M-cycle = 4 T-cycles)
		ppu_step(cpu->interconnect->ppu, 4);

		cpu->cycles_left--;

	}
	debug_print("cpu execution stopped%s", "\n");
}

void* cpu_thread_run(void* arg){
	Cpu* cpu = (Cpu*)arg;
	run(cpu);
	return NULL;
}

pthread_t start_cpu_thread(Cpu* cpu){
	pthread_t thread;
	pthread_create(&thread, NULL, cpu_thread_run, cpu);
	return thread;
}

void stop_cpu(Cpu* cpu){
	cpu->should_stop = 1;
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
    char instruction_text[256];
    switch(instruction_set[opcode].parLength){

    	case 0:{
    		snprintf(instruction_text, sizeof(instruction_text), "%s", instruction_set[opcode].disassembly);
    	}
    	break;
    	case 1:{
    		uint8_t value = get_one_byte_parameter(cpu);
	    	snprintf(instruction_text, sizeof(instruction_text), instruction_set[opcode].disassembly, value);
    	}
    	break;
    	case 2:{
	    	uint16_t value = get_two_byte_parameter(cpu);
	    	snprintf(instruction_text, sizeof(instruction_text), instruction_set[opcode].disassembly, value);
    	}
    	break;
    	default:
    		fprintf(stderr, "Unsupported debug number of parameters!\n");
    		exit(-1);
    };

    add_instruction_to_buffer(cpu->instruction_count, cpu->reg_pc, instruction_text);

    #endif /* DEBUG */
	
	int8_t jmp_occured = 0;
	if (instruction_set[opcode].execute)
	{
		jmp_occured = instruction_set[opcode].execute(cpu);
	}else{
		#ifdef DEBUG
		print_instruction_buffer();
		#endif
		if (instruction_set == cb_instructions){
			fprintf(stderr, "0x%x: CB prefixed instruction 0x%x not implemented!\n", cpu->reg_pc, opcode);
		}else
		{
			fprintf(stderr, "0x%x: Instruction 0x%x not implemented!\n", cpu->reg_pc, opcode);
		}
		exit(-1);
	}

	// Set cycles - use default value unless execute function set it explicitly
	if (cpu->cycles_left == 0) {
		cpu->cycles_left = instruction_set[opcode].cycles;
	}

	if (! jmp_occured)
		cpu->reg_pc+= instruction_set[opcode].parLength +1;

}
//--------------------------------------------------------------



void initialize_opcodes(void){
	for (int i = 0; i < 256; i++){
		instructions[i] = (Instruction){"NOT IMPLEMENTED", 0, 0, NULL};
		cb_instructions[i] = (Instruction){"NOT IMPLEMENTED", 0, 0, NULL};
	}
	instructions[0x00] = (Instruction){"NOP", 0, 4, opCode0x00};
	instructions[0x01] = (Instruction){"LD BC, $%x", 2, 12, opCode0x01};
	instructions[0x04] = (Instruction){"INC B", 0, 4, opCode0x04};
	instructions[0x05] = (Instruction){"DEC B", 0, 4, opCode0x05};
	instructions[0x06] = (Instruction){"LD B, 0x%x", 1, 8, opCode0x06};
	instructions[0x08] = (Instruction){"LD $%x, SP", 2, 20, opCode0x08};
	instructions[0x0b] = (Instruction){"DEC BC", 0, 8, opCode0x0b};
	instructions[0x0c] = (Instruction){"INC C", 0, 4, opCode0x0c};
	instructions[0x0d] = (Instruction){"DEC C", 0, 4, opCode0x0d};
	instructions[0x0e] = (Instruction){"LD C, 0x%x", 1, 8, opCode0x0e};
	
	instructions[0x11] = (Instruction){"LD DE, $%x", 2, 12, opCode0x11};
	instructions[0x12] = (Instruction){"LD (DE), A", 0, 8, opCode0x12};
	instructions[0x13] = (Instruction){"INC DE", 0, 8, opCode0x13};
	instructions[0x15] = (Instruction){"DEC D", 0, 4, opCode0x15};
	instructions[0x16] = (Instruction){"LD D, 0x%x", 1, 8, opCode0x16};
	instructions[0x17] = (Instruction){"RLA", 0, 4, opCode0x17};
	instructions[0x18] = (Instruction){"JR 0x%x", 1, 12, opCode0x18};
	instructions[0x19] = (Instruction){"ADD HL, DE", 0, 8, opCode0x19};
	instructions[0x1a] = (Instruction){"LD A, (DE)", 0, 8, opCode0x1a};
	instructions[0x1c] = (Instruction){"INC E", 0, 4, opCode0x1c};
	instructions[0x1d] = (Instruction){"DEC E", 0, 4, opCode0x1d};
	instructions[0x1e] = (Instruction){"LD E, 0x%x", 1, 8, opCode0x1e};

	instructions[0x20] = (Instruction){"JRNZ, 0x%x", 1, 8, opCode0x20};
	instructions[0x21] = (Instruction){"LD HL, $%x", 2, 12, opCode0x21};
	instructions[0x22] = (Instruction){"LDI HL, A", 0, 8, opCode0x22};
	instructions[0x23] = (Instruction){"INC HL", 0, 8, opCode0x23};
	instructions[0x24] = (Instruction){"INC H", 0, 4, opCode0x24};
	instructions[0x25] = (Instruction){"DEC H", 0, 4, opCode0x25};
	instructions[0x28] = (Instruction){"JR Z, 0x%x", 1, 8, opCode0x28};
	instructions[0x2a] = (Instruction){"LDI A, (HL)", 0, 8, opCode0x2a};
	instructions[0x2c] = (Instruction){"INC L", 0, 4, opCode0x2c};

	instructions[0x31] = (Instruction){"LD SP, $%x", 2, 12, opCode0x31};
	instructions[0x32] = (Instruction){"LDD HL, A", 0, 8, opCode0x32};
	instructions[0x36] = (Instruction){"LD (HL), 0x%x", 1, 12, opCode0x36};
	instructions[0x3c] = (Instruction){"INC A", 0, 4, opCode0x3c};
	instructions[0x3d] = (Instruction){"DEC A", 0, 4, opCode0x3d};
	instructions[0x3e] = (Instruction){"LD A, $%x", 1, 8, opCode0x3e};

	instructions[0x47] = (Instruction){"LD B,A", 0, 4, opCode0x47};
	instructions[0x4f] = (Instruction){"LD C,A", 0, 4, opCode0x4f};

	instructions[0x56] = (Instruction){"LD D,(HL)", 0, 8, opCode0x56};
	instructions[0x57] = (Instruction){"LD D,A", 0, 4, opCode0x57};
	instructions[0x5e] = (Instruction){"LD E,(HL)", 0, 8, opCode0x5e};
	instructions[0x5f] = (Instruction){"LD E,A", 0, 4, opCode0x5f};
	instructions[0x67] = (Instruction){"LD H,A", 0, 4, opCode0x67};
	instructions[0x77] = (Instruction){"LD (HL), A", 0, 8, opCode0x77};
	instructions[0x78] = (Instruction){"LD A,B", 0, 4, opCode0x78};
	instructions[0x79] = (Instruction){"LD A,C", 0, 4, opCode0x79};
	instructions[0x7b] = (Instruction){"LD A,E", 0, 4, opCode0x7b};
	instructions[0x7c] = (Instruction){"LD A,H", 0, 4, opCode0x7c};
	instructions[0x7d] = (Instruction){"LD A,L", 0, 4, opCode0x7d};
	instructions[0x7e] = (Instruction){"LD A,(HL)", 0, 8, opCode0x7e};

	instructions[0x80] = (Instruction){"ADD A,B", 0, 4, opCode0x80};
	instructions[0x81] = (Instruction){"ADD A,C", 0, 4, opCode0x81};
	instructions[0x82] = (Instruction){"ADD A,D", 0, 4, opCode0x82};
	instructions[0x83] = (Instruction){"ADD A,E", 0, 4, opCode0x83};
	instructions[0x84] = (Instruction){"ADD A,H", 0, 4, opCode0x84};
	instructions[0x85] = (Instruction){"ADD A,L", 0, 4, opCode0x85};
	instructions[0x86] = (Instruction){"ADD A,(HL)", 0, 8, opCode0x86};
	instructions[0x87] = (Instruction){"ADD A,A", 0, 4, opCode0x87};

	instructions[0x90] = (Instruction){"SUB B", 0, 4, opCode0x90};

	instructions[0xa1] = (Instruction){"AND C", 0, 4, opCode0xa1};
	instructions[0xa7] = (Instruction){"AND A", 0, 4, opCode0xa7};
	instructions[0xa9] = (Instruction){"XOR C", 0, 4, opCode0xa9};
	instructions[0xaf] = (Instruction){"XOR A, A", 0, 4, opCode0xaf};
	instructions[0xb0] = (Instruction){"OR B", 0, 4, opCode0xb0};
	instructions[0xb1] = (Instruction){"OR C", 0, 4, opCode0xb1};

	instructions[0xbe] = (Instruction){"CP (HL)", 0, 8, opCode0xbe};

	instructions[0xc1] = (Instruction){"POP BC", 0, 12, opCode0xc1};
	instructions[0xc3] = (Instruction){"JP $%x", 2, 16, opCode0xc3};
	instructions[0xc5] = (Instruction){"PUSH BC", 0, 16, opCode0xc5};
	instructions[0xca] = (Instruction){"JP Z, $%x", 2, 12, opCode0xca};
	instructions[0xc6] = (Instruction){"ADD A,0x%x", 1, 8, opCode0xc6};
	instructions[0xc8] = (Instruction){"RET Z", 0, 8, opCode0xc8};
	instructions[0xc9] = (Instruction){"RET", 0, 8, opCode0xc9};
	instructions[0xcd] = (Instruction){"CALL $%x", 2, 12, opCode0xcd};

	instructions[0xd1] = (Instruction){"POP DE", 0, 12, opCode0xd1};
	instructions[0xd5] = (Instruction){"PUSH DE", 0, 16, opCode0xd5};

	instructions[0xe1] = (Instruction){"POP HL", 0, 12, opCode0xe1};
	instructions[0xe0] = (Instruction){"LDH 0x%x, A", 1, 12, opCode0xe0};
	instructions[0xe2] = (Instruction){"LD (C),A", 0, 8, opCode0xe2};
	instructions[0xe5] = (Instruction){"PUSH HL", 0, 16, opCode0xe5};
	instructions[0xe6] = (Instruction){"AND 0x%x", 1, 8, opCode0xe6};
	instructions[0xe9] = (Instruction){"JP (HL)", 0, 4, opCode0xe9};
	instructions[0xea] = (Instruction){"LD $%x, A", 2, 16, opCode0xea};
	instructions[0xef] = (Instruction){"RST 28", 0, 16, opCode0xef};

	instructions[0xf0] = (Instruction){"LDH A, 0x%x", 1, 12, opCode0xf0};
	instructions[0xf1] = (Instruction){"POP AF", 0, 12, opCode0xf1};
	instructions[0xf3] = (Instruction){"DI", 0, 4, opCode0xf3};
	instructions[0xf5] = (Instruction){"PUSH AF", 0, 16, opCode0xf5};
	instructions[0xfa] = (Instruction){"LD A, $%x", 2, 16, opCode0xfa};
	instructions[0xfb] = (Instruction){"EI", 0, 4, opCode0xfb};

	instructions[0x2f] = (Instruction){"CPL", 0, 4, opCode0x2f};
	instructions[0xff] = (Instruction){"RST 38", 0, 16, opCode0xff};
	instructions[0xfe] = (Instruction){"CP 0x%x", 1, 8, opCode0xfe};

	//--------------------------------------------------------------------

	cb_instructions[0x11] = (Instruction){"RL C", 0, 8, opCode0xcb11};
	cb_instructions[0x37] = (Instruction){"SWAP A", 0, 8, opCode0xcb37};
	cb_instructions[0x7c] = (Instruction){"BIT 7, H", 0, 8, opCode0xcb7c};
	cb_instructions[0x87] = (Instruction){"RES 0, A", 0, 8, opCode0xcb87};

}
