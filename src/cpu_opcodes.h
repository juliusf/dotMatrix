
int8_t opCode0x00(Cpu* cpu){ // NOP
	// No operation
	return PC_NO_JMP;
}

int8_t opCode0x01(Cpu* cpu){ // LD BC, nn
	uint16_t value = get_two_byte_parameter(cpu);
	cpu->reg_bc = value;
	return PC_NO_JMP;
}

int8_t opCode0x03(Cpu* cpu){ // INC BC
	cpu->reg_bc++;
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

int8_t opCode0x07(Cpu* cpu){ // RLCA
	// RLCA: Rotate A left, bit 7 -> C and bit 0
	uint8_t bit7 = (cpu->reg_a & 0x80) >> 7;
	cpu->reg_a = (cpu->reg_a << 1) | bit7;

	if (bit7) {
		set_bit(&(cpu->reg_f), FLAG_BIT_C);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_C);
	}

	// RLCA always clears Z, N, and H flags
	clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	clear_bit(&(cpu->reg_f), FLAG_BIT_N);
	clear_bit(&(cpu->reg_f), FLAG_BIT_H);
	return PC_NO_JMP;
}

int8_t opCode0x08(Cpu* cpu){ // LD (nn), SP
	uint16_t addr = get_two_byte_parameter(cpu);
	write_addr_to_ram(cpu->interconnect, addr, cpu->reg_sp);
	return PC_NO_JMP;
}

int8_t opCode0x09(Cpu* cpu){ // ADD HL, BC
	opcodes_add_hl(cpu, cpu->reg_bc);
	return PC_NO_JMP;
}

int8_t opCode0x0a(Cpu* cpu){ // LD A, (BC)
	cpu->reg_a = read_from_ram(cpu->interconnect, cpu->reg_bc);
	return PC_NO_JMP;
}

