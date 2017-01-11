
int8_t opCode0x05(Cpu* cpu){ // DEC B
	cpu->reg_b--;
	cpu_inc_toggle_bits(cpu, &(cpu->reg_b));

	return PC_NO_JMP;
}

int8_t opCode0x06(Cpu* cpu){ //LD B, n
	uint8_t value = get_one_byte_parameter(cpu);
	cpu->reg_b = value;
	return PC_NO_JMP;
}

int8_t opCode0x08(Cpu* cpu){ // LD (nn), SP
	uint16_t addr = get_two_byte_parameter(cpu);
	write_addr_to_ram(cpu->interconnect, addr, cpu->reg_sp);
	return PC_NO_JMP;
}

int8_t opCode0x0c(Cpu* cpu){
	cpu->reg_c++;
	cpu_inc_toggle_bits(cpu, &(cpu->reg_c));

	return PC_NO_JMP;
}

int8_t opCode0x0e(Cpu* cpu){ // LD C, n
	uint8_t addr = get_one_byte_parameter(cpu);
	cpu->reg_c = addr;
	return PC_NO_JMP;
}

int8_t opCode0x11(Cpu* cpu){ //LD, DE, n
	uint16_t value = get_one_byte_parameter(cpu);
	cpu->reg_de = value;
	return PC_NO_JMP;
}

int8_t opCode0x1a(Cpu* cpu){ // LD A, (DE)
	cpu->reg_a = cpu->reg_de;
	return PC_NO_JMP;
}

int8_t opCode0x13(Cpu* cpu){ // INC DE
	cpu->reg_de++;
	return PC_NO_JMP;
}

int8_t opCode0x17(Cpu* cpu){ // RLA
	opcodes_rl_reg_a(cpu, &(cpu->reg_a));
	return PC_NO_JMP;
}

int8_t opCode0x20(Cpu* cpu){ // JRNZ
	int8_t addr_offset = get_one_byte_parameter(cpu); 
	if (! get_bit( &(cpu->reg_f), FLAG_BIT_Z)){
		cpu->reg_pc += addr_offset + 2;
		return PC_JMP;
	}
	return PC_NO_JMP;	
}

int8_t opCode0x21(Cpu* cpu){ // LD HL, $nn
	uint16_t value = get_two_byte_parameter(cpu);
	cpu->reg_hl = value;
	return PC_NO_JMP;
}

int8_t opCode0x22(Cpu* cpu){ // LDI (HL), A
	write_to_ram(cpu->interconnect, cpu->reg_hl, cpu->reg_a);
	cpu->reg_hl++;
	return PC_NO_JMP;
}

int8_t opCode0x23(Cpu* cpu){ // INC HL
	cpu->reg_hl++;
	return PC_NO_JMP;
}

int8_t opCode0x28(Cpu* cpu){ // JR Z, n
	int8_t offset = get_one_byte_parameter(cpu);

	if (get_bit(&(cpu->reg_f), FLAG_BIT_Z)){
		cpu->reg_pc = cpu->reg_pc + 2 + offset;
		printf("reg PC: %x\n", cpu->reg_pc);
		exit(-1);
		return PC_JMP;
	}
	return PC_NO_JMP;
}	

int8_t opCode0x2f(Cpu* cpu){ // CPL
	cpu->reg_a = cpu->reg_a ^ 1;
	set_bit(&(cpu->reg_f), FLAG_BIT_N);
	set_bit(&(cpu->reg_f), FLAG_BIT_H);
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

int8_t opCode0x3d(Cpu* cpu){ // DEC A
	cpu->reg_a--;
	cpu_inc_toggle_bits(cpu, &(cpu->reg_a));
	return PC_NO_JMP;
}

int8_t opCode0x3e(Cpu* cpu){ // LD A,n
	uint8_t value = get_one_byte_parameter(cpu);
	cpu->reg_a = value;
	return PC_NO_JMP;
}

int8_t opCode0x4f(Cpu* cpu){ //
	cpu->reg_c = cpu->reg_a;
	return PC_NO_JMP;
}

int8_t opCode0x77(Cpu* cpu){ // LD (HL), A
	cpu->reg_hl = cpu->reg_a;
	return PC_NO_JMP;
}

int8_t opCode0x7b(Cpu* cpu){ // LD A, E
	cpu->reg_a = cpu->reg_e;
	return PC_NO_JMP;
}

int8_t opCode0xaf(Cpu* cpu){// XOR A, A
	uint8_t result = cpu->reg_a ^ cpu->reg_a;

	cpu->reg_a = result;

	if (! result){
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	clear_bit(&(cpu->reg_f), FLAG_BIT_N);
	clear_bit(&(cpu->reg_f), FLAG_BIT_H);
	clear_bit(&(cpu->reg_f), FLAG_BIT_C);



	return PC_NO_JMP;
}

int8_t opCode0xc1(Cpu* cpu){ // POP BC
	pop_stack(cpu, &(cpu->reg_bc));
	return PC_NO_JMP;
}

int8_t opCode0xc5(Cpu* cpu){ // PUSH BC
	push_stack(cpu, cpu->reg_bc);
	return PC_NO_JMP;
}

int8_t opCode0xc9(Cpu* cpu){ // RET
	pop_stack(cpu, &(cpu->reg_pc));
	return PC_JMP;
}

int8_t opCode0xcd(Cpu* cpu){ // CALL n
	uint16_t new_addr = get_two_byte_parameter(cpu);
	push_stack(cpu, cpu->reg_pc +3);
	cpu->reg_pc = new_addr;
	return PC_JMP;
}

int8_t opCode0xe0(Cpu* cpu){ // LDH (n), A
	uint8_t offset = get_one_byte_parameter(cpu);
	uint16_t addr = 0xFF00 + offset;

	write_to_ram(cpu->interconnect, addr, cpu->reg_a);
	return PC_NO_JMP;
}

int8_t opCode0xe2(Cpu* cpu){ // LD(C), A
	uint8_t value = cpu->reg_a;
	uint16_t target = 0xFF00 + cpu->reg_c;
	write_to_ram(cpu->interconnect, target, value);
	return PC_NO_JMP;
}

int8_t opCode0xea(Cpu* cpu){ // LD (nn), A
	uint16_t addr = get_two_byte_parameter(cpu);
	write_addr_to_ram(cpu->interconnect, addr, cpu->reg_a);
	return PC_NO_JMP;
}

int8_t opCode0xfe(Cpu* cpu){ // CP n
	uint8_t value = get_one_byte_parameter(cpu);
	opcodes_cp(cpu, value);
	return PC_NO_JMP;
}

int8_t opCode0xff(Cpu* cpu){ // RST 38
	push_stack(cpu, cpu->reg_pc);
	cpu->reg_pc = 0x0038;
	return PC_JMP;
}

//---------------------------------------------------------------


int8_t opCode0xcb11(Cpu* cpu){ //RL C
	opcodes_rl(cpu, &(cpu->reg_c));
	return PC_NO_JMP;
}

int8_t opCode0xcb7c(Cpu* cpu){ // BIT 7h
	if ( ! get_bit(&(cpu->reg_h), 7) ){
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);	
	}else{
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	clear_bit(& (cpu->reg_f), FLAG_BIT_N);
	set_bit(&(cpu->reg_f), FLAG_BIT_H);
	return PC_NO_JMP;
}
