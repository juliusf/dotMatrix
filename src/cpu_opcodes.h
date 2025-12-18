
int8_t opCode0x00(Cpu* cpu){ // NOP
	// No operation
	return PC_NO_JMP;
}

int8_t opCode0x04(Cpu* cpu){ // INC B
	cpu->reg_b++;
	cpu_inc_toggle_bits(cpu, &(cpu->reg_b));

	return PC_NO_JMP;
}

int8_t opCode0x05(Cpu* cpu){ // DEC B
	cpu->reg_b--;
	cpu_dec_toggle_bits(cpu, &(cpu->reg_b));

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

int8_t opCode0x0c(Cpu* cpu){ // INC C
	cpu->reg_c++;
	cpu_inc_toggle_bits(cpu, &(cpu->reg_c));

	return PC_NO_JMP;
}

int8_t opCode0x0d(Cpu* cpu){ // DEC C
	cpu->reg_c--;
	cpu_dec_toggle_bits(cpu, &(cpu->reg_c));

	return PC_NO_JMP;
}

int8_t opCode0x0e(Cpu* cpu){ // LD C, n
	uint8_t addr = get_one_byte_parameter(cpu);
	cpu->reg_c = addr;
	return PC_NO_JMP;
}

int8_t opCode0x11(Cpu* cpu){ //LD, DE, n
	uint16_t value = get_two_byte_parameter(cpu);
	cpu->reg_de = value;
	return PC_NO_JMP;
}

int8_t opCode0x1a(Cpu* cpu){ // LD A, (DE)
	cpu->reg_a = read_from_ram(cpu->interconnect, cpu->reg_de);
	return PC_NO_JMP;
}

int8_t opCode0x1d(Cpu* cpu){ // DEC E
	cpu->reg_e--;
	cpu_dec_toggle_bits(cpu, &(cpu->reg_e));
	return PC_NO_JMP;
}

int8_t opCode0x1e(Cpu* cpu){ // LD E, n
	uint8_t value = get_one_byte_parameter(cpu);
	cpu->reg_e = value;
	return PC_NO_JMP;
}

int8_t opCode0x13(Cpu* cpu){ // INC DE
	cpu->reg_de++;
	return PC_NO_JMP;
}

int8_t opCode0x15(Cpu* cpu){ // DEC D
	cpu->reg_d--;
	cpu_dec_toggle_bits(cpu, &(cpu->reg_d));
	return PC_NO_JMP;
}

int8_t opCode0x16(Cpu* cpu){ // LD D, n
	uint8_t value = get_one_byte_parameter(cpu);
	cpu->reg_d = value;
	return PC_NO_JMP;
}

int8_t opCode0x17(Cpu* cpu){ // RLA
	opcodes_rl(cpu, &(cpu->reg_a));
	return PC_NO_JMP;
}

int8_t opCode0x18(Cpu* cpu){ // JR n
	int8_t addr_offset = get_one_byte_parameter(cpu);
	cpu->reg_pc += addr_offset + 2;
	return PC_JMP;
}

int8_t opCode0x19(Cpu* cpu){ // ADD HL, DE
	opcodes_add_hl(cpu, cpu->reg_de);
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

int8_t opCode0x24(Cpu* cpu){ // INC H
	cpu->reg_h++;
	cpu_inc_toggle_bits(cpu, &(cpu->reg_h));
	return PC_NO_JMP;
}

int8_t opCode0x25(Cpu* cpu){ // DEC H
	cpu->reg_h--;
	cpu_dec_toggle_bits(cpu, &(cpu->reg_h));
	return PC_NO_JMP;
}

int8_t opCode0x28(Cpu* cpu){ // JR Z, n
	int8_t offset = get_one_byte_parameter(cpu);

	if (get_bit(&(cpu->reg_f), FLAG_BIT_Z)){
		cpu->reg_pc = cpu->reg_pc + 2 + offset;
		return PC_JMP;
	}
	return PC_NO_JMP;
}

int8_t opCode0x2c(Cpu* cpu){ // INC L
	cpu->reg_l++;
	cpu_inc_toggle_bits(cpu, &(cpu->reg_l));
	return PC_NO_JMP;
}

int8_t opCode0x2f(Cpu* cpu){ // CPL
	// CPL: Complement A (invert all bits)
	// N=1, H=1, Z and C unchanged
	cpu->reg_a = ~cpu->reg_a;
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

int8_t opCode0x3c(Cpu* cpu){ // INC A
	cpu->reg_a++;
	cpu_inc_toggle_bits(cpu, &(cpu->reg_a));
	return PC_NO_JMP;
}

int8_t opCode0x3d(Cpu* cpu){ // DEC A
	cpu->reg_a--;
	cpu_dec_toggle_bits(cpu, &(cpu->reg_a));
	return PC_NO_JMP;
}

int8_t opCode0x3e(Cpu* cpu){ // LD A,n
	uint8_t value = get_one_byte_parameter(cpu);
	cpu->reg_a = value;
	return PC_NO_JMP;
}

int8_t opCode0x4f(Cpu* cpu){ // LD C, A
	cpu->reg_c = cpu->reg_a;
	return PC_NO_JMP;
}

int8_t opCode0x57(Cpu* cpu){ // LD D, A
	cpu->reg_d = cpu->reg_a;
	return PC_NO_JMP;
}

int8_t opCode0x67(Cpu* cpu){ // LD H, A
	cpu->reg_h = cpu->reg_a;
	return PC_NO_JMP;
}

int8_t opCode0x77(Cpu* cpu){ // LD (HL), A
	write_to_ram(cpu->interconnect, cpu->reg_hl, cpu->reg_a);
	return PC_NO_JMP;
}

int8_t opCode0x78(Cpu* cpu){ // LD A, B
	cpu->reg_a = cpu->reg_b;
	return PC_NO_JMP;
}

int8_t opCode0x7b(Cpu* cpu){ // LD A, E
	cpu->reg_a = cpu->reg_e;
	return PC_NO_JMP;
}

int8_t opCode0x7c(Cpu* cpu){ // LD A, H
	cpu->reg_a = cpu->reg_h;
	return PC_NO_JMP;
}

int8_t opCode0x7d(Cpu* cpu){ // LD A, L
	cpu->reg_a = cpu->reg_l;
	return PC_NO_JMP;
}

int8_t opCode0x80(Cpu* cpu){ // ADD A, B
	opcodes_add(cpu, cpu->reg_b);
	return PC_NO_JMP;
}

int8_t opCode0x81(Cpu* cpu){ // ADD A, C
	opcodes_add(cpu, cpu->reg_c);
	return PC_NO_JMP;
}

int8_t opCode0x82(Cpu* cpu){ // ADD A, D
	opcodes_add(cpu, cpu->reg_d);
	return PC_NO_JMP;
}

int8_t opCode0x83(Cpu* cpu){ // ADD A, E
	opcodes_add(cpu, cpu->reg_e);
	return PC_NO_JMP;
}

int8_t opCode0x84(Cpu* cpu){ // ADD A, H
	opcodes_add(cpu, cpu->reg_h);
	return PC_NO_JMP;
}

int8_t opCode0x85(Cpu* cpu){ // ADD A, L
	opcodes_add(cpu, cpu->reg_l);
	return PC_NO_JMP;
}

int8_t opCode0x86(Cpu* cpu){ // ADD A, (HL)
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	opcodes_add(cpu, value);
	return PC_NO_JMP;
}

int8_t opCode0x87(Cpu* cpu){ // ADD A, A
	opcodes_add(cpu, cpu->reg_a);
	return PC_NO_JMP;
}

int8_t opCode0x90(Cpu* cpu){ // SUB B
	opcodes_sub(cpu, cpu->reg_b);
	return PC_NO_JMP;
}

int8_t opCode0xaf(Cpu* cpu){// XOR A, A
	uint8_t result = cpu->reg_a ^ cpu->reg_a;

	cpu->reg_a = result;

	// Set Z flag if result is 0, clear otherwise
	if (result == 0){
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	clear_bit(&(cpu->reg_f), FLAG_BIT_N);
	clear_bit(&(cpu->reg_f), FLAG_BIT_H);
	clear_bit(&(cpu->reg_f), FLAG_BIT_C);

	return PC_NO_JMP;
}

int8_t opCode0xbe(Cpu* cpu){ // CP (HL)
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	opcodes_cp(cpu, value);
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

int8_t opCode0xc6(Cpu* cpu){ // ADD A, n
	uint8_t value = get_one_byte_parameter(cpu);
	opcodes_add(cpu, value);
	return PC_NO_JMP;
}

int8_t opCode0xc9(Cpu* cpu){ // RET
	pop_stack(cpu, &(cpu->reg_pc));
	return PC_JMP;
}

int8_t opCode0xd1(Cpu* cpu){ // POP DE
	pop_stack(cpu, &(cpu->reg_de));
	return PC_NO_JMP;
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

int8_t opCode0xe5(Cpu* cpu){ // PUSH HL
	push_stack(cpu, cpu->reg_hl);
	return PC_NO_JMP;
}

int8_t opCode0xea(Cpu* cpu){ // LD (nn), A
	uint16_t addr = get_two_byte_parameter(cpu);
	write_to_ram(cpu->interconnect, addr, cpu->reg_a);
	return PC_NO_JMP;
}

int8_t opCode0xf0(Cpu* cpu){ // LDH A, (n)
	uint8_t offset = get_one_byte_parameter(cpu);
	uint16_t addr = 0xFF00 + offset;
	cpu->reg_a = read_from_ram(cpu->interconnect, addr);
	return PC_NO_JMP;
}

int8_t opCode0xfe(Cpu* cpu){ // CP n
	uint8_t value = get_one_byte_parameter(cpu);
	opcodes_cp(cpu, value);
	return PC_NO_JMP;
}

int8_t opCode0xff(Cpu* cpu){ // RST 38
	push_stack(cpu, cpu->reg_pc + 1);
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
