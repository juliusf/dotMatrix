#include "cpu.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

#include "util.h"
#include "interconnect.h"
#include "cpu_opcodes.h"

// Game Boy DMG CPU frequency: 4.194304 MHz (2^22 Hz)
// 1 M-cycle = 4 T-cycles, so M-cycle frequency = 1.048576 MHz
// Frame timing: 70224 T-cycles = 17556 M-cycles per frame
#define CYCLES_PER_FRAME 17556  // M-cycles per frame
#define NANOSECONDS_PER_FRAME 16742706L  // ~16.742 ms per frame (~59.7 Hz)

void initialize_opcodes(void);
void run_instruction(Cpu* cpu);
void run_instruction_set(Cpu* cpu, Instruction instruction_set[256], uint8_t opcode);
void handle_interrupts(Cpu* cpu);

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
	(*cpu)->ime = 0;  // Interrupts disabled at startup
	(*cpu)->ime_scheduled = 0;
	(*cpu)->halted = 0;
	(*cpu)->in_interrupt = 0;
	initialize_opcodes();
}

// Handle interrupts - checks for pending interrupts and dispatches them
void handle_interrupts(Cpu* cpu) {
	// Check if PPU requested a V-Blank interrupt
	if (cpu->interconnect->ppu->vblank_interrupt_requested) {
		cpu->interconnect->ppu->vblank_interrupt_requested = 0;
		cpu->interconnect->interrupt_flag |= INT_VBLANK;
	}

	// Check which interrupts are both requested (IF) and enabled (IE)
	uint8_t pending = cpu->interconnect->interrupt_flag & cpu->interconnect->interrupt_enable & 0x1F;

	// If CPU is halted and there's ANY interrupt (IF & IE), wake it up
	// (HALT wakes up even if IME is disabled)
	if (cpu->halted && pending) {
		cpu->halted = 0;
	}

	// Only handle interrupts if IME is enabled
	if (!cpu->ime) {
		return;
	}

	if (pending == 0) {
		return;  // No interrupts to handle
	}

	// Disable IME (interrupts are disabled during interrupt handling)
	cpu->ime = 0;

	// Find highest priority interrupt (lowest bit number)
	uint8_t interrupt_bit = 0;
	uint16_t interrupt_vector = 0;

	if (pending & INT_VBLANK) {
		interrupt_bit = INT_VBLANK;
		interrupt_vector = 0x0040;  // V-Blank interrupt vector
	} else if (pending & INT_LCD) {
		interrupt_bit = INT_LCD;
		interrupt_vector = 0x0048;  // LCD STAT interrupt vector
	} else if (pending & INT_TIMER) {
		interrupt_bit = INT_TIMER;
		interrupt_vector = 0x0050;  // Timer interrupt vector
	} else if (pending & INT_SERIAL) {
		interrupt_bit = INT_SERIAL;
		interrupt_vector = 0x0058;  // Serial interrupt vector
	} else if (pending & INT_JOYPAD) {
		interrupt_bit = INT_JOYPAD;
		interrupt_vector = 0x0060;  // Joypad interrupt vector
	}

	// Clear the interrupt flag for this interrupt
	cpu->interconnect->interrupt_flag &= ~interrupt_bit;

	// Push current PC onto stack
	cpu->reg_sp -= 2;
	write_addr_to_ram(cpu->interconnect, cpu->reg_sp, cpu->reg_pc);

	// Jump to interrupt vector
	cpu->reg_pc = interrupt_vector;

	// Mark that we're in an interrupt handler for timing adjustments
	cpu->in_interrupt = 1;

	// No dispatch overhead for now (testing flag logic)
	cpu->cycles_left = 0;
}

// Sleep until target time is reached, using nanosleep for efficiency
static void sleep_until(struct timespec target) {
	struct timespec current, remaining;
	clock_gettime(CLOCK_MONOTONIC, &current);

	// Calculate time remaining
	remaining.tv_sec = target.tv_sec - current.tv_sec;
	remaining.tv_nsec = target.tv_nsec - current.tv_nsec;

	// Handle nanosecond underflow
	if (remaining.tv_nsec < 0) {
		remaining.tv_sec--;
		remaining.tv_nsec += 1000000000L;
	}

	// If we're already past the target time, don't sleep
	if (remaining.tv_sec < 0 || (remaining.tv_sec == 0 && remaining.tv_nsec <= 0)) {
		return;
	}

	// Sleep for most of the remaining time, but wake up a bit early
	// to avoid oversleeping (nanosleep can sleep longer than requested)
	if (remaining.tv_nsec > 1000000) {  // If more than 1ms remaining
		remaining.tv_nsec -= 500000;  // Wake up 0.5ms early
		nanosleep(&remaining, NULL);
	}

	// Busy-wait for the last bit to be precise
	do {
		clock_gettime(CLOCK_MONOTONIC, &current);
	} while (current.tv_sec < target.tv_sec ||
	         (current.tv_sec == target.tv_sec && current.tv_nsec < target.tv_nsec));
}