int8_t opCode0x0b(Cpu* cpu){ // DEC BC
	cpu->reg_bc--;
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

int8_t opCode0x12(Cpu* cpu){ // LD (DE), A
	write_to_ram(cpu->interconnect, cpu->reg_de, cpu->reg_a);
	return PC_NO_JMP;
}

int8_t opCode0x1a(Cpu* cpu){ // LD A, (DE)
	cpu->reg_a = read_from_ram(cpu->interconnect, cpu->reg_de);
	return PC_NO_JMP;
}

int8_t opCode0x1b(Cpu* cpu){ // DEC DE
	cpu->reg_de--;
	return PC_NO_JMP;
}

int8_t opCode0x1c(Cpu* cpu){ // INC E
	cpu->reg_e++;
	cpu_inc_toggle_bits(cpu, &(cpu->reg_e));
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

int8_t opCode0x14(Cpu* cpu){ // INC D
	cpu->reg_d++;
	cpu_inc_toggle_bits(cpu, &(cpu->reg_d));
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

int8_t opCode0x1f(Cpu* cpu){ // RRA
	opcodes_rr(cpu, &(cpu->reg_a));
	// RRA always clears Z flag (unlike CB RR which sets Z based on result)
	clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	return PC_NO_JMP;
}

int8_t opCode0x19(Cpu* cpu){ // ADD HL, DE
	opcodes_add_hl(cpu, cpu->reg_de);
	return PC_NO_JMP;
}

int8_t opCode0x20(Cpu* cpu){ // JRNZ
	int8_t addr_offset = get_one_byte_parameter(cpu);
	uint8_t z_flag = get_bit( &(cpu->reg_f), FLAG_BIT_Z);

	if (! z_flag){
		cpu->reg_pc += addr_offset + 2;
		cpu->cycles_left = 3; // Branch taken: 12 T-cycles = 3 M-cycles
		return PC_JMP;
	}
	cpu->cycles_left = 2; // Branch not taken: 8 T-cycles = 2 M-cycles
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

int8_t opCode0x26(Cpu* cpu){ // LD H, n
	uint8_t value = get_one_byte_parameter(cpu);
	cpu->reg_h = value;
	return PC_NO_JMP;
}

int8_t opCode0x28(Cpu* cpu){ // JR Z, n
	int8_t offset = get_one_byte_parameter(cpu);

	if (get_bit(&(cpu->reg_f), FLAG_BIT_Z)){
		cpu->reg_pc = cpu->reg_pc + 2 + offset;
		cpu->cycles_left = 3; // Branch taken: 12 T-cycles = 3 M-cycles
		return PC_JMP;
	}
	cpu->cycles_left = 2; // Branch not taken: 8 T-cycles = 2 M-cycles
	return PC_NO_JMP;
}

int8_t opCode0x29(Cpu* cpu){ // ADD HL, HL
	opcodes_add_hl(cpu, cpu->reg_hl);
	return PC_NO_JMP;
}

int8_t opCode0x2a(Cpu* cpu){ // LDI A, (HL)
	cpu->reg_a = read_from_ram(cpu->interconnect, cpu->reg_hl);
	cpu->reg_hl++;
	return PC_NO_JMP;
}

int8_t opCode0x2b(Cpu* cpu){ // DEC HL
	cpu->reg_hl--;
	return PC_NO_JMP;
}

int8_t opCode0x2c(Cpu* cpu){ // INC L
	cpu->reg_l++;
	cpu_inc_toggle_bits(cpu, &(cpu->reg_l));
	return PC_NO_JMP;
}

int8_t opCode0x2d(Cpu* cpu){ // DEC L
	cpu->reg_l--;
	cpu_dec_toggle_bits(cpu, &(cpu->reg_l));
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

int8_t opCode0x30(Cpu* cpu){ // JR NC, n
	int8_t offset = get_one_byte_parameter(cpu);

	if (!get_bit(&(cpu->reg_f), FLAG_BIT_C)){
		cpu->reg_pc = cpu->reg_pc + 2 + offset;
		cpu->cycles_left = 3; // Branch taken: 12 T-cycles = 3 M-cycles
		return PC_JMP;
	}
	cpu->cycles_left = 2; // Branch not taken: 8 T-cycles = 2 M-cycles
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

int8_t opCode0x34(Cpu* cpu){ // INC (HL)
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	value++;
	cpu_inc_toggle_bits(cpu, &value);
	write_to_ram(cpu->interconnect, cpu->reg_hl, value);
	return PC_NO_JMP;
}

int8_t opCode0x35(Cpu* cpu){ // DEC (HL)
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	value--;
	cpu_dec_toggle_bits(cpu, &value);
	write_to_ram(cpu->interconnect, cpu->reg_hl, value);
	return PC_NO_JMP;
}

int8_t opCode0x36(Cpu* cpu){ // LD (HL), n
	uint8_t value = get_one_byte_parameter(cpu);
	write_to_ram(cpu->interconnect, cpu->reg_hl, value);
	return PC_NO_JMP;
}

int8_t opCode0x3a(Cpu* cpu){ // LDD A, (HL)
	cpu->reg_a = read_from_ram(cpu->interconnect, cpu->reg_hl);
	cpu->reg_hl--;
	return PC_NO_JMP;
}

int8_t opCode0x38(Cpu* cpu){ // JR C, n
	int8_t offset = get_one_byte_parameter(cpu);

	if (get_bit(&(cpu->reg_f), FLAG_BIT_C)){
		cpu->reg_pc = cpu->reg_pc + 2 + offset;
		cpu->cycles_left = 3; // Branch taken: 12 T-cycles = 3 M-cycles
		return PC_JMP;
	}
	cpu->cycles_left = 2; // Branch not taken: 8 T-cycles = 2 M-cycles
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

int8_t opCode0x40(Cpu* cpu){ // LD B, B
	cpu->reg_b = cpu->reg_b;
	return PC_NO_JMP;
}

int8_t opCode0x41(Cpu* cpu){ // LD B, C
	cpu->reg_b = cpu->reg_c;
	return PC_NO_JMP;
}

int8_t opCode0x42(Cpu* cpu){ // LD B, D
	cpu->reg_b = cpu->reg_d;
	return PC_NO_JMP;
}

int8_t opCode0x43(Cpu* cpu){ // LD B, E
	cpu->reg_b = cpu->reg_e;
	return PC_NO_JMP;
}

int8_t opCode0x44(Cpu* cpu){ // LD B, H
	cpu->reg_b = cpu->reg_h;
	return PC_NO_JMP;
}

int8_t opCode0x45(Cpu* cpu){ // LD B, L
	cpu->reg_b = cpu->reg_l;
	return PC_NO_JMP;
}

int8_t opCode0x46(Cpu* cpu){ // LD B, (HL)
	cpu->reg_b = read_from_ram(cpu->interconnect, cpu->reg_hl);
	return PC_NO_JMP;
}

int8_t opCode0x47(Cpu* cpu){ // LD B, A
	cpu->reg_b = cpu->reg_a;
	return PC_NO_JMP;
}

int8_t opCode0x48(Cpu* cpu){ // LD C, B
	cpu->reg_c = cpu->reg_b;
	return PC_NO_JMP;
}

int8_t opCode0x49(Cpu* cpu){ // LD C, C
	cpu->reg_c = cpu->reg_c;
	return PC_NO_JMP;
}

int8_t opCode0x4a(Cpu* cpu){ // LD C, D
	cpu->reg_c = cpu->reg_d;
	return PC_NO_JMP;
}

int8_t opCode0x4b(Cpu* cpu){ // LD C, E
	cpu->reg_c = cpu->reg_e;
	return PC_NO_JMP;
}

int8_t opCode0x4c(Cpu* cpu){ // LD C, H
	cpu->reg_c = cpu->reg_h;
	return PC_NO_JMP;
}

int8_t opCode0x4d(Cpu* cpu){ // LD C, L
	cpu->reg_c = cpu->reg_l;
	return PC_NO_JMP;
}

int8_t opCode0x4e(Cpu* cpu){ // LD C, (HL)
	cpu->reg_c = read_from_ram(cpu->interconnect, cpu->reg_hl);
	return PC_NO_JMP;
}

int8_t opCode0x4f(Cpu* cpu){ // LD C, A
	cpu->reg_c = cpu->reg_a;
	return PC_NO_JMP;
}

int8_t opCode0x50(Cpu* cpu){ // LD D, B
	cpu->reg_d = cpu->reg_b;
	return PC_NO_JMP;
}

int8_t opCode0x51(Cpu* cpu){ // LD D, C
	cpu->reg_d = cpu->reg_c;
	return PC_NO_JMP;
}

int8_t opCode0x52(Cpu* cpu){ // LD D, D
	cpu->reg_d = cpu->reg_d;
	return PC_NO_JMP;
}

int8_t opCode0x53(Cpu* cpu){ // LD D, E
	cpu->reg_d = cpu->reg_e;
	return PC_NO_JMP;
}

int8_t opCode0x54(Cpu* cpu){ // LD D, H
	cpu->reg_d = cpu->reg_h;
	return PC_NO_JMP;
}

int8_t opCode0x55(Cpu* cpu){ // LD D, L
	cpu->reg_d = cpu->reg_l;
	return PC_NO_JMP;
}

int8_t opCode0x56(Cpu* cpu){ // LD D, (HL)
	cpu->reg_d = read_from_ram(cpu->interconnect, cpu->reg_hl);
	return PC_NO_JMP;
}

int8_t opCode0x57(Cpu* cpu){ // LD D, A
	cpu->reg_d = cpu->reg_a;
	return PC_NO_JMP;
}

int8_t opCode0x58(Cpu* cpu){ // LD E, B
	cpu->reg_e = cpu->reg_b;
	return PC_NO_JMP;
}

int8_t opCode0x59(Cpu* cpu){ // LD E, C
	cpu->reg_e = cpu->reg_c;
	return PC_NO_JMP;
}

int8_t opCode0x5a(Cpu* cpu){ // LD E, D
	cpu->reg_e = cpu->reg_d;
	return PC_NO_JMP;
}

int8_t opCode0x5b(Cpu* cpu){ // LD E, E
	cpu->reg_e = cpu->reg_e;
	return PC_NO_JMP;
}

int8_t opCode0x5c(Cpu* cpu){ // LD E, H
	cpu->reg_e = cpu->reg_h;
	return PC_NO_JMP;
}

int8_t opCode0x5d(Cpu* cpu){ // LD E, L
	cpu->reg_e = cpu->reg_l;
	return PC_NO_JMP;
}

int8_t opCode0x5e(Cpu* cpu){ // LD E, (HL)
	cpu->reg_e = read_from_ram(cpu->interconnect, cpu->reg_hl);
	return PC_NO_JMP;
}

int8_t opCode0x5f(Cpu* cpu){ // LD E, A
	cpu->reg_e = cpu->reg_a;
	return PC_NO_JMP;
}

int8_t opCode0x60(Cpu* cpu){ // LD H, B
	cpu->reg_h = cpu->reg_b;
	return PC_NO_JMP;
}

int8_t opCode0x61(Cpu* cpu){ // LD H, C
	cpu->reg_h = cpu->reg_c;
	return PC_NO_JMP;
}

int8_t opCode0x62(Cpu* cpu){ // LD H, D
	cpu->reg_h = cpu->reg_d;
	return PC_NO_JMP;
}

int8_t opCode0x63(Cpu* cpu){ // LD H, E
	cpu->reg_h = cpu->reg_e;
	return PC_NO_JMP;
}

int8_t opCode0x64(Cpu* cpu){ // LD H, H
	cpu->reg_h = cpu->reg_h;
	return PC_NO_JMP;
}

int8_t opCode0x65(Cpu* cpu){ // LD H, L
	cpu->reg_h = cpu->reg_l;
	return PC_NO_JMP;
}

int8_t opCode0x66(Cpu* cpu){ // LD H, (HL)
	cpu->reg_h = read_from_ram(cpu->interconnect, cpu->reg_hl);
	return PC_NO_JMP;
}

int8_t opCode0x67(Cpu* cpu){ // LD H, A
	cpu->reg_h = cpu->reg_a;
	return PC_NO_JMP;
}

int8_t opCode0x68(Cpu* cpu){ // LD L, B
	cpu->reg_l = cpu->reg_b;
	return PC_NO_JMP;
}

int8_t opCode0x69(Cpu* cpu){ // LD L, C
	cpu->reg_l = cpu->reg_c;
	return PC_NO_JMP;
}

int8_t opCode0x6a(Cpu* cpu){ // LD L, D
	cpu->reg_l = cpu->reg_d;
	return PC_NO_JMP;
}

int8_t opCode0x6b(Cpu* cpu){ // LD L, E
	cpu->reg_l = cpu->reg_e;
	return PC_NO_JMP;
}

int8_t opCode0x6c(Cpu* cpu){ // LD L, H
	cpu->reg_l = cpu->reg_h;
	return PC_NO_JMP;
}

int8_t opCode0x6d(Cpu* cpu){ // LD L, L
	cpu->reg_l = cpu->reg_l;
	return PC_NO_JMP;
}

int8_t opCode0x6e(Cpu* cpu){ // LD L, (HL)
	cpu->reg_l = read_from_ram(cpu->interconnect, cpu->reg_hl);
	return PC_NO_JMP;
}

int8_t opCode0x6f(Cpu* cpu){ // LD L, A
	cpu->reg_l = cpu->reg_a;
	return PC_NO_JMP;
}

int8_t opCode0x70(Cpu* cpu){ // LD (HL), B
	write_to_ram(cpu->interconnect, cpu->reg_hl, cpu->reg_b);
	return PC_NO_JMP;
}

int8_t opCode0x71(Cpu* cpu){ // LD (HL), C
	write_to_ram(cpu->interconnect, cpu->reg_hl, cpu->reg_c);
	return PC_NO_JMP;
}

int8_t opCode0x72(Cpu* cpu){ // LD (HL), D
	write_to_ram(cpu->interconnect, cpu->reg_hl, cpu->reg_d);
	return PC_NO_JMP;
}

int8_t opCode0x73(Cpu* cpu){ // LD (HL), E
	write_to_ram(cpu->interconnect, cpu->reg_hl, cpu->reg_e);
	return PC_NO_JMP;
}

int8_t opCode0x76(Cpu* cpu){ // HALT
	cpu->halted = 1;
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

int8_t opCode0x79(Cpu* cpu){ // LD A, C
	cpu->reg_a = cpu->reg_c;
	return PC_NO_JMP;
}

int8_t opCode0x7a(Cpu* cpu){ // LD A, D
	cpu->reg_a = cpu->reg_d;
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

int8_t opCode0x7e(Cpu* cpu){ // LD A, (HL)
	cpu->reg_a = read_from_ram(cpu->interconnect, cpu->reg_hl);
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

int8_t opCode0x88(Cpu* cpu){ // ADC A, B
	opcodes_adc(cpu, cpu->reg_b);
	return PC_NO_JMP;
}

int8_t opCode0x89(Cpu* cpu){ // ADC A, C
	opcodes_adc(cpu, cpu->reg_c);
	return PC_NO_JMP;
}

int8_t opCode0x90(Cpu* cpu){ // SUB B
	opcodes_sub(cpu, cpu->reg_b);
	return PC_NO_JMP;
}

int8_t opCode0xa1(Cpu* cpu){ // AND C
	opcodes_and(cpu, cpu->reg_c);
	return PC_NO_JMP;
}

int8_t opCode0xa6(Cpu* cpu){ // AND (HL)
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	opcodes_and(cpu, value);
	return PC_NO_JMP;
}

int8_t opCode0xa7(Cpu* cpu){ // AND A
	opcodes_and(cpu, cpu->reg_a);
	return PC_NO_JMP;
}

int8_t opCode0xa9(Cpu* cpu){ // XOR C
	opcodes_xor(cpu, cpu->reg_c);
	return PC_NO_JMP;
}

int8_t opCode0xae(Cpu* cpu){ // XOR (HL)
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	opcodes_xor(cpu, value);
	return PC_NO_JMP;
}

int8_t opCode0xaf(Cpu* cpu){// XOR A, A
	opcodes_xor(cpu, cpu->reg_a);
	return PC_NO_JMP;
}

int8_t opCode0xb0(Cpu* cpu){ // OR B
	opcodes_or(cpu, cpu->reg_b);
	return PC_NO_JMP;
}

int8_t opCode0xb1(Cpu* cpu){ // OR C
	opcodes_or(cpu, cpu->reg_c);
	return PC_NO_JMP;
}

int8_t opCode0xb6(Cpu* cpu){ // OR (HL)
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	opcodes_or(cpu, value);
	return PC_NO_JMP;
}

int8_t opCode0xb7(Cpu* cpu){ // OR A
	opcodes_or(cpu, cpu->reg_a);
	return PC_NO_JMP;
}

int8_t opCode0xbb(Cpu* cpu){ // CP E
	opcodes_cp(cpu, cpu->reg_e);
	return PC_NO_JMP;
}

int8_t opCode0xbe(Cpu* cpu){ // CP (HL)
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	opcodes_cp(cpu, value);
	return PC_NO_JMP;
}

int8_t opCode0xc0(Cpu* cpu){ // RET NZ
	if (!get_bit(&(cpu->reg_f), FLAG_BIT_Z)){
		pop_stack(cpu, &(cpu->reg_pc));
		cpu->cycles_left = 5; // Branch taken: 20 T-cycles = 5 M-cycles
		return PC_JMP;
	}
	cpu->cycles_left = 2; // Branch not taken: 8 T-cycles = 2 M-cycles
	return PC_NO_JMP;
}

int8_t opCode0xc1(Cpu* cpu){ // POP BC
	pop_stack(cpu, &(cpu->reg_bc));
	return PC_NO_JMP;
}

int8_t opCode0xc2(Cpu* cpu){ // JP NZ, nn
	uint16_t addr = get_two_byte_parameter(cpu);
	if (!get_bit(&(cpu->reg_f), FLAG_BIT_Z)){
		cpu->reg_pc = addr;
		cpu->cycles_left = 4; // Branch taken: 16 T-cycles = 4 M-cycles
		return PC_JMP;
	}
	cpu->cycles_left = 3; // Branch not taken: 12 T-cycles = 3 M-cycles
	return PC_NO_JMP;
}

int8_t opCode0xc3(Cpu* cpu){ // JP nn
	uint16_t addr = get_two_byte_parameter(cpu);
	cpu->reg_pc = addr;
	return PC_JMP;
}

int8_t opCode0xc4(Cpu* cpu){ // CALL NZ, nn
	uint16_t addr = get_two_byte_parameter(cpu);
	if (!get_bit(&(cpu->reg_f), FLAG_BIT_Z)){
		push_stack(cpu, cpu->reg_pc + 3);
		cpu->reg_pc = addr;
		cpu->cycles_left = 6; // Branch taken: 24 T-cycles = 6 M-cycles
		return PC_JMP;
	}
	cpu->cycles_left = 3; // Branch not taken: 12 T-cycles = 3 M-cycles
	return PC_NO_JMP;
}

int8_t opCode0xca(Cpu* cpu){ // JP Z, nn
	uint16_t addr = get_two_byte_parameter(cpu);
	if (get_bit(&(cpu->reg_f), FLAG_BIT_Z)){
		cpu->reg_pc = addr;
		cpu->cycles_left = 4; // Branch taken: 16 T-cycles = 4 M-cycles
		return PC_JMP;
	}
	cpu->cycles_left = 3; // Branch not taken: 12 T-cycles = 3 M-cycles
	return PC_NO_JMP;
}

int8_t opCode0xce(Cpu* cpu){ // ADC A, n
	uint8_t value = get_one_byte_parameter(cpu);
	opcodes_adc(cpu, value);
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

int8_t opCode0xc8(Cpu* cpu){ // RET Z
	if (get_bit(&(cpu->reg_f), FLAG_BIT_Z)){
		pop_stack(cpu, &(cpu->reg_pc));
		cpu->cycles_left = 5; // Branch taken: 20 T-cycles = 5 M-cycles
		return PC_JMP;
	}
	cpu->cycles_left = 2; // Branch not taken: 8 T-cycles = 2 M-cycles
	return PC_NO_JMP;
}

int8_t opCode0xc9(Cpu* cpu){ // RET
	pop_stack(cpu, &(cpu->reg_pc));
	// Clear interrupt flag if set (not used for timing, global RET=2 is used instead)
	if (cpu->in_interrupt) {
		cpu->in_interrupt = 0;
	}
	return PC_JMP;
}

int8_t opCode0xd0(Cpu* cpu){ // RET NC
	if (!get_bit(&(cpu->reg_f), FLAG_BIT_C)){
		pop_stack(cpu, &(cpu->reg_pc));
		cpu->cycles_left = 5; // Branch taken: 20 T-cycles = 5 M-cycles
		return PC_JMP;
	}
	cpu->cycles_left = 2; // Branch not taken: 8 T-cycles = 2 M-cycles
	return PC_NO_JMP;
}

int8_t opCode0xd1(Cpu* cpu){ // POP DE
	pop_stack(cpu, &(cpu->reg_de));
	return PC_NO_JMP;
}

int8_t opCode0xd6(Cpu* cpu){ // SUB n
	uint8_t value = get_one_byte_parameter(cpu);
	opcodes_sub(cpu, value);
	return PC_NO_JMP;
}

int8_t opCode0xd8(Cpu* cpu){ // RET C
	if (get_bit(&(cpu->reg_f), FLAG_BIT_C)){
		pop_stack(cpu, &(cpu->reg_pc));
		cpu->cycles_left = 5; // Branch taken: 20 T-cycles = 5 M-cycles
		return PC_JMP;
	}
	cpu->cycles_left = 2; // Branch not taken: 8 T-cycles = 2 M-cycles
	return PC_NO_JMP;
}

int8_t opCode0xd5(Cpu* cpu){ // PUSH DE
	push_stack(cpu, cpu->reg_de);
	return PC_NO_JMP;
}

int8_t opCode0xd9(Cpu* cpu){ // RETI
	pop_stack(cpu, &(cpu->reg_pc));
	cpu->ime = 1;  // Re-enable interrupts immediately
	// Clear interrupt flag if set
	if (cpu->in_interrupt) {
		cpu->in_interrupt = 0;
	}
	return PC_JMP;
}

int8_t opCode0xe1(Cpu* cpu){ // POP HL
	pop_stack(cpu, &(cpu->reg_hl));
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

int8_t opCode0xe6(Cpu* cpu){ // AND n
	uint8_t value = get_one_byte_parameter(cpu);
	opcodes_and(cpu, value);
	return PC_NO_JMP;
}

int8_t opCode0xe9(Cpu* cpu){ // JP (HL)
	cpu->reg_pc = cpu->reg_hl;
	return PC_JMP;
}

int8_t opCode0xea(Cpu* cpu){ // LD (nn), A
	uint16_t addr = get_two_byte_parameter(cpu);
	write_to_ram(cpu->interconnect, addr, cpu->reg_a);
	return PC_NO_JMP;
}

int8_t opCode0xee(Cpu* cpu){ // XOR n
	uint8_t value = get_one_byte_parameter(cpu);
	opcodes_xor(cpu, value);
	return PC_NO_JMP;
}

int8_t opCode0xef(Cpu* cpu){ // RST 28H
	push_stack(cpu, cpu->reg_pc + 1);
	cpu->reg_pc = 0x0028;
	return PC_JMP;
}

int8_t opCode0xf0(Cpu* cpu){ // LDH A, (n)
	uint8_t offset = get_one_byte_parameter(cpu);
	uint16_t addr = 0xFF00 + offset;
	cpu->reg_a = read_from_ram(cpu->interconnect, addr);
	return PC_NO_JMP;
}

int8_t opCode0xf1(Cpu* cpu){ // POP AF
	pop_stack(cpu, &(cpu->reg_af));
	// Lower 4 bits of F register are always 0 on Game Boy
	cpu->reg_f &= 0xF0;
	return PC_NO_JMP;
}

int8_t opCode0xf3(Cpu* cpu){ // DI (Disable Interrupts)
	cpu->ime = 0;
	return PC_NO_JMP;
}

int8_t opCode0xf5(Cpu* cpu){ // PUSH AF
	push_stack(cpu, cpu->reg_af);
	return PC_NO_JMP;
}

int8_t opCode0xf6(Cpu* cpu){ // OR n
	uint8_t value = get_one_byte_parameter(cpu);
	opcodes_or(cpu, value);
	return PC_NO_JMP;
}

int8_t opCode0xfa(Cpu* cpu){ // LD A, (nn)
	uint16_t addr = get_two_byte_parameter(cpu);
	cpu->reg_a = read_from_ram(cpu->interconnect, addr);
	return PC_NO_JMP;
}

int8_t opCode0xfb(Cpu* cpu){ // EI (Enable Interrupts)
	// EI enables interrupts after the NEXT instruction executes
	cpu->ime_scheduled = 1;
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

int8_t opCode0xcb12(Cpu* cpu){ //RL D
	opcodes_rl(cpu, &(cpu->reg_d));
	return PC_NO_JMP;
}

int8_t opCode0xcb19(Cpu* cpu){ //RR C
	opcodes_rr(cpu, &(cpu->reg_c));
	return PC_NO_JMP;
}

int8_t opCode0xcb1a(Cpu* cpu){ //RR D
	opcodes_rr(cpu, &(cpu->reg_d));
	return PC_NO_JMP;
}

int8_t opCode0xcb27(Cpu* cpu){ // SLA A
	opcodes_sla(cpu, &(cpu->reg_a));
	return PC_NO_JMP;
}

int8_t opCode0xcb33(Cpu* cpu){ // SWAP E
	opcodes_swap(cpu, &(cpu->reg_e));
	return PC_NO_JMP;
}

int8_t opCode0xcb37(Cpu* cpu){ // SWAP A
	opcodes_swap(cpu, &(cpu->reg_a));
	return PC_NO_JMP;
}

int8_t opCode0xcb38(Cpu* cpu){ // SRL B
	opcodes_srl(cpu, &(cpu->reg_b));
	return PC_NO_JMP;
}

int8_t opCode0xcb3f(Cpu* cpu){ // SRL A
	opcodes_srl(cpu, &(cpu->reg_a));
	return PC_NO_JMP;
}

int8_t opCode0xcb40(Cpu* cpu){ // BIT 0, B
	if ( ! get_bit(&(cpu->reg_b), 0) ){
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}else{
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	clear_bit(& (cpu->reg_f), FLAG_BIT_N);
	set_bit(&(cpu->reg_f), FLAG_BIT_H);
	return PC_NO_JMP;
}

int8_t opCode0xcb4c(Cpu* cpu){ // BIT 1, H
	if ( ! get_bit(&(cpu->reg_h), 1) ){
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}else{
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	clear_bit(& (cpu->reg_f), FLAG_BIT_N);
	set_bit(&(cpu->reg_f), FLAG_BIT_H);
	return PC_NO_JMP;
}

int8_t opCode0xcb4f(Cpu* cpu){ // BIT 1, A
	if ( ! get_bit(&(cpu->reg_a), 1) ){
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}else{
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	clear_bit(& (cpu->reg_f), FLAG_BIT_N);
	set_bit(&(cpu->reg_f), FLAG_BIT_H);
	return PC_NO_JMP;
}

int8_t opCode0xcb50(Cpu* cpu){ // BIT 2, B
	if ( ! get_bit(&(cpu->reg_b), 2) ){
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}else{
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	clear_bit(& (cpu->reg_f), FLAG_BIT_N);
	set_bit(&(cpu->reg_f), FLAG_BIT_H);
	return PC_NO_JMP;
}

int8_t opCode0xcb58(Cpu* cpu){ // BIT 3, B
	if ( ! get_bit(&(cpu->reg_b), 3) ){
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}else{
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	clear_bit(& (cpu->reg_f), FLAG_BIT_N);
	set_bit(&(cpu->reg_f), FLAG_BIT_H);
	return PC_NO_JMP;
}

int8_t opCode0xcb5f(Cpu* cpu){ // BIT 3, A
	if ( ! get_bit(&(cpu->reg_a), 3) ){
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}else{
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	clear_bit(& (cpu->reg_f), FLAG_BIT_N);
	set_bit(&(cpu->reg_f), FLAG_BIT_H);
	return PC_NO_JMP;
}

int8_t opCode0xcb60(Cpu* cpu){ // BIT 4, B
	if ( ! get_bit(&(cpu->reg_b), 4) ){
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}else{
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	clear_bit(& (cpu->reg_f), FLAG_BIT_N);
	set_bit(&(cpu->reg_f), FLAG_BIT_H);
	return PC_NO_JMP;
}

int8_t opCode0xcb68(Cpu* cpu){ // BIT 5, B
	if ( ! get_bit(&(cpu->reg_b), 5) ){
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}else{
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	clear_bit(& (cpu->reg_f), FLAG_BIT_N);
	set_bit(&(cpu->reg_f), FLAG_BIT_H);
	return PC_NO_JMP;
}

int8_t opCode0xcb6f(Cpu* cpu){ // BIT 5, A
	if ( ! get_bit(&(cpu->reg_a), 5) ){
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}else{
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	clear_bit(& (cpu->reg_f), FLAG_BIT_N);
	set_bit(&(cpu->reg_f), FLAG_BIT_H);
	return PC_NO_JMP;
}

int8_t opCode0xcb70(Cpu* cpu){ // BIT 6, B
	if ( ! get_bit(&(cpu->reg_b), 6) ){
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}else{
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	clear_bit(& (cpu->reg_f), FLAG_BIT_N);
	set_bit(&(cpu->reg_f), FLAG_BIT_H);
	return PC_NO_JMP;
}

int8_t opCode0xcb77(Cpu* cpu){ // BIT 6, A
	if ( ! get_bit(&(cpu->reg_a), 6) ){
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}else{
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	clear_bit(& (cpu->reg_f), FLAG_BIT_N);
	set_bit(&(cpu->reg_f), FLAG_BIT_H);
	return PC_NO_JMP;
}

int8_t opCode0xcb78(Cpu* cpu){ // BIT 7, B
	if ( ! get_bit(&(cpu->reg_b), 7) ){
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}else{
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	clear_bit(& (cpu->reg_f), FLAG_BIT_N);
	set_bit(&(cpu->reg_f), FLAG_BIT_H);
	return PC_NO_JMP;
}

int8_t opCode0xcb7c(Cpu* cpu){ // BIT 7, H
	if ( ! get_bit(&(cpu->reg_h), 7) ){
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}else{
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	clear_bit(& (cpu->reg_f), FLAG_BIT_N);
	set_bit(&(cpu->reg_f), FLAG_BIT_H);
	return PC_NO_JMP;
}

int8_t opCode0xcb7e(Cpu* cpu){ // BIT 7, (HL)
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	if ( ! get_bit(&value, 7) ){
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}else{
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	clear_bit(& (cpu->reg_f), FLAG_BIT_N);
	set_bit(&(cpu->reg_f), FLAG_BIT_H);
	return PC_NO_JMP;
}

int8_t opCode0xcb7f(Cpu* cpu){ // BIT 7, A
	if ( ! get_bit(&(cpu->reg_a), 7) ){
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}else{
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	clear_bit(& (cpu->reg_f), FLAG_BIT_N);
	set_bit(&(cpu->reg_f), FLAG_BIT_H);
	return PC_NO_JMP;
}

int8_t opCode0xcb86(Cpu* cpu){ // RES 0, (HL)
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	clear_bit(&value, 0);
	write_to_ram(cpu->interconnect, cpu->reg_hl, value);
	return PC_NO_JMP;
}

int8_t opCode0xcb87(Cpu* cpu){ // RES 0, A
	clear_bit(&(cpu->reg_a), 0);
	return PC_NO_JMP;
}

int8_t opCode0xcbfe(Cpu* cpu){ // SET 7, (HL)
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	set_bit(&value, 7);
	write_to_ram(cpu->interconnect, cpu->reg_hl, value);
	return PC_NO_JMP;
}