void run(Cpu* cpu){
	debug_print("starting execution%s", "\n");

	// Frame timing tracking
	struct timespec frame_start, next_frame_time;
	clock_gettime(CLOCK_MONOTONIC, &frame_start);
	next_frame_time = frame_start;

	uint32_t cycles_this_frame = 0;

	while(!cpu->should_stop){

		uint8_t instruction_cycles;

		if (cpu->halted) {
			// CPU is halted, don't execute instructions but still step hardware
			// HALT takes 4 T-cycles (1 M-cycle) per iteration
			instruction_cycles = 1;
			cpu->cycles_left = 1;
		} else {
			// Execute next instruction (this sets cycles_left)
			run_instruction(cpu);
			assert(cpu->cycles_left > 0);
			instruction_cycles = cpu->cycles_left;
		}

		// Handle delayed IME enable (EI instruction enables interrupts AFTER next instruction)
		if (cpu->ime_scheduled) {
			cpu->ime = 1;
			cpu->ime_scheduled = 0;
		}

		// Record the cycles
		cycles_this_frame += instruction_cycles;

		// Execute all M-cycles for this instruction/HALT
		while(cpu->cycles_left > 0) {
			// Step PPU (1 M-cycle = 4 T-cycles)
			ppu_step(cpu->interconnect->ppu, 4);
			// Step Timer (1 M-cycle = 4 T-cycles)
			timer_step(cpu->interconnect, 4);
			cpu->cycles_left--;
		}

		// Check for and handle interrupts
		handle_interrupts(cpu);

		// Check if we've completed a frame worth of cycles
		if (cycles_this_frame >= CYCLES_PER_FRAME) {
			cycles_this_frame -= CYCLES_PER_FRAME;

			// Calculate next frame time
			next_frame_time.tv_nsec += NANOSECONDS_PER_FRAME;
			if (next_frame_time.tv_nsec >= 1000000000L) {
				next_frame_time.tv_sec++;
				next_frame_time.tv_nsec -= 1000000000L;
			}

			// Sleep until next frame time
			sleep_until(next_frame_time);
		}
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
	// Instruction table: cycles are in M-cycles (1 M-cycle = 4 T-cycles)
	instructions[0x00] = (Instruction){"NOP", 0, 1, opCode0x00};
	instructions[0x01] = (Instruction){"LD BC, $%x", 2, 3, opCode0x01};
	instructions[0x02] = (Instruction){"LD (BC), A", 0, 2, opCode0x02};
	instructions[0x03] = (Instruction){"INC BC", 0, 2, opCode0x03};
	instructions[0x04] = (Instruction){"INC B", 0, 1, opCode0x04};
	instructions[0x05] = (Instruction){"DEC B", 0, 1, opCode0x05};
	instructions[0x06] = (Instruction){"LD B, 0x%x", 1, 2, opCode0x06};
	instructions[0x07] = (Instruction){"RLCA", 0, 1, opCode0x07};
	instructions[0x08] = (Instruction){"LD $%x, SP", 2, 5, opCode0x08};
	instructions[0x09] = (Instruction){"ADD HL, BC", 0, 2, opCode0x09};
	instructions[0x0a] = (Instruction){"LD A,(BC)", 0, 2, opCode0x0a};
	instructions[0x0b] = (Instruction){"DEC BC", 0, 2, opCode0x0b};
	instructions[0x0c] = (Instruction){"INC C", 0, 1, opCode0x0c};
	instructions[0x0d] = (Instruction){"DEC C", 0, 1, opCode0x0d};
	instructions[0x0e] = (Instruction){"LD C, 0x%x", 1, 2, opCode0x0e};
	instructions[0x0f] = (Instruction){"RRCA", 0, 1, opCode0x0f};
	instructions[0x10] = (Instruction){"STOP", 0, 1, opCode0x10};

	instructions[0x11] = (Instruction){"LD DE, $%x", 2, 3, opCode0x11};
	instructions[0x12] = (Instruction){"LD (DE), A", 0, 2, opCode0x12};
	instructions[0x13] = (Instruction){"INC DE", 0, 2, opCode0x13};
	instructions[0x14] = (Instruction){"INC D", 0, 1, opCode0x14};
	instructions[0x15] = (Instruction){"DEC D", 0, 1, opCode0x15};
	instructions[0x16] = (Instruction){"LD D, 0x%x", 1, 2, opCode0x16};
	instructions[0x17] = (Instruction){"RLA", 0, 1, opCode0x17};
	instructions[0x18] = (Instruction){"JR 0x%x", 1, 3, opCode0x18};
	instructions[0x19] = (Instruction){"ADD HL, DE", 0, 2, opCode0x19};
	instructions[0x1a] = (Instruction){"LD A, (DE)", 0, 2, opCode0x1a};
	instructions[0x1b] = (Instruction){"DEC DE", 0, 2, opCode0x1b};
	instructions[0x1f] = (Instruction){"RRA", 0, 1, opCode0x1f};
	instructions[0x1c] = (Instruction){"INC E", 0, 1, opCode0x1c};
	instructions[0x1d] = (Instruction){"DEC E", 0, 1, opCode0x1d};
	instructions[0x1e] = (Instruction){"LD E, 0x%x", 1, 2, opCode0x1e};

	instructions[0x20] = (Instruction){"JRNZ, 0x%x", 1, 2, opCode0x20};
	instructions[0x21] = (Instruction){"LD HL, $%x", 2, 3, opCode0x21};
	instructions[0x22] = (Instruction){"LDI HL, A", 0, 2, opCode0x22};
	instructions[0x23] = (Instruction){"INC HL", 0, 2, opCode0x23};
	instructions[0x24] = (Instruction){"INC H", 0, 1, opCode0x24};
	instructions[0x25] = (Instruction){"DEC H", 0, 1, opCode0x25};
	instructions[0x26] = (Instruction){"LD H, 0x%x", 1, 2, opCode0x26};
	instructions[0x27] = (Instruction){"DAA", 0, 1, opCode0x27};
	instructions[0x28] = (Instruction){"JR Z, 0x%x", 1, 2, opCode0x28};
	instructions[0x29] = (Instruction){"ADD HL, HL", 0, 2, opCode0x29};
	instructions[0x2a] = (Instruction){"LDI A, (HL)", 0, 2, opCode0x2a};
	instructions[0x2b] = (Instruction){"DEC HL", 0, 2, opCode0x2b};
	instructions[0x2c] = (Instruction){"INC L", 0, 1, opCode0x2c};
	instructions[0x2d] = (Instruction){"DEC L", 0, 1, opCode0x2d};
	instructions[0x2e] = (Instruction){"LD L, 0x%x", 1, 2, opCode0x2e};
	instructions[0x2f] = (Instruction){"CPL", 0, 1, opCode0x2f};
	instructions[0x30] = (Instruction){"JR NC, 0x%x", 1, 2, opCode0x30};

	instructions[0x31] = (Instruction){"LD SP, $%x", 2, 3, opCode0x31};
	instructions[0x32] = (Instruction){"LDD HL, A", 0, 2, opCode0x32};
	instructions[0x33] = (Instruction){"INC SP", 0, 2, opCode0x33};
	instructions[0x34] = (Instruction){"INC (HL)", 0, 3, opCode0x34};
	instructions[0x35] = (Instruction){"DEC (HL)", 0, 3, opCode0x35};
	instructions[0x36] = (Instruction){"LD (HL), 0x%x", 1, 3, opCode0x36};
	instructions[0x37] = (Instruction){"SCF", 0, 1, opCode0x37};
	instructions[0x38] = (Instruction){"JR C, 0x%x", 1, 2, opCode0x38};
	instructions[0x39] = (Instruction){"ADD HL, SP", 0, 2, opCode0x39};
	instructions[0x3a] = (Instruction){"LDD A,(HL)", 0, 2, opCode0x3a};
	instructions[0x3b] = (Instruction){"DEC SP", 0, 2, opCode0x3b};
	instructions[0x3c] = (Instruction){"INC A", 0, 1, opCode0x3c};
	instructions[0x3d] = (Instruction){"DEC A", 0, 1, opCode0x3d};
	instructions[0x3e] = (Instruction){"LD A, $%x", 1, 2, opCode0x3e};
	instructions[0x3f] = (Instruction){"CCF", 0, 1, opCode0x3f};

	instructions[0x40] = (Instruction){"LD B,B", 0, 1, opCode0x40};
	instructions[0x41] = (Instruction){"LD B,C", 0, 1, opCode0x41};
	instructions[0x42] = (Instruction){"LD B,D", 0, 1, opCode0x42};
	instructions[0x43] = (Instruction){"LD B,E", 0, 1, opCode0x43};
	instructions[0x44] = (Instruction){"LD B,H", 0, 1, opCode0x44};
	instructions[0x45] = (Instruction){"LD B,L", 0, 1, opCode0x45};
	instructions[0x46] = (Instruction){"LD B,(HL)", 0, 2, opCode0x46};
	instructions[0x47] = (Instruction){"LD B,A", 0, 1, opCode0x47};
	instructions[0x48] = (Instruction){"LD C,B", 0, 1, opCode0x48};
	instructions[0x49] = (Instruction){"LD C,C", 0, 1, opCode0x49};
	instructions[0x4a] = (Instruction){"LD C,D", 0, 1, opCode0x4a};
	instructions[0x4b] = (Instruction){"LD C,E", 0, 1, opCode0x4b};
	instructions[0x4c] = (Instruction){"LD C,H", 0, 1, opCode0x4c};
	instructions[0x4d] = (Instruction){"LD C,L", 0, 1, opCode0x4d};
	instructions[0x4e] = (Instruction){"LD C,(HL)", 0, 2, opCode0x4e};
	instructions[0x4f] = (Instruction){"LD C,A", 0, 1, opCode0x4f};

	instructions[0x50] = (Instruction){"LD D,B", 0, 1, opCode0x50};
	instructions[0x51] = (Instruction){"LD D,C", 0, 1, opCode0x51};
	instructions[0x52] = (Instruction){"LD D,D", 0, 1, opCode0x52};
	instructions[0x53] = (Instruction){"LD D,E", 0, 1, opCode0x53};
	instructions[0x54] = (Instruction){"LD D,H", 0, 1, opCode0x54};
	instructions[0x55] = (Instruction){"LD D,L", 0, 1, opCode0x55};
	instructions[0x56] = (Instruction){"LD D,(HL)", 0, 2, opCode0x56};
	instructions[0x57] = (Instruction){"LD D,A", 0, 1, opCode0x57};
	instructions[0x58] = (Instruction){"LD E,B", 0, 1, opCode0x58};
	instructions[0x59] = (Instruction){"LD E,C", 0, 1, opCode0x59};
	instructions[0x5a] = (Instruction){"LD E,D", 0, 1, opCode0x5a};
	instructions[0x5b] = (Instruction){"LD E,E", 0, 1, opCode0x5b};
	instructions[0x5c] = (Instruction){"LD E,H", 0, 1, opCode0x5c};
	instructions[0x5d] = (Instruction){"LD E,L", 0, 1, opCode0x5d};
	instructions[0x5e] = (Instruction){"LD E,(HL)", 0, 2, opCode0x5e};
	instructions[0x5f] = (Instruction){"LD E,A", 0, 1, opCode0x5f};
	instructions[0x60] = (Instruction){"LD H,B", 0, 1, opCode0x60};
	instructions[0x61] = (Instruction){"LD H,C", 0, 1, opCode0x61};
	instructions[0x62] = (Instruction){"LD H,D", 0, 1, opCode0x62};
	instructions[0x63] = (Instruction){"LD H,E", 0, 1, opCode0x63};
	instructions[0x64] = (Instruction){"LD H,H", 0, 1, opCode0x64};
	instructions[0x65] = (Instruction){"LD H,L", 0, 1, opCode0x65};
	instructions[0x66] = (Instruction){"LD H,(HL)", 0, 2, opCode0x66};
	instructions[0x67] = (Instruction){"LD H,A", 0, 1, opCode0x67};
	instructions[0x68] = (Instruction){"LD L,B", 0, 1, opCode0x68};
	instructions[0x69] = (Instruction){"LD L,C", 0, 1, opCode0x69};
	instructions[0x6a] = (Instruction){"LD L,D", 0, 1, opCode0x6a};
	instructions[0x6b] = (Instruction){"LD L,E", 0, 1, opCode0x6b};
	instructions[0x6c] = (Instruction){"LD L,H", 0, 1, opCode0x6c};
	instructions[0x6d] = (Instruction){"LD L,L", 0, 1, opCode0x6d};
	instructions[0x6e] = (Instruction){"LD L,(HL)", 0, 2, opCode0x6e};
	instructions[0x6f] = (Instruction){"LD L,A", 0, 1, opCode0x6f};
	instructions[0x70] = (Instruction){"LD (HL),B", 0, 2, opCode0x70};
	instructions[0x71] = (Instruction){"LD (HL),C", 0, 2, opCode0x71};
	instructions[0x72] = (Instruction){"LD (HL),D", 0, 2, opCode0x72};
	instructions[0x73] = (Instruction){"LD (HL),E", 0, 2, opCode0x73};
	instructions[0x74] = (Instruction){"LD (HL),H", 0, 2, opCode0x74};
	instructions[0x75] = (Instruction){"LD (HL),L", 0, 2, opCode0x75};
	instructions[0x76] = (Instruction){"HALT", 0, 1, opCode0x76};
	instructions[0x77] = (Instruction){"LD (HL), A", 0, 2, opCode0x77};
	instructions[0x78] = (Instruction){"LD A,B", 0, 1, opCode0x78};
	instructions[0x79] = (Instruction){"LD A,C", 0, 1, opCode0x79};
	instructions[0x7a] = (Instruction){"LD A,D", 0, 1, opCode0x7a};
	instructions[0x7b] = (Instruction){"LD A,E", 0, 1, opCode0x7b};
	instructions[0x7c] = (Instruction){"LD A,H", 0, 1, opCode0x7c};
	instructions[0x7d] = (Instruction){"LD A,L", 0, 1, opCode0x7d};
	instructions[0x7e] = (Instruction){"LD A,(HL)", 0, 2, opCode0x7e};
	instructions[0x7f] = (Instruction){"LD A,A", 0, 1, opCode0x7f};

	instructions[0x80] = (Instruction){"ADD A,B", 0, 1, opCode0x80};
	instructions[0x81] = (Instruction){"ADD A,C", 0, 1, opCode0x81};
	instructions[0x82] = (Instruction){"ADD A,D", 0, 1, opCode0x82};
	instructions[0x83] = (Instruction){"ADD A,E", 0, 1, opCode0x83};
	instructions[0x84] = (Instruction){"ADD A,H", 0, 1, opCode0x84};
	instructions[0x85] = (Instruction){"ADD A,L", 0, 1, opCode0x85};
	instructions[0x86] = (Instruction){"ADD A,(HL)", 0, 2, opCode0x86};
	instructions[0x87] = (Instruction){"ADD A,A", 0, 1, opCode0x87};
	instructions[0x88] = (Instruction){"ADC A,B", 0, 1, opCode0x88};
	instructions[0x89] = (Instruction){"ADC A,C", 0, 1, opCode0x89};
	instructions[0x8a] = (Instruction){"ADC A,D", 0, 1, opCode0x8a};
	instructions[0x8b] = (Instruction){"ADC A,E", 0, 1, opCode0x8b};
	instructions[0x8c] = (Instruction){"ADC A,H", 0, 1, opCode0x8c};
	instructions[0x8d] = (Instruction){"ADC A,L", 0, 1, opCode0x8d};
	instructions[0x8e] = (Instruction){"ADC A,(HL)", 0, 2, opCode0x8e};
	instructions[0x8f] = (Instruction){"ADC A,A", 0, 1, opCode0x8f};
	instructions[0x90] = (Instruction){"SUB B", 0, 1, opCode0x90};
	instructions[0x91] = (Instruction){"SUB C", 0, 1, opCode0x91};
	instructions[0x92] = (Instruction){"SUB D", 0, 1, opCode0x92};
	instructions[0x93] = (Instruction){"SUB E", 0, 1, opCode0x93};
	instructions[0x94] = (Instruction){"SUB H", 0, 1, opCode0x94};
	instructions[0x95] = (Instruction){"SUB L", 0, 1, opCode0x95};
	instructions[0x96] = (Instruction){"SUB (HL)", 0, 2, opCode0x96};
	instructions[0x97] = (Instruction){"SUB A", 0, 1, opCode0x97};
	instructions[0x98] = (Instruction){"SBC A,B", 0, 1, opCode0x98};
	instructions[0x99] = (Instruction){"SBC A,C", 0, 1, opCode0x99};
	instructions[0x9a] = (Instruction){"SBC A,D", 0, 1, opCode0x9a};
	instructions[0x9b] = (Instruction){"SBC A,E", 0, 1, opCode0x9b};
	instructions[0x9c] = (Instruction){"SBC A,H", 0, 1, opCode0x9c};
	instructions[0x9d] = (Instruction){"SBC A,L", 0, 1, opCode0x9d};
	instructions[0x9e] = (Instruction){"SBC A,(HL)", 0, 2, opCode0x9e};
	instructions[0x9f] = (Instruction){"SBC A,A", 0, 1, opCode0x9f};
	instructions[0xa0] = (Instruction){"AND B", 0, 1, opCode0xa0};
	instructions[0xa1] = (Instruction){"AND C", 0, 1, opCode0xa1};
	instructions[0xa2] = (Instruction){"AND D", 0, 1, opCode0xa2};
	instructions[0xa3] = (Instruction){"AND E", 0, 1, opCode0xa3};
	instructions[0xa4] = (Instruction){"AND H", 0, 1, opCode0xa4};
	instructions[0xa5] = (Instruction){"AND L", 0, 1, opCode0xa5};
	instructions[0xa6] = (Instruction){"AND (HL)", 0, 2, opCode0xa6};
	instructions[0xa7] = (Instruction){"AND A", 0, 1, opCode0xa7};
	instructions[0xa8] = (Instruction){"XOR B", 0, 1, opCode0xa8};
	instructions[0xa9] = (Instruction){"XOR C", 0, 1, opCode0xa9};
	instructions[0xaa] = (Instruction){"XOR D", 0, 1, opCode0xaa};
	instructions[0xab] = (Instruction){"XOR E", 0, 1, opCode0xab};
	instructions[0xac] = (Instruction){"XOR H", 0, 1, opCode0xac};
	instructions[0xad] = (Instruction){"XOR L", 0, 1, opCode0xad};
	instructions[0xae] = (Instruction){"XOR (HL)", 0, 2, opCode0xae};
	instructions[0xaf] = (Instruction){"XOR A, A", 0, 1, opCode0xaf};
	instructions[0xb0] = (Instruction){"OR B", 0, 1, opCode0xb0};
	instructions[0xb1] = (Instruction){"OR C", 0, 1, opCode0xb1};
	instructions[0xb2] = (Instruction){"OR D", 0, 1, opCode0xb2};
	instructions[0xb3] = (Instruction){"OR E", 0, 1, opCode0xb3};
	instructions[0xb4] = (Instruction){"OR H", 0, 1, opCode0xb4};
	instructions[0xb5] = (Instruction){"OR L", 0, 1, opCode0xb5};
	instructions[0xb6] = (Instruction){"OR (HL)", 0, 2, opCode0xb6};
	instructions[0xb7] = (Instruction){"OR A", 0, 1, opCode0xb7};
	instructions[0xb8] = (Instruction){"CP B", 0, 1, opCode0xb8};
	instructions[0xb9] = (Instruction){"CP C", 0, 1, opCode0xb9};
	instructions[0xba] = (Instruction){"CP D", 0, 1, opCode0xba};
	instructions[0xbb] = (Instruction){"CP E", 0, 1, opCode0xbb};
	instructions[0xbc] = (Instruction){"CP H", 0, 1, opCode0xbc};
	instructions[0xbd] = (Instruction){"CP L", 0, 1, opCode0xbd};
	instructions[0xbe] = (Instruction){"CP (HL)", 0, 2, opCode0xbe};
	instructions[0xbf] = (Instruction){"CP A", 0, 1, opCode0xbf};

	instructions[0xc0] = (Instruction){"RET NZ", 0, 2, opCode0xc0};
	instructions[0xc1] = (Instruction){"POP BC", 0, 3, opCode0xc1};
	instructions[0xc2] = (Instruction){"JP NZ, $%x", 2, 3, opCode0xc2};
	instructions[0xc3] = (Instruction){"JP $%x", 2, 4, opCode0xc3};
	instructions[0xc4] = (Instruction){"CALL NZ, $%x", 2, 3, opCode0xc4};
	instructions[0xc5] = (Instruction){"PUSH BC", 0, 4, opCode0xc5};
	instructions[0xc6] = (Instruction){"ADD A,0x%x", 1, 2, opCode0xc6};
	instructions[0xc7] = (Instruction){"RST 00", 0, 4, opCode0xc7};
	instructions[0xc8] = (Instruction){"RET Z", 0, 2, opCode0xc8};
	instructions[0xc9] = (Instruction){"RET", 0, 4, opCode0xc9};
	instructions[0xca] = (Instruction){"JP Z, $%x", 2, 3, opCode0xca};
	instructions[0xcc] = (Instruction){"CALL Z, $%x", 2, 3, opCode0xcc};
	instructions[0xcd] = (Instruction){"CALL $%x", 2, 6, opCode0xcd};
	instructions[0xce] = (Instruction){"ADC A,0x%x", 1, 2, opCode0xce};
	instructions[0xcf] = (Instruction){"RST 08", 0, 4, opCode0xcf};
	instructions[0xd0] = (Instruction){"RET NC", 0, 2, opCode0xd0};
	instructions[0xd1] = (Instruction){"POP DE", 0, 3, opCode0xd1};
	instructions[0xd2] = (Instruction){"JP NC, $%x", 2, 3, opCode0xd2};
	instructions[0xd4] = (Instruction){"CALL NC, $%x", 2, 3, opCode0xd4};
	instructions[0xd5] = (Instruction){"PUSH DE", 0, 4, opCode0xd5};
	instructions[0xd6] = (Instruction){"SUB 0x%x", 1, 2, opCode0xd6};
	instructions[0xd7] = (Instruction){"RST 10", 0, 4, opCode0xd7};
	instructions[0xd8] = (Instruction){"RET C", 0, 2, opCode0xd8};
	instructions[0xd9] = (Instruction){"RETI", 0, 4, opCode0xd9};
	instructions[0xda] = (Instruction){"JP C, $%x", 2, 3, opCode0xda};
	instructions[0xdc] = (Instruction){"CALL C, $%x", 2, 3, opCode0xdc};
	instructions[0xde] = (Instruction){"SBC A,0x%x", 1, 2, opCode0xde};
	instructions[0xdf] = (Instruction){"RST 18", 0, 4, opCode0xdf};
	instructions[0xe0] = (Instruction){"LDH 0x%x, A", 1, 3, opCode0xe0};
	instructions[0xe1] = (Instruction){"POP HL", 0, 3, opCode0xe1};
	instructions[0xe2] = (Instruction){"LD (C),A", 0, 2, opCode0xe2};
	instructions[0xe5] = (Instruction){"PUSH HL", 0, 4, opCode0xe5};
	instructions[0xe6] = (Instruction){"AND 0x%x", 1, 2, opCode0xe6};
	instructions[0xe7] = (Instruction){"RST 20", 0, 4, opCode0xe7};
	instructions[0xe8] = (Instruction){"ADD SP,0x%x", 1, 4, opCode0xe8};
	instructions[0xe9] = (Instruction){"JP (HL)", 0, 1, opCode0xe9};
	instructions[0xea] = (Instruction){"LD $%x, A", 2, 4, opCode0xea};
	instructions[0xee] = (Instruction){"XOR 0x%x", 1, 2, opCode0xee};
	instructions[0xef] = (Instruction){"RST 28", 0, 4, opCode0xef};
	instructions[0xf0] = (Instruction){"LDH A, 0x%x", 1, 3, opCode0xf0};
	instructions[0xf1] = (Instruction){"POP AF", 0, 3, opCode0xf1};
	instructions[0xf3] = (Instruction){"DI", 0, 1, opCode0xf3};
	instructions[0xf5] = (Instruction){"PUSH AF", 0, 4, opCode0xf5};
	instructions[0xf6] = (Instruction){"OR 0x%x", 1, 2, opCode0xf6};
	instructions[0xf7] = (Instruction){"RST 30", 0, 4, opCode0xf7};
	instructions[0xf8] = (Instruction){"LD HL,SP+0x%x", 1, 3, opCode0xf8};
	instructions[0xf9] = (Instruction){"LD SP,HL", 0, 2, opCode0xf9};
	instructions[0xfa] = (Instruction){"LD A, $%x", 2, 4, opCode0xfa};
	instructions[0xfb] = (Instruction){"EI", 0, 1, opCode0xfb};
	instructions[0xfe] = (Instruction){"CP 0x%x", 1, 2, opCode0xfe};
	instructions[0xff] = (Instruction){"RST 38", 0, 4, opCode0xff};

	//--------------------------------------------------------------------

	// RLC instructions (0x00-0x07)
	cb_instructions[0x00] = (Instruction){"RLC B", 0, 2, opCode0xcb00};
	cb_instructions[0x01] = (Instruction){"RLC C", 0, 2, opCode0xcb01};
	cb_instructions[0x02] = (Instruction){"RLC D", 0, 2, opCode0xcb02};
	cb_instructions[0x03] = (Instruction){"RLC E", 0, 2, opCode0xcb03};
	cb_instructions[0x04] = (Instruction){"RLC H", 0, 2, opCode0xcb04};
	cb_instructions[0x05] = (Instruction){"RLC L", 0, 2, opCode0xcb05};
	cb_instructions[0x06] = (Instruction){"RLC (HL)", 0, 4, opCode0xcb06};
	cb_instructions[0x07] = (Instruction){"RLC A", 0, 2, opCode0xcb07};

	// RRC instructions (0x08-0x0F)
	cb_instructions[0x08] = (Instruction){"RRC B", 0, 2, opCode0xcb08};
	cb_instructions[0x09] = (Instruction){"RRC C", 0, 2, opCode0xcb09};
	cb_instructions[0x0a] = (Instruction){"RRC D", 0, 2, opCode0xcb0a};
	cb_instructions[0x0b] = (Instruction){"RRC E", 0, 2, opCode0xcb0b};
	cb_instructions[0x0c] = (Instruction){"RRC H", 0, 2, opCode0xcb0c};
	cb_instructions[0x0d] = (Instruction){"RRC L", 0, 2, opCode0xcb0d};
	cb_instructions[0x0e] = (Instruction){"RRC (HL)", 0, 4, opCode0xcb0e};
	cb_instructions[0x0f] = (Instruction){"RRC A", 0, 2, opCode0xcb0f};

	// RL instructions (0x10-0x17)
	cb_instructions[0x10] = (Instruction){"RL B", 0, 2, opCode0xcb10};
	cb_instructions[0x11] = (Instruction){"RL C", 0, 2, opCode0xcb11};
	cb_instructions[0x12] = (Instruction){"RL D", 0, 2, opCode0xcb12};
	cb_instructions[0x13] = (Instruction){"RL E", 0, 2, opCode0xcb13};
	cb_instructions[0x14] = (Instruction){"RL H", 0, 2, opCode0xcb14};
	cb_instructions[0x15] = (Instruction){"RL L", 0, 2, opCode0xcb15};
	cb_instructions[0x16] = (Instruction){"RL (HL)", 0, 4, opCode0xcb16};
	cb_instructions[0x17] = (Instruction){"RL A", 0, 2, opCode0xcb17};

	// RR instructions (0x18-0x1F)
	cb_instructions[0x18] = (Instruction){"RR B", 0, 2, opCode0xcb18};
	cb_instructions[0x19] = (Instruction){"RR C", 0, 2, opCode0xcb19};
	cb_instructions[0x1a] = (Instruction){"RR D", 0, 2, opCode0xcb1a};
	cb_instructions[0x1b] = (Instruction){"RR E", 0, 2, opCode0xcb1b};
	cb_instructions[0x1c] = (Instruction){"RR H", 0, 2, opCode0xcb1c};
	cb_instructions[0x1d] = (Instruction){"RR L", 0, 2, opCode0xcb1d};
	cb_instructions[0x1e] = (Instruction){"RR (HL)", 0, 4, opCode0xcb1e};
	cb_instructions[0x1f] = (Instruction){"RR A", 0, 2, opCode0xcb1f};

	// SLA instructions (0x20-0x27)
	cb_instructions[0x20] = (Instruction){"SLA B", 0, 2, opCode0xcb20};
	cb_instructions[0x21] = (Instruction){"SLA C", 0, 2, opCode0xcb21};
	cb_instructions[0x22] = (Instruction){"SLA D", 0, 2, opCode0xcb22};
	cb_instructions[0x23] = (Instruction){"SLA E", 0, 2, opCode0xcb23};
	cb_instructions[0x24] = (Instruction){"SLA H", 0, 2, opCode0xcb24};
	cb_instructions[0x25] = (Instruction){"SLA L", 0, 2, opCode0xcb25};
	cb_instructions[0x26] = (Instruction){"SLA (HL)", 0, 4, opCode0xcb26};
	cb_instructions[0x27] = (Instruction){"SLA A", 0, 2, opCode0xcb27};

	// SRA instructions (0x28-0x2F)
	cb_instructions[0x28] = (Instruction){"SRA B", 0, 2, opCode0xcb28};
	cb_instructions[0x29] = (Instruction){"SRA C", 0, 2, opCode0xcb29};
	cb_instructions[0x2a] = (Instruction){"SRA D", 0, 2, opCode0xcb2a};
	cb_instructions[0x2b] = (Instruction){"SRA E", 0, 2, opCode0xcb2b};
	cb_instructions[0x2c] = (Instruction){"SRA H", 0, 2, opCode0xcb2c};
	cb_instructions[0x2d] = (Instruction){"SRA L", 0, 2, opCode0xcb2d};
	cb_instructions[0x2e] = (Instruction){"SRA (HL)", 0, 4, opCode0xcb2e};
	cb_instructions[0x2f] = (Instruction){"SRA A", 0, 2, opCode0xcb2f};

	// SWAP instructions (0x30-0x37)
	cb_instructions[0x30] = (Instruction){"SWAP B", 0, 2, opCode0xcb30};
	cb_instructions[0x31] = (Instruction){"SWAP C", 0, 2, opCode0xcb31};
	cb_instructions[0x32] = (Instruction){"SWAP D", 0, 2, opCode0xcb32};
	cb_instructions[0x33] = (Instruction){"SWAP E", 0, 2, opCode0xcb33};
	cb_instructions[0x34] = (Instruction){"SWAP H", 0, 2, opCode0xcb34};
	cb_instructions[0x35] = (Instruction){"SWAP L", 0, 2, opCode0xcb35};
	cb_instructions[0x36] = (Instruction){"SWAP (HL)", 0, 4, opCode0xcb36};
	cb_instructions[0x37] = (Instruction){"SWAP A", 0, 2, opCode0xcb37};

	// SRL instructions (0x38-0x3F)
	cb_instructions[0x38] = (Instruction){"SRL B", 0, 2, opCode0xcb38};
	cb_instructions[0x39] = (Instruction){"SRL C", 0, 2, opCode0xcb39};
	cb_instructions[0x3a] = (Instruction){"SRL D", 0, 2, opCode0xcb3a};
	cb_instructions[0x3b] = (Instruction){"SRL E", 0, 2, opCode0xcb3b};
	cb_instructions[0x3c] = (Instruction){"SRL H", 0, 2, opCode0xcb3c};
	cb_instructions[0x3d] = (Instruction){"SRL L", 0, 2, opCode0xcb3d};
	cb_instructions[0x3e] = (Instruction){"SRL (HL)", 0, 4, opCode0xcb3e};
	cb_instructions[0x3f] = (Instruction){"SRL A", 0, 2, opCode0xcb3f};

	// BIT instructions (0x40-0x7F)
	cb_instructions[0x40] = (Instruction){"BIT 0, B", 0, 2, opCode0xcb40};
	cb_instructions[0x41] = (Instruction){"BIT 0, C", 0, 2, opCode0xcb41};
	cb_instructions[0x42] = (Instruction){"BIT 0, D", 0, 2, opCode0xcb42};
	cb_instructions[0x43] = (Instruction){"BIT 0, E", 0, 2, opCode0xcb43};
	cb_instructions[0x44] = (Instruction){"BIT 0, H", 0, 2, opCode0xcb44};
	cb_instructions[0x45] = (Instruction){"BIT 0, L", 0, 2, opCode0xcb45};
	cb_instructions[0x46] = (Instruction){"BIT 0, (HL)", 0, 3, opCode0xcb46};
	cb_instructions[0x47] = (Instruction){"BIT 0, A", 0, 2, opCode0xcb47};
	cb_instructions[0x48] = (Instruction){"BIT 1, B", 0, 2, opCode0xcb48};
	cb_instructions[0x49] = (Instruction){"BIT 1, C", 0, 2, opCode0xcb49};
	cb_instructions[0x4a] = (Instruction){"BIT 1, D", 0, 2, opCode0xcb4a};
	cb_instructions[0x4b] = (Instruction){"BIT 1, E", 0, 2, opCode0xcb4b};
	cb_instructions[0x4c] = (Instruction){"BIT 1, H", 0, 2, opCode0xcb4c};
	cb_instructions[0x4d] = (Instruction){"BIT 1, L", 0, 2, opCode0xcb4d};
	cb_instructions[0x4e] = (Instruction){"BIT 1, (HL)", 0, 3, opCode0xcb4e};
	cb_instructions[0x4f] = (Instruction){"BIT 1, A", 0, 2, opCode0xcb4f};
	cb_instructions[0x50] = (Instruction){"BIT 2, B", 0, 2, opCode0xcb50};
	cb_instructions[0x51] = (Instruction){"BIT 2, C", 0, 2, opCode0xcb51};
	cb_instructions[0x52] = (Instruction){"BIT 2, D", 0, 2, opCode0xcb52};
	cb_instructions[0x53] = (Instruction){"BIT 2, E", 0, 2, opCode0xcb53};
	cb_instructions[0x54] = (Instruction){"BIT 2, H", 0, 2, opCode0xcb54};
	cb_instructions[0x55] = (Instruction){"BIT 2, L", 0, 2, opCode0xcb55};
	cb_instructions[0x56] = (Instruction){"BIT 2, (HL)", 0, 3, opCode0xcb56};
	cb_instructions[0x57] = (Instruction){"BIT 2, A", 0, 2, opCode0xcb57};
	cb_instructions[0x58] = (Instruction){"BIT 3, B", 0, 2, opCode0xcb58};
	cb_instructions[0x59] = (Instruction){"BIT 3, C", 0, 2, opCode0xcb59};
	cb_instructions[0x5a] = (Instruction){"BIT 3, D", 0, 2, opCode0xcb5a};
	cb_instructions[0x5b] = (Instruction){"BIT 3, E", 0, 2, opCode0xcb5b};
	cb_instructions[0x5c] = (Instruction){"BIT 3, H", 0, 2, opCode0xcb5c};
	cb_instructions[0x5d] = (Instruction){"BIT 3, L", 0, 2, opCode0xcb5d};
	cb_instructions[0x5e] = (Instruction){"BIT 3, (HL)", 0, 3, opCode0xcb5e};
	cb_instructions[0x5f] = (Instruction){"BIT 3, A", 0, 2, opCode0xcb5f};
	cb_instructions[0x60] = (Instruction){"BIT 4, B", 0, 2, opCode0xcb60};
	cb_instructions[0x61] = (Instruction){"BIT 4, C", 0, 2, opCode0xcb61};
	cb_instructions[0x62] = (Instruction){"BIT 4, D", 0, 2, opCode0xcb62};
	cb_instructions[0x63] = (Instruction){"BIT 4, E", 0, 2, opCode0xcb63};
	cb_instructions[0x64] = (Instruction){"BIT 4, H", 0, 2, opCode0xcb64};
	cb_instructions[0x65] = (Instruction){"BIT 4, L", 0, 2, opCode0xcb65};
	cb_instructions[0x66] = (Instruction){"BIT 4, (HL)", 0, 3, opCode0xcb66};
	cb_instructions[0x67] = (Instruction){"BIT 4, A", 0, 2, opCode0xcb67};
	cb_instructions[0x68] = (Instruction){"BIT 5, B", 0, 2, opCode0xcb68};
	cb_instructions[0x69] = (Instruction){"BIT 5, C", 0, 2, opCode0xcb69};
	cb_instructions[0x6a] = (Instruction){"BIT 5, D", 0, 2, opCode0xcb6a};
	cb_instructions[0x6b] = (Instruction){"BIT 5, E", 0, 2, opCode0xcb6b};
	cb_instructions[0x6c] = (Instruction){"BIT 5, H", 0, 2, opCode0xcb6c};
	cb_instructions[0x6d] = (Instruction){"BIT 5, L", 0, 2, opCode0xcb6d};
	cb_instructions[0x6e] = (Instruction){"BIT 5, (HL)", 0, 3, opCode0xcb6e};
	cb_instructions[0x6f] = (Instruction){"BIT 5, A", 0, 2, opCode0xcb6f};
	cb_instructions[0x70] = (Instruction){"BIT 6, B", 0, 2, opCode0xcb70};
	cb_instructions[0x71] = (Instruction){"BIT 6, C", 0, 2, opCode0xcb71};
	cb_instructions[0x72] = (Instruction){"BIT 6, D", 0, 2, opCode0xcb72};
	cb_instructions[0x73] = (Instruction){"BIT 6, E", 0, 2, opCode0xcb73};
	cb_instructions[0x74] = (Instruction){"BIT 6, H", 0, 2, opCode0xcb74};
	cb_instructions[0x75] = (Instruction){"BIT 6, L", 0, 2, opCode0xcb75};
	cb_instructions[0x76] = (Instruction){"BIT 6, (HL)", 0, 3, opCode0xcb76};
	cb_instructions[0x77] = (Instruction){"BIT 6, A", 0, 2, opCode0xcb77};
	cb_instructions[0x78] = (Instruction){"BIT 7, B", 0, 2, opCode0xcb78};
	cb_instructions[0x79] = (Instruction){"BIT 7, C", 0, 2, opCode0xcb79};
	cb_instructions[0x7a] = (Instruction){"BIT 7, D", 0, 2, opCode0xcb7a};
	cb_instructions[0x7b] = (Instruction){"BIT 7, E", 0, 2, opCode0xcb7b};
	cb_instructions[0x7c] = (Instruction){"BIT 7, H", 0, 2, opCode0xcb7c};
	cb_instructions[0x7d] = (Instruction){"BIT 7, L", 0, 2, opCode0xcb7d};
	cb_instructions[0x7e] = (Instruction){"BIT 7, (HL)", 0, 3, opCode0xcb7e};
	cb_instructions[0x7f] = (Instruction){"BIT 7, A", 0, 2, opCode0xcb7f};

	// RES instructions (0x80-0xBF)
	cb_instructions[0x80] = (Instruction){"RES 0, B", 0, 2, opCode0xcb80};
	cb_instructions[0x81] = (Instruction){"RES 0, C", 0, 2, opCode0xcb81};
	cb_instructions[0x82] = (Instruction){"RES 0, D", 0, 2, opCode0xcb82};
	cb_instructions[0x83] = (Instruction){"RES 0, E", 0, 2, opCode0xcb83};
	cb_instructions[0x84] = (Instruction){"RES 0, H", 0, 2, opCode0xcb84};
	cb_instructions[0x85] = (Instruction){"RES 0, L", 0, 2, opCode0xcb85};
	cb_instructions[0x86] = (Instruction){"RES 0, (HL)", 0, 4, opCode0xcb86};
	cb_instructions[0x87] = (Instruction){"RES 0, A", 0, 2, opCode0xcb87};
	cb_instructions[0x88] = (Instruction){"RES 1, B", 0, 2, opCode0xcb88};
	cb_instructions[0x89] = (Instruction){"RES 1, C", 0, 2, opCode0xcb89};
	cb_instructions[0x8a] = (Instruction){"RES 1, D", 0, 2, opCode0xcb8a};
	cb_instructions[0x8b] = (Instruction){"RES 1, E", 0, 2, opCode0xcb8b};
	cb_instructions[0x8c] = (Instruction){"RES 1, H", 0, 2, opCode0xcb8c};
	cb_instructions[0x8d] = (Instruction){"RES 1, L", 0, 2, opCode0xcb8d};
	cb_instructions[0x8e] = (Instruction){"RES 1, (HL)", 0, 4, opCode0xcb8e};
	cb_instructions[0x8f] = (Instruction){"RES 1, A", 0, 2, opCode0xcb8f};
	cb_instructions[0x90] = (Instruction){"RES 2, B", 0, 2, opCode0xcb90};
	cb_instructions[0x91] = (Instruction){"RES 2, C", 0, 2, opCode0xcb91};
	cb_instructions[0x92] = (Instruction){"RES 2, D", 0, 2, opCode0xcb92};
	cb_instructions[0x93] = (Instruction){"RES 2, E", 0, 2, opCode0xcb93};
	cb_instructions[0x94] = (Instruction){"RES 2, H", 0, 2, opCode0xcb94};
	cb_instructions[0x95] = (Instruction){"RES 2, L", 0, 2, opCode0xcb95};
	cb_instructions[0x96] = (Instruction){"RES 2, (HL)", 0, 4, opCode0xcb96};
	cb_instructions[0x97] = (Instruction){"RES 2, A", 0, 2, opCode0xcb97};
	cb_instructions[0x98] = (Instruction){"RES 3, B", 0, 2, opCode0xcb98};
	cb_instructions[0x99] = (Instruction){"RES 3, C", 0, 2, opCode0xcb99};
	cb_instructions[0x9a] = (Instruction){"RES 3, D", 0, 2, opCode0xcb9a};
	cb_instructions[0x9b] = (Instruction){"RES 3, E", 0, 2, opCode0xcb9b};
	cb_instructions[0x9c] = (Instruction){"RES 3, H", 0, 2, opCode0xcb9c};
	cb_instructions[0x9d] = (Instruction){"RES 3, L", 0, 2, opCode0xcb9d};
	cb_instructions[0x9e] = (Instruction){"RES 3, (HL)", 0, 4, opCode0xcb9e};
	cb_instructions[0x9f] = (Instruction){"RES 3, A", 0, 2, opCode0xcb9f};
	cb_instructions[0xa0] = (Instruction){"RES 4, B", 0, 2, opCode0xcba0};
	cb_instructions[0xa1] = (Instruction){"RES 4, C", 0, 2, opCode0xcba1};
	cb_instructions[0xa2] = (Instruction){"RES 4, D", 0, 2, opCode0xcba2};
	cb_instructions[0xa3] = (Instruction){"RES 4, E", 0, 2, opCode0xcba3};
	cb_instructions[0xa4] = (Instruction){"RES 4, H", 0, 2, opCode0xcba4};
	cb_instructions[0xa5] = (Instruction){"RES 4, L", 0, 2, opCode0xcba5};
	cb_instructions[0xa6] = (Instruction){"RES 4, (HL)", 0, 4, opCode0xcba6};
	cb_instructions[0xa7] = (Instruction){"RES 4, A", 0, 2, opCode0xcba7};
	cb_instructions[0xa8] = (Instruction){"RES 5, B", 0, 2, opCode0xcba8};
	cb_instructions[0xa9] = (Instruction){"RES 5, C", 0, 2, opCode0xcba9};
	cb_instructions[0xaa] = (Instruction){"RES 5, D", 0, 2, opCode0xcbaa};
	cb_instructions[0xab] = (Instruction){"RES 5, E", 0, 2, opCode0xcbab};
	cb_instructions[0xac] = (Instruction){"RES 5, H", 0, 2, opCode0xcbac};
	cb_instructions[0xad] = (Instruction){"RES 5, L", 0, 2, opCode0xcbad};
	cb_instructions[0xae] = (Instruction){"RES 5, (HL)", 0, 4, opCode0xcbae};
	cb_instructions[0xaf] = (Instruction){"RES 5, A", 0, 2, opCode0xcbaf};
	cb_instructions[0xb0] = (Instruction){"RES 6, B", 0, 2, opCode0xcbb0};
	cb_instructions[0xb1] = (Instruction){"RES 6, C", 0, 2, opCode0xcbb1};
	cb_instructions[0xb2] = (Instruction){"RES 6, D", 0, 2, opCode0xcbb2};
	cb_instructions[0xb3] = (Instruction){"RES 6, E", 0, 2, opCode0xcbb3};
	cb_instructions[0xb4] = (Instruction){"RES 6, H", 0, 2, opCode0xcbb4};
	cb_instructions[0xb5] = (Instruction){"RES 6, L", 0, 2, opCode0xcbb5};
	cb_instructions[0xb6] = (Instruction){"RES 6, (HL)", 0, 4, opCode0xcbb6};
	cb_instructions[0xb7] = (Instruction){"RES 6, A", 0, 2, opCode0xcbb7};
	cb_instructions[0xb8] = (Instruction){"RES 7, B", 0, 2, opCode0xcbb8};
	cb_instructions[0xb9] = (Instruction){"RES 7, C", 0, 2, opCode0xcbb9};
	cb_instructions[0xba] = (Instruction){"RES 7, D", 0, 2, opCode0xcbba};
	cb_instructions[0xbb] = (Instruction){"RES 7, E", 0, 2, opCode0xcbbb};
	cb_instructions[0xbc] = (Instruction){"RES 7, H", 0, 2, opCode0xcbbc};
	cb_instructions[0xbd] = (Instruction){"RES 7, L", 0, 2, opCode0xcbbd};
	cb_instructions[0xbe] = (Instruction){"RES 7, (HL)", 0, 4, opCode0xcbbe};
	cb_instructions[0xbf] = (Instruction){"RES 7, A", 0, 2, opCode0xcbbf};

	// SET instructions (0xC0-0xFF)
	cb_instructions[0xc0] = (Instruction){"SET 0, B", 0, 2, opCode0xcbc0};
	cb_instructions[0xc1] = (Instruction){"SET 0, C", 0, 2, opCode0xcbc1};
	cb_instructions[0xc2] = (Instruction){"SET 0, D", 0, 2, opCode0xcbc2};
	cb_instructions[0xc3] = (Instruction){"SET 0, E", 0, 2, opCode0xcbc3};
	cb_instructions[0xc4] = (Instruction){"SET 0, H", 0, 2, opCode0xcbc4};
	cb_instructions[0xc5] = (Instruction){"SET 0, L", 0, 2, opCode0xcbc5};
	cb_instructions[0xc6] = (Instruction){"SET 0, (HL)", 0, 4, opCode0xcbc6};
	cb_instructions[0xc7] = (Instruction){"SET 0, A", 0, 2, opCode0xcbc7};
	cb_instructions[0xc8] = (Instruction){"SET 1, B", 0, 2, opCode0xcbc8};
	cb_instructions[0xc9] = (Instruction){"SET 1, C", 0, 2, opCode0xcbc9};
	cb_instructions[0xca] = (Instruction){"SET 1, D", 0, 2, opCode0xcbca};
	cb_instructions[0xcb] = (Instruction){"SET 1, E", 0, 2, opCode0xcbcb};
	cb_instructions[0xcc] = (Instruction){"SET 1, H", 0, 2, opCode0xcbcc};
	cb_instructions[0xcd] = (Instruction){"SET 1, L", 0, 2, opCode0xcbcd};
	cb_instructions[0xce] = (Instruction){"SET 1, (HL)", 0, 4, opCode0xcbce};
	cb_instructions[0xcf] = (Instruction){"SET 1, A", 0, 2, opCode0xcbcf};
	cb_instructions[0xd0] = (Instruction){"SET 2, B", 0, 2, opCode0xcbd0};
	cb_instructions[0xd1] = (Instruction){"SET 2, C", 0, 2, opCode0xcbd1};
	cb_instructions[0xd2] = (Instruction){"SET 2, D", 0, 2, opCode0xcbd2};
	cb_instructions[0xd3] = (Instruction){"SET 2, E", 0, 2, opCode0xcbd3};
	cb_instructions[0xd4] = (Instruction){"SET 2, H", 0, 2, opCode0xcbd4};
	cb_instructions[0xd5] = (Instruction){"SET 2, L", 0, 2, opCode0xcbd5};
	cb_instructions[0xd6] = (Instruction){"SET 2, (HL)", 0, 4, opCode0xcbd6};
	cb_instructions[0xd7] = (Instruction){"SET 2, A", 0, 2, opCode0xcbd7};
	cb_instructions[0xd8] = (Instruction){"SET 3, B", 0, 2, opCode0xcbd8};
	cb_instructions[0xd9] = (Instruction){"SET 3, C", 0, 2, opCode0xcbd9};
	cb_instructions[0xda] = (Instruction){"SET 3, D", 0, 2, opCode0xcbda};
	cb_instructions[0xdb] = (Instruction){"SET 3, E", 0, 2, opCode0xcbdb};
	cb_instructions[0xdc] = (Instruction){"SET 3, H", 0, 2, opCode0xcbdc};
	cb_instructions[0xdd] = (Instruction){"SET 3, L", 0, 2, opCode0xcbdd};
	cb_instructions[0xde] = (Instruction){"SET 3, (HL)", 0, 4, opCode0xcbde};
	cb_instructions[0xdf] = (Instruction){"SET 3, A", 0, 2, opCode0xcbdf};
	cb_instructions[0xe0] = (Instruction){"SET 4, B", 0, 2, opCode0xcbe0};
	cb_instructions[0xe1] = (Instruction){"SET 4, C", 0, 2, opCode0xcbe1};
	cb_instructions[0xe2] = (Instruction){"SET 4, D", 0, 2, opCode0xcbe2};
	cb_instructions[0xe3] = (Instruction){"SET 4, E", 0, 2, opCode0xcbe3};
	cb_instructions[0xe4] = (Instruction){"SET 4, H", 0, 2, opCode0xcbe4};
	cb_instructions[0xe5] = (Instruction){"SET 4, L", 0, 2, opCode0xcbe5};
	cb_instructions[0xe6] = (Instruction){"SET 4, (HL)", 0, 4, opCode0xcbe6};
	cb_instructions[0xe7] = (Instruction){"SET 4, A", 0, 2, opCode0xcbe7};
	cb_instructions[0xe8] = (Instruction){"SET 5, B", 0, 2, opCode0xcbe8};
	cb_instructions[0xe9] = (Instruction){"SET 5, C", 0, 2, opCode0xcbe9};
	cb_instructions[0xea] = (Instruction){"SET 5, D", 0, 2, opCode0xcbea};
	cb_instructions[0xeb] = (Instruction){"SET 5, E", 0, 2, opCode0xcbeb};
	cb_instructions[0xec] = (Instruction){"SET 5, H", 0, 2, opCode0xcbec};
	cb_instructions[0xed] = (Instruction){"SET 5, L", 0, 2, opCode0xcbed};
	cb_instructions[0xee] = (Instruction){"SET 5, (HL)", 0, 4, opCode0xcbee};
	cb_instructions[0xef] = (Instruction){"SET 5, A", 0, 2, opCode0xcbef};
	cb_instructions[0xf0] = (Instruction){"SET 6, B", 0, 2, opCode0xcbf0};
	cb_instructions[0xf1] = (Instruction){"SET 6, C", 0, 2, opCode0xcbf1};
	cb_instructions[0xf2] = (Instruction){"SET 6, D", 0, 2, opCode0xcbf2};
	cb_instructions[0xf3] = (Instruction){"SET 6, E", 0, 2, opCode0xcbf3};
	cb_instructions[0xf4] = (Instruction){"SET 6, H", 0, 2, opCode0xcbf4};
	cb_instructions[0xf5] = (Instruction){"SET 6, L", 0, 2, opCode0xcbf5};
	cb_instructions[0xf6] = (Instruction){"SET 6, (HL)", 0, 4, opCode0xcbf6};
	cb_instructions[0xf7] = (Instruction){"SET 6, A", 0, 2, opCode0xcbf7};
	cb_instructions[0xf8] = (Instruction){"SET 7, B", 0, 2, opCode0xcbf8};
	cb_instructions[0xf9] = (Instruction){"SET 7, C", 0, 2, opCode0xcbf9};
	cb_instructions[0xfa] = (Instruction){"SET 7, D", 0, 2, opCode0xcbfa};
	cb_instructions[0xfb] = (Instruction){"SET 7, E", 0, 2, opCode0xcbfb};
	cb_instructions[0xfc] = (Instruction){"SET 7, H", 0, 2, opCode0xcbfc};
	cb_instructions[0xfd] = (Instruction){"SET 7, L", 0, 2, opCode0xcbfd};
	cb_instructions[0xfe] = (Instruction){"SET 7, (HL)", 0, 4, opCode0xcbfe};
	cb_instructions[0xff] = (Instruction){"SET 7, A", 0, 2, opCode0xcbff};

}
