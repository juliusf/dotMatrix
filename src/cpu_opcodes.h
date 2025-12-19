
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

int8_t opCode0x02(Cpu* cpu){ // LD (BC), A
	write_to_ram(cpu->interconnect, cpu->reg_bc, cpu->reg_a);
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

int8_t opCode0x0f(Cpu* cpu){ // RRCA
	// RRCA: Rotate A right, bit 0 -> C and bit 7
	uint8_t bit0 = cpu->reg_a & 0x01;
	cpu->reg_a = (cpu->reg_a >> 1) | (bit0 << 7);

	if (bit0) {
		set_bit(&(cpu->reg_f), FLAG_BIT_C);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_C);
	}

	// RRCA always clears Z, N, and H flags
	clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	clear_bit(&(cpu->reg_f), FLAG_BIT_N);
	clear_bit(&(cpu->reg_f), FLAG_BIT_H);
	return PC_NO_JMP;
}

int8_t opCode0x10(Cpu* cpu){ // STOP
	// STOP halts the CPU and screen until a button is pressed
	// For now, we'll treat it like HALT
	cpu->halted = 1;
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

int8_t opCode0x27(Cpu* cpu){ // DAA
	// DAA: Decimal Adjust Accumulator
	uint16_t result = cpu->reg_a;

	if (!get_bit(&(cpu->reg_f), FLAG_BIT_N)) {
		// After addition
		if (get_bit(&(cpu->reg_f), FLAG_BIT_C) || cpu->reg_a > 0x99) {
			result += 0x60;
			set_bit(&(cpu->reg_f), FLAG_BIT_C);
		}
		if (get_bit(&(cpu->reg_f), FLAG_BIT_H) || (cpu->reg_a & 0x0F) > 0x09) {
			result += 0x06;
		}
	} else {
		// After subtraction
		if (get_bit(&(cpu->reg_f), FLAG_BIT_C)) {
			result -= 0x60;
		}
		if (get_bit(&(cpu->reg_f), FLAG_BIT_H)) {
			result -= 0x06;
		}
	}

	cpu->reg_a = (uint8_t)result;

	// Set Z flag if result is 0
	if (cpu->reg_a == 0) {
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	// Clear H flag
	clear_bit(&(cpu->reg_f), FLAG_BIT_H);

	return PC_NO_JMP;
}

int8_t opCode0x2d(Cpu* cpu){ // DEC L
	cpu->reg_l--;
	cpu_dec_toggle_bits(cpu, &(cpu->reg_l));
	return PC_NO_JMP;
}

int8_t opCode0x2e(Cpu* cpu){ // LD L, n
	uint8_t value = get_one_byte_parameter(cpu);
	cpu->reg_l = value;
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

int8_t opCode0x33(Cpu* cpu){ // INC SP
	cpu->reg_sp++;
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

int8_t opCode0x37(Cpu* cpu){ // SCF
	// SCF: Set Carry Flag
	// N=0, H=0, C=1, Z unchanged
	set_bit(&(cpu->reg_f), FLAG_BIT_C);
	clear_bit(&(cpu->reg_f), FLAG_BIT_N);
	clear_bit(&(cpu->reg_f), FLAG_BIT_H);
	return PC_NO_JMP;
}

int8_t opCode0x39(Cpu* cpu){ // ADD HL, SP
	opcodes_add_hl(cpu, cpu->reg_sp);
	return PC_NO_JMP;
}

int8_t opCode0x3a(Cpu* cpu){ // LDD A, (HL)
	cpu->reg_a = read_from_ram(cpu->interconnect, cpu->reg_hl);
	cpu->reg_hl--;
	return PC_NO_JMP;
}

int8_t opCode0x3b(Cpu* cpu){ // DEC SP
	cpu->reg_sp--;
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

int8_t opCode0x3f(Cpu* cpu){ // CCF
	// CCF: Complement Carry Flag
	// N=0, H=0, C=!C, Z unchanged
	if (get_bit(&(cpu->reg_f), FLAG_BIT_C)) {
		clear_bit(&(cpu->reg_f), FLAG_BIT_C);
	} else {
		set_bit(&(cpu->reg_f), FLAG_BIT_C);
	}
	clear_bit(&(cpu->reg_f), FLAG_BIT_N);
	clear_bit(&(cpu->reg_f), FLAG_BIT_H);
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

int8_t opCode0x74(Cpu* cpu){ // LD (HL), H
	write_to_ram(cpu->interconnect, cpu->reg_hl, cpu->reg_h);
	return PC_NO_JMP;
}

int8_t opCode0x75(Cpu* cpu){ // LD (HL), L
	write_to_ram(cpu->interconnect, cpu->reg_hl, cpu->reg_l);
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

int8_t opCode0x7f(Cpu* cpu){ // LD A, A
	cpu->reg_a = cpu->reg_a;
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

int8_t opCode0x8a(Cpu* cpu){ // ADC A, D
	opcodes_adc(cpu, cpu->reg_d);
	return PC_NO_JMP;
}

int8_t opCode0x8b(Cpu* cpu){ // ADC A, E
	opcodes_adc(cpu, cpu->reg_e);
	return PC_NO_JMP;
}

int8_t opCode0x8c(Cpu* cpu){ // ADC A, H
	opcodes_adc(cpu, cpu->reg_h);
	return PC_NO_JMP;
}

int8_t opCode0x8d(Cpu* cpu){ // ADC A, L
	opcodes_adc(cpu, cpu->reg_l);
	return PC_NO_JMP;
}

int8_t opCode0x8e(Cpu* cpu){ // ADC A, (HL)
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	opcodes_adc(cpu, value);
	return PC_NO_JMP;
}

int8_t opCode0x8f(Cpu* cpu){ // ADC A, A
	opcodes_adc(cpu, cpu->reg_a);
	return PC_NO_JMP;
}

int8_t opCode0x90(Cpu* cpu){ // SUB B
	opcodes_sub(cpu, cpu->reg_b);
	return PC_NO_JMP;
}

int8_t opCode0x91(Cpu* cpu){ // SUB C
	opcodes_sub(cpu, cpu->reg_c);
	return PC_NO_JMP;
}

int8_t opCode0x92(Cpu* cpu){ // SUB D
	opcodes_sub(cpu, cpu->reg_d);
	return PC_NO_JMP;
}

int8_t opCode0x93(Cpu* cpu){ // SUB E
	opcodes_sub(cpu, cpu->reg_e);
	return PC_NO_JMP;
}

int8_t opCode0x94(Cpu* cpu){ // SUB H
	opcodes_sub(cpu, cpu->reg_h);
	return PC_NO_JMP;
}

int8_t opCode0x95(Cpu* cpu){ // SUB L
	opcodes_sub(cpu, cpu->reg_l);
	return PC_NO_JMP;
}

int8_t opCode0x96(Cpu* cpu){ // SUB (HL)
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	opcodes_sub(cpu, value);
	return PC_NO_JMP;
}

int8_t opCode0x97(Cpu* cpu){ // SUB A
	opcodes_sub(cpu, cpu->reg_a);
	return PC_NO_JMP;
}

int8_t opCode0x98(Cpu* cpu){ // SBC A, B
	opcodes_sbc(cpu, cpu->reg_b);
	return PC_NO_JMP;
}

int8_t opCode0x99(Cpu* cpu){ // SBC A, C
	opcodes_sbc(cpu, cpu->reg_c);
	return PC_NO_JMP;
}

int8_t opCode0x9a(Cpu* cpu){ // SBC A, D
	opcodes_sbc(cpu, cpu->reg_d);
	return PC_NO_JMP;
}

int8_t opCode0x9b(Cpu* cpu){ // SBC A, E
	opcodes_sbc(cpu, cpu->reg_e);
	return PC_NO_JMP;
}

int8_t opCode0x9c(Cpu* cpu){ // SBC A, H
	opcodes_sbc(cpu, cpu->reg_h);
	return PC_NO_JMP;
}

int8_t opCode0x9d(Cpu* cpu){ // SBC A, L
	opcodes_sbc(cpu, cpu->reg_l);
	return PC_NO_JMP;
}

int8_t opCode0x9e(Cpu* cpu){ // SBC A, (HL)
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	opcodes_sbc(cpu, value);
	return PC_NO_JMP;
}

int8_t opCode0x9f(Cpu* cpu){ // SBC A, A
	opcodes_sbc(cpu, cpu->reg_a);
	return PC_NO_JMP;
}

int8_t opCode0xa0(Cpu* cpu){ // AND B
	opcodes_and(cpu, cpu->reg_b);
	return PC_NO_JMP;
}

int8_t opCode0xa1(Cpu* cpu){ // AND C
	opcodes_and(cpu, cpu->reg_c);
	return PC_NO_JMP;
}

int8_t opCode0xa2(Cpu* cpu){ // AND D
	opcodes_and(cpu, cpu->reg_d);
	return PC_NO_JMP;
}

int8_t opCode0xa3(Cpu* cpu){ // AND E
	opcodes_and(cpu, cpu->reg_e);
	return PC_NO_JMP;
}

int8_t opCode0xa4(Cpu* cpu){ // AND H
	opcodes_and(cpu, cpu->reg_h);
	return PC_NO_JMP;
}

int8_t opCode0xa5(Cpu* cpu){ // AND L
	opcodes_and(cpu, cpu->reg_l);
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

int8_t opCode0xa8(Cpu* cpu){ // XOR B
	opcodes_xor(cpu, cpu->reg_b);
	return PC_NO_JMP;
}

int8_t opCode0xa9(Cpu* cpu){ // XOR C
	opcodes_xor(cpu, cpu->reg_c);
	return PC_NO_JMP;
}

int8_t opCode0xaa(Cpu* cpu){ // XOR D
	opcodes_xor(cpu, cpu->reg_d);
	return PC_NO_JMP;
}

int8_t opCode0xab(Cpu* cpu){ // XOR E
	opcodes_xor(cpu, cpu->reg_e);
	return PC_NO_JMP;
}

int8_t opCode0xac(Cpu* cpu){ // XOR H
	opcodes_xor(cpu, cpu->reg_h);
	return PC_NO_JMP;
}

int8_t opCode0xad(Cpu* cpu){ // XOR L
	opcodes_xor(cpu, cpu->reg_l);
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

int8_t opCode0xb2(Cpu* cpu){ // OR D
	opcodes_or(cpu, cpu->reg_d);
	return PC_NO_JMP;
}

int8_t opCode0xb3(Cpu* cpu){ // OR E
	opcodes_or(cpu, cpu->reg_e);
	return PC_NO_JMP;
}

int8_t opCode0xb4(Cpu* cpu){ // OR H
	opcodes_or(cpu, cpu->reg_h);
	return PC_NO_JMP;
}

int8_t opCode0xb5(Cpu* cpu){ // OR L
	opcodes_or(cpu, cpu->reg_l);
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

int8_t opCode0xb8(Cpu* cpu){ // CP B
	opcodes_cp(cpu, cpu->reg_b);
	return PC_NO_JMP;
}

int8_t opCode0xb9(Cpu* cpu){ // CP C
	opcodes_cp(cpu, cpu->reg_c);
	return PC_NO_JMP;
}

int8_t opCode0xba(Cpu* cpu){ // CP D
	opcodes_cp(cpu, cpu->reg_d);
	return PC_NO_JMP;
}

int8_t opCode0xbb(Cpu* cpu){ // CP E
	opcodes_cp(cpu, cpu->reg_e);
	return PC_NO_JMP;
}

int8_t opCode0xbc(Cpu* cpu){ // CP H
	opcodes_cp(cpu, cpu->reg_h);
	return PC_NO_JMP;
}

int8_t opCode0xbd(Cpu* cpu){ // CP L
	opcodes_cp(cpu, cpu->reg_l);
	return PC_NO_JMP;
}

int8_t opCode0xbe(Cpu* cpu){ // CP (HL)
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	opcodes_cp(cpu, value);
	return PC_NO_JMP;
}

int8_t opCode0xbf(Cpu* cpu){ // CP A
	opcodes_cp(cpu, cpu->reg_a);
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

int8_t opCode0xcc(Cpu* cpu){ // CALL Z, nn
	uint16_t addr = get_two_byte_parameter(cpu);
	if (get_bit(&(cpu->reg_f), FLAG_BIT_Z)){
		push_stack(cpu, cpu->reg_pc + 3);
		cpu->reg_pc = addr;
		cpu->cycles_left = 6; // Branch taken: 24 T-cycles = 6 M-cycles
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

int8_t opCode0xcf(Cpu* cpu){ // RST 08H
	push_stack(cpu, cpu->reg_pc + 1);
	cpu->reg_pc = 0x0008;
	return PC_JMP;
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

int8_t opCode0xc7(Cpu* cpu){ // RST 00H
	push_stack(cpu, cpu->reg_pc + 1);
	cpu->reg_pc = 0x0000;
	return PC_JMP;
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

int8_t opCode0xd2(Cpu* cpu){ // JP NC, nn
	uint16_t addr = get_two_byte_parameter(cpu);
	if (!get_bit(&(cpu->reg_f), FLAG_BIT_C)){
		cpu->reg_pc = addr;
		cpu->cycles_left = 4; // Branch taken: 16 T-cycles = 4 M-cycles
		return PC_JMP;
	}
	cpu->cycles_left = 3; // Branch not taken: 12 T-cycles = 3 M-cycles
	return PC_NO_JMP;
}

int8_t opCode0xd4(Cpu* cpu){ // CALL NC, nn
	uint16_t addr = get_two_byte_parameter(cpu);
	if (!get_bit(&(cpu->reg_f), FLAG_BIT_C)){
		push_stack(cpu, cpu->reg_pc + 3);
		cpu->reg_pc = addr;
		cpu->cycles_left = 6; // Branch taken: 24 T-cycles = 6 M-cycles
		return PC_JMP;
	}
	cpu->cycles_left = 3; // Branch not taken: 12 T-cycles = 3 M-cycles
	return PC_NO_JMP;
}

int8_t opCode0xd6(Cpu* cpu){ // SUB n
	uint8_t value = get_one_byte_parameter(cpu);
	opcodes_sub(cpu, value);
	return PC_NO_JMP;
}

int8_t opCode0xd7(Cpu* cpu){ // RST 10H
	push_stack(cpu, cpu->reg_pc + 1);
	cpu->reg_pc = 0x0010;
	return PC_JMP;
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

int8_t opCode0xda(Cpu* cpu){ // JP C, nn
	uint16_t addr = get_two_byte_parameter(cpu);
	if (get_bit(&(cpu->reg_f), FLAG_BIT_C)){
		cpu->reg_pc = addr;
		cpu->cycles_left = 4; // Branch taken: 16 T-cycles = 4 M-cycles
		return PC_JMP;
	}
	cpu->cycles_left = 3; // Branch not taken: 12 T-cycles = 3 M-cycles
	return PC_NO_JMP;
}

int8_t opCode0xdc(Cpu* cpu){ // CALL C, nn
	uint16_t addr = get_two_byte_parameter(cpu);
	if (get_bit(&(cpu->reg_f), FLAG_BIT_C)){
		push_stack(cpu, cpu->reg_pc + 3);
		cpu->reg_pc = addr;
		cpu->cycles_left = 6; // Branch taken: 24 T-cycles = 6 M-cycles
		return PC_JMP;
	}
	cpu->cycles_left = 3; // Branch not taken: 12 T-cycles = 3 M-cycles
	return PC_NO_JMP;
}

int8_t opCode0xde(Cpu* cpu){ // SBC A, n
	uint8_t value = get_one_byte_parameter(cpu);
	opcodes_sbc(cpu, value);
	return PC_NO_JMP;
}

int8_t opCode0xdf(Cpu* cpu){ // RST 18H
	push_stack(cpu, cpu->reg_pc + 1);
	cpu->reg_pc = 0x0018;
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

int8_t opCode0xe7(Cpu* cpu){ // RST 20H
	push_stack(cpu, cpu->reg_pc + 1);
	cpu->reg_pc = 0x0020;
	return PC_JMP;
}

int8_t opCode0xe8(Cpu* cpu){ // ADD SP, n
	int8_t offset = (int8_t)get_one_byte_parameter(cpu);
	uint32_t result = cpu->reg_sp + offset;

	// Clear Z and N flags
	clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	clear_bit(&(cpu->reg_f), FLAG_BIT_N);

	// Set H flag if carry from bit 3
	if (((cpu->reg_sp & 0x0F) + (offset & 0x0F)) > 0x0F) {
		set_bit(&(cpu->reg_f), FLAG_BIT_H);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_H);
	}

	// Set C flag if carry from bit 7
	if (((cpu->reg_sp & 0xFF) + (offset & 0xFF)) > 0xFF) {
		set_bit(&(cpu->reg_f), FLAG_BIT_C);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_C);
	}

	cpu->reg_sp = (uint16_t)result;
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

int8_t opCode0xf7(Cpu* cpu){ // RST 30H
	push_stack(cpu, cpu->reg_pc + 1);
	cpu->reg_pc = 0x0030;
	return PC_JMP;
}

int8_t opCode0xf8(Cpu* cpu){ // LD HL, SP+n
	int8_t offset = (int8_t)get_one_byte_parameter(cpu);
	uint32_t result = cpu->reg_sp + offset;

	// Clear Z and N flags
	clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	clear_bit(&(cpu->reg_f), FLAG_BIT_N);

	// Set H flag if carry from bit 3
	if (((cpu->reg_sp & 0x0F) + (offset & 0x0F)) > 0x0F) {
		set_bit(&(cpu->reg_f), FLAG_BIT_H);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_H);
	}

	// Set C flag if carry from bit 7
	if (((cpu->reg_sp & 0xFF) + (offset & 0xFF)) > 0xFF) {
		set_bit(&(cpu->reg_f), FLAG_BIT_C);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_C);
	}

	cpu->reg_hl = (uint16_t)result;
	return PC_NO_JMP;
}

int8_t opCode0xf9(Cpu* cpu){ // LD SP, HL
	cpu->reg_sp = cpu->reg_hl;
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

// RLC instructions (0x00-0x07)
int8_t opCode0xcb00(Cpu* cpu){ opcodes_rlc(cpu, &(cpu->reg_b)); return PC_NO_JMP; }
int8_t opCode0xcb01(Cpu* cpu){ opcodes_rlc(cpu, &(cpu->reg_c)); return PC_NO_JMP; }
int8_t opCode0xcb02(Cpu* cpu){ opcodes_rlc(cpu, &(cpu->reg_d)); return PC_NO_JMP; }
int8_t opCode0xcb03(Cpu* cpu){ opcodes_rlc(cpu, &(cpu->reg_e)); return PC_NO_JMP; }
int8_t opCode0xcb04(Cpu* cpu){ opcodes_rlc(cpu, &(cpu->reg_h)); return PC_NO_JMP; }
int8_t opCode0xcb05(Cpu* cpu){ opcodes_rlc(cpu, &(cpu->reg_l)); return PC_NO_JMP; }
int8_t opCode0xcb06(Cpu* cpu){
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	opcodes_rlc(cpu, &value);
	write_to_ram(cpu->interconnect, cpu->reg_hl, value);
	return PC_NO_JMP;
}
int8_t opCode0xcb07(Cpu* cpu){ opcodes_rlc(cpu, &(cpu->reg_a)); return PC_NO_JMP; }

// RRC instructions (0x08-0x0F)
int8_t opCode0xcb08(Cpu* cpu){ opcodes_rrc(cpu, &(cpu->reg_b)); return PC_NO_JMP; }
int8_t opCode0xcb09(Cpu* cpu){ opcodes_rrc(cpu, &(cpu->reg_c)); return PC_NO_JMP; }
int8_t opCode0xcb0a(Cpu* cpu){ opcodes_rrc(cpu, &(cpu->reg_d)); return PC_NO_JMP; }
int8_t opCode0xcb0b(Cpu* cpu){ opcodes_rrc(cpu, &(cpu->reg_e)); return PC_NO_JMP; }
int8_t opCode0xcb0c(Cpu* cpu){ opcodes_rrc(cpu, &(cpu->reg_h)); return PC_NO_JMP; }
int8_t opCode0xcb0d(Cpu* cpu){ opcodes_rrc(cpu, &(cpu->reg_l)); return PC_NO_JMP; }
int8_t opCode0xcb0e(Cpu* cpu){
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	opcodes_rrc(cpu, &value);
	write_to_ram(cpu->interconnect, cpu->reg_hl, value);
	return PC_NO_JMP;
}
int8_t opCode0xcb0f(Cpu* cpu){ opcodes_rrc(cpu, &(cpu->reg_a)); return PC_NO_JMP; }

// RL instructions (0x10-0x17)
int8_t opCode0xcb10(Cpu* cpu){ opcodes_rl(cpu, &(cpu->reg_b)); return PC_NO_JMP; }
int8_t opCode0xcb13(Cpu* cpu){ opcodes_rl(cpu, &(cpu->reg_e)); return PC_NO_JMP; }
int8_t opCode0xcb14(Cpu* cpu){ opcodes_rl(cpu, &(cpu->reg_h)); return PC_NO_JMP; }
int8_t opCode0xcb15(Cpu* cpu){ opcodes_rl(cpu, &(cpu->reg_l)); return PC_NO_JMP; }
int8_t opCode0xcb16(Cpu* cpu){
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	opcodes_rl(cpu, &value);
	write_to_ram(cpu->interconnect, cpu->reg_hl, value);
	return PC_NO_JMP;
}
int8_t opCode0xcb17(Cpu* cpu){ opcodes_rl(cpu, &(cpu->reg_a)); return PC_NO_JMP; }

// RR instructions (0x18-0x1F)
int8_t opCode0xcb18(Cpu* cpu){ opcodes_rr(cpu, &(cpu->reg_b)); return PC_NO_JMP; }
int8_t opCode0xcb1b(Cpu* cpu){ opcodes_rr(cpu, &(cpu->reg_e)); return PC_NO_JMP; }
int8_t opCode0xcb1c(Cpu* cpu){ opcodes_rr(cpu, &(cpu->reg_h)); return PC_NO_JMP; }
int8_t opCode0xcb1d(Cpu* cpu){ opcodes_rr(cpu, &(cpu->reg_l)); return PC_NO_JMP; }
int8_t opCode0xcb1e(Cpu* cpu){
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	opcodes_rr(cpu, &value);
	write_to_ram(cpu->interconnect, cpu->reg_hl, value);
	return PC_NO_JMP;
}
int8_t opCode0xcb1f(Cpu* cpu){ opcodes_rr(cpu, &(cpu->reg_a)); return PC_NO_JMP; }

// SLA instructions (0x20-0x27)
int8_t opCode0xcb20(Cpu* cpu){ opcodes_sla(cpu, &(cpu->reg_b)); return PC_NO_JMP; }
int8_t opCode0xcb21(Cpu* cpu){ opcodes_sla(cpu, &(cpu->reg_c)); return PC_NO_JMP; }
int8_t opCode0xcb22(Cpu* cpu){ opcodes_sla(cpu, &(cpu->reg_d)); return PC_NO_JMP; }
int8_t opCode0xcb23(Cpu* cpu){ opcodes_sla(cpu, &(cpu->reg_e)); return PC_NO_JMP; }
int8_t opCode0xcb24(Cpu* cpu){ opcodes_sla(cpu, &(cpu->reg_h)); return PC_NO_JMP; }
int8_t opCode0xcb25(Cpu* cpu){ opcodes_sla(cpu, &(cpu->reg_l)); return PC_NO_JMP; }
int8_t opCode0xcb26(Cpu* cpu){
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	opcodes_sla(cpu, &value);
	write_to_ram(cpu->interconnect, cpu->reg_hl, value);
	return PC_NO_JMP;
}

// SRA instructions (0x28-0x2F)
int8_t opCode0xcb28(Cpu* cpu){ opcodes_sra(cpu, &(cpu->reg_b)); return PC_NO_JMP; }
int8_t opCode0xcb29(Cpu* cpu){ opcodes_sra(cpu, &(cpu->reg_c)); return PC_NO_JMP; }
int8_t opCode0xcb2a(Cpu* cpu){ opcodes_sra(cpu, &(cpu->reg_d)); return PC_NO_JMP; }
int8_t opCode0xcb2b(Cpu* cpu){ opcodes_sra(cpu, &(cpu->reg_e)); return PC_NO_JMP; }
int8_t opCode0xcb2c(Cpu* cpu){ opcodes_sra(cpu, &(cpu->reg_h)); return PC_NO_JMP; }
int8_t opCode0xcb2d(Cpu* cpu){ opcodes_sra(cpu, &(cpu->reg_l)); return PC_NO_JMP; }
int8_t opCode0xcb2e(Cpu* cpu){
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	opcodes_sra(cpu, &value);
	write_to_ram(cpu->interconnect, cpu->reg_hl, value);
	return PC_NO_JMP;
}
int8_t opCode0xcb2f(Cpu* cpu){ opcodes_sra(cpu, &(cpu->reg_a)); return PC_NO_JMP; }

// SWAP instructions (0x30-0x37)
int8_t opCode0xcb30(Cpu* cpu){ opcodes_swap(cpu, &(cpu->reg_b)); return PC_NO_JMP; }
int8_t opCode0xcb31(Cpu* cpu){ opcodes_swap(cpu, &(cpu->reg_c)); return PC_NO_JMP; }
int8_t opCode0xcb32(Cpu* cpu){ opcodes_swap(cpu, &(cpu->reg_d)); return PC_NO_JMP; }
int8_t opCode0xcb34(Cpu* cpu){ opcodes_swap(cpu, &(cpu->reg_h)); return PC_NO_JMP; }
int8_t opCode0xcb35(Cpu* cpu){ opcodes_swap(cpu, &(cpu->reg_l)); return PC_NO_JMP; }
int8_t opCode0xcb36(Cpu* cpu){
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	opcodes_swap(cpu, &value);
	write_to_ram(cpu->interconnect, cpu->reg_hl, value);
	return PC_NO_JMP;
}

// SRL instructions (0x38-0x3F)
int8_t opCode0xcb39(Cpu* cpu){ opcodes_srl(cpu, &(cpu->reg_c)); return PC_NO_JMP; }
int8_t opCode0xcb3a(Cpu* cpu){ opcodes_srl(cpu, &(cpu->reg_d)); return PC_NO_JMP; }
int8_t opCode0xcb3b(Cpu* cpu){ opcodes_srl(cpu, &(cpu->reg_e)); return PC_NO_JMP; }
int8_t opCode0xcb3c(Cpu* cpu){ opcodes_srl(cpu, &(cpu->reg_h)); return PC_NO_JMP; }
int8_t opCode0xcb3d(Cpu* cpu){ opcodes_srl(cpu, &(cpu->reg_l)); return PC_NO_JMP; }
int8_t opCode0xcb3e(Cpu* cpu){
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl);
	opcodes_srl(cpu, &value);
	write_to_ram(cpu->interconnect, cpu->reg_hl, value);
	return PC_NO_JMP;
}

// BIT instructions (0x40-0x7F) - Test bit b in register r
// Helper macro for BIT instruction
#define BIT_INSTR(bit, reg) \
	if (!get_bit(&(cpu->reg), bit)) { \
		set_bit(&(cpu->reg_f), FLAG_BIT_Z); \
	} else { \
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z); \
	} \
	clear_bit(&(cpu->reg_f), FLAG_BIT_N); \
	set_bit(&(cpu->reg_f), FLAG_BIT_H); \
	return PC_NO_JMP;

#define BIT_INSTR_HL(bit) \
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl); \
	if (!get_bit(&value, bit)) { \
		set_bit(&(cpu->reg_f), FLAG_BIT_Z); \
	} else { \
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z); \
	} \
	clear_bit(&(cpu->reg_f), FLAG_BIT_N); \
	set_bit(&(cpu->reg_f), FLAG_BIT_H); \
	return PC_NO_JMP;

// BIT 0,r (0x40-0x47)
int8_t opCode0xcb41(Cpu* cpu){ BIT_INSTR(0, reg_c) }
int8_t opCode0xcb42(Cpu* cpu){ BIT_INSTR(0, reg_d) }
int8_t opCode0xcb43(Cpu* cpu){ BIT_INSTR(0, reg_e) }
int8_t opCode0xcb44(Cpu* cpu){ BIT_INSTR(0, reg_h) }
int8_t opCode0xcb45(Cpu* cpu){ BIT_INSTR(0, reg_l) }
int8_t opCode0xcb46(Cpu* cpu){ BIT_INSTR_HL(0) }
int8_t opCode0xcb47(Cpu* cpu){ BIT_INSTR(0, reg_a) }

// BIT 1,r (0x48-0x4F)
int8_t opCode0xcb48(Cpu* cpu){ BIT_INSTR(1, reg_b) }
int8_t opCode0xcb49(Cpu* cpu){ BIT_INSTR(1, reg_c) }
int8_t opCode0xcb4a(Cpu* cpu){ BIT_INSTR(1, reg_d) }
int8_t opCode0xcb4b(Cpu* cpu){ BIT_INSTR(1, reg_e) }
int8_t opCode0xcb4d(Cpu* cpu){ BIT_INSTR(1, reg_l) }
int8_t opCode0xcb4e(Cpu* cpu){ BIT_INSTR_HL(1) }

// BIT 2,r (0x50-0x57)
int8_t opCode0xcb51(Cpu* cpu){ BIT_INSTR(2, reg_c) }
int8_t opCode0xcb52(Cpu* cpu){ BIT_INSTR(2, reg_d) }
int8_t opCode0xcb53(Cpu* cpu){ BIT_INSTR(2, reg_e) }
int8_t opCode0xcb54(Cpu* cpu){ BIT_INSTR(2, reg_h) }
int8_t opCode0xcb55(Cpu* cpu){ BIT_INSTR(2, reg_l) }
int8_t opCode0xcb56(Cpu* cpu){ BIT_INSTR_HL(2) }
int8_t opCode0xcb57(Cpu* cpu){ BIT_INSTR(2, reg_a) }

// BIT 3,r (0x58-0x5F)
int8_t opCode0xcb59(Cpu* cpu){ BIT_INSTR(3, reg_c) }
int8_t opCode0xcb5a(Cpu* cpu){ BIT_INSTR(3, reg_d) }
int8_t opCode0xcb5b(Cpu* cpu){ BIT_INSTR(3, reg_e) }
int8_t opCode0xcb5c(Cpu* cpu){ BIT_INSTR(3, reg_h) }
int8_t opCode0xcb5d(Cpu* cpu){ BIT_INSTR(3, reg_l) }
int8_t opCode0xcb5e(Cpu* cpu){ BIT_INSTR_HL(3) }

// BIT 4,r (0x60-0x67)
int8_t opCode0xcb61(Cpu* cpu){ BIT_INSTR(4, reg_c) }
int8_t opCode0xcb62(Cpu* cpu){ BIT_INSTR(4, reg_d) }
int8_t opCode0xcb63(Cpu* cpu){ BIT_INSTR(4, reg_e) }
int8_t opCode0xcb64(Cpu* cpu){ BIT_INSTR(4, reg_h) }
int8_t opCode0xcb65(Cpu* cpu){ BIT_INSTR(4, reg_l) }
int8_t opCode0xcb66(Cpu* cpu){ BIT_INSTR_HL(4) }
int8_t opCode0xcb67(Cpu* cpu){ BIT_INSTR(4, reg_a) }

// BIT 5,r (0x68-0x6F)
int8_t opCode0xcb69(Cpu* cpu){ BIT_INSTR(5, reg_c) }
int8_t opCode0xcb6a(Cpu* cpu){ BIT_INSTR(5, reg_d) }
int8_t opCode0xcb6b(Cpu* cpu){ BIT_INSTR(5, reg_e) }
int8_t opCode0xcb6c(Cpu* cpu){ BIT_INSTR(5, reg_h) }
int8_t opCode0xcb6d(Cpu* cpu){ BIT_INSTR(5, reg_l) }
int8_t opCode0xcb6e(Cpu* cpu){ BIT_INSTR_HL(5) }

// BIT 6,r (0x70-0x77)
int8_t opCode0xcb71(Cpu* cpu){ BIT_INSTR(6, reg_c) }
int8_t opCode0xcb72(Cpu* cpu){ BIT_INSTR(6, reg_d) }
int8_t opCode0xcb73(Cpu* cpu){ BIT_INSTR(6, reg_e) }
int8_t opCode0xcb74(Cpu* cpu){ BIT_INSTR(6, reg_h) }
int8_t opCode0xcb75(Cpu* cpu){ BIT_INSTR(6, reg_l) }
int8_t opCode0xcb76(Cpu* cpu){ BIT_INSTR_HL(6) }

// BIT 7,r (0x78-0x7F)
int8_t opCode0xcb79(Cpu* cpu){ BIT_INSTR(7, reg_c) }
int8_t opCode0xcb7a(Cpu* cpu){ BIT_INSTR(7, reg_d) }
int8_t opCode0xcb7b(Cpu* cpu){ BIT_INSTR(7, reg_e) }
int8_t opCode0xcb7d(Cpu* cpu){ BIT_INSTR(7, reg_l) }

// RES instructions (0x80-0xBF) - Reset bit b in register r
#define RES_INSTR(bit, reg) \
	clear_bit(&(cpu->reg), bit); \
	return PC_NO_JMP;

#define RES_INSTR_HL(bit) \
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl); \
	clear_bit(&value, bit); \
	write_to_ram(cpu->interconnect, cpu->reg_hl, value); \
	return PC_NO_JMP;

// RES 0,r (0x80-0x87)
int8_t opCode0xcb80(Cpu* cpu){ RES_INSTR(0, reg_b) }
int8_t opCode0xcb81(Cpu* cpu){ RES_INSTR(0, reg_c) }
int8_t opCode0xcb82(Cpu* cpu){ RES_INSTR(0, reg_d) }
int8_t opCode0xcb83(Cpu* cpu){ RES_INSTR(0, reg_e) }
int8_t opCode0xcb84(Cpu* cpu){ RES_INSTR(0, reg_h) }
int8_t opCode0xcb85(Cpu* cpu){ RES_INSTR(0, reg_l) }

// RES 1,r (0x88-0x8F)
int8_t opCode0xcb88(Cpu* cpu){ RES_INSTR(1, reg_b) }
int8_t opCode0xcb89(Cpu* cpu){ RES_INSTR(1, reg_c) }
int8_t opCode0xcb8a(Cpu* cpu){ RES_INSTR(1, reg_d) }
int8_t opCode0xcb8b(Cpu* cpu){ RES_INSTR(1, reg_e) }
int8_t opCode0xcb8c(Cpu* cpu){ RES_INSTR(1, reg_h) }
int8_t opCode0xcb8d(Cpu* cpu){ RES_INSTR(1, reg_l) }
int8_t opCode0xcb8e(Cpu* cpu){ RES_INSTR_HL(1) }
int8_t opCode0xcb8f(Cpu* cpu){ RES_INSTR(1, reg_a) }

// RES 2,r (0x90-0x97)
int8_t opCode0xcb90(Cpu* cpu){ RES_INSTR(2, reg_b) }
int8_t opCode0xcb91(Cpu* cpu){ RES_INSTR(2, reg_c) }
int8_t opCode0xcb92(Cpu* cpu){ RES_INSTR(2, reg_d) }
int8_t opCode0xcb93(Cpu* cpu){ RES_INSTR(2, reg_e) }
int8_t opCode0xcb94(Cpu* cpu){ RES_INSTR(2, reg_h) }
int8_t opCode0xcb95(Cpu* cpu){ RES_INSTR(2, reg_l) }
int8_t opCode0xcb96(Cpu* cpu){ RES_INSTR_HL(2) }
int8_t opCode0xcb97(Cpu* cpu){ RES_INSTR(2, reg_a) }

// RES 3,r (0x98-0x9F)
int8_t opCode0xcb98(Cpu* cpu){ RES_INSTR(3, reg_b) }
int8_t opCode0xcb99(Cpu* cpu){ RES_INSTR(3, reg_c) }
int8_t opCode0xcb9a(Cpu* cpu){ RES_INSTR(3, reg_d) }
int8_t opCode0xcb9b(Cpu* cpu){ RES_INSTR(3, reg_e) }
int8_t opCode0xcb9c(Cpu* cpu){ RES_INSTR(3, reg_h) }
int8_t opCode0xcb9d(Cpu* cpu){ RES_INSTR(3, reg_l) }
int8_t opCode0xcb9e(Cpu* cpu){ RES_INSTR_HL(3) }
int8_t opCode0xcb9f(Cpu* cpu){ RES_INSTR(3, reg_a) }

// RES 4,r (0xA0-0xA7)
int8_t opCode0xcba0(Cpu* cpu){ RES_INSTR(4, reg_b) }
int8_t opCode0xcba1(Cpu* cpu){ RES_INSTR(4, reg_c) }
int8_t opCode0xcba2(Cpu* cpu){ RES_INSTR(4, reg_d) }
int8_t opCode0xcba3(Cpu* cpu){ RES_INSTR(4, reg_e) }
int8_t opCode0xcba4(Cpu* cpu){ RES_INSTR(4, reg_h) }
int8_t opCode0xcba5(Cpu* cpu){ RES_INSTR(4, reg_l) }
int8_t opCode0xcba6(Cpu* cpu){ RES_INSTR_HL(4) }
int8_t opCode0xcba7(Cpu* cpu){ RES_INSTR(4, reg_a) }

// RES 5,r (0xA8-0xAF)
int8_t opCode0xcba8(Cpu* cpu){ RES_INSTR(5, reg_b) }
int8_t opCode0xcba9(Cpu* cpu){ RES_INSTR(5, reg_c) }
int8_t opCode0xcbaa(Cpu* cpu){ RES_INSTR(5, reg_d) }
int8_t opCode0xcbab(Cpu* cpu){ RES_INSTR(5, reg_e) }
int8_t opCode0xcbac(Cpu* cpu){ RES_INSTR(5, reg_h) }
int8_t opCode0xcbad(Cpu* cpu){ RES_INSTR(5, reg_l) }
int8_t opCode0xcbae(Cpu* cpu){ RES_INSTR_HL(5) }
int8_t opCode0xcbaf(Cpu* cpu){ RES_INSTR(5, reg_a) }

// RES 6,r (0xB0-0xB7)
int8_t opCode0xcbb0(Cpu* cpu){ RES_INSTR(6, reg_b) }
int8_t opCode0xcbb1(Cpu* cpu){ RES_INSTR(6, reg_c) }
int8_t opCode0xcbb2(Cpu* cpu){ RES_INSTR(6, reg_d) }
int8_t opCode0xcbb3(Cpu* cpu){ RES_INSTR(6, reg_e) }
int8_t opCode0xcbb4(Cpu* cpu){ RES_INSTR(6, reg_h) }
int8_t opCode0xcbb5(Cpu* cpu){ RES_INSTR(6, reg_l) }
int8_t opCode0xcbb6(Cpu* cpu){ RES_INSTR_HL(6) }
int8_t opCode0xcbb7(Cpu* cpu){ RES_INSTR(6, reg_a) }

// RES 7,r (0xB8-0xBF)
int8_t opCode0xcbb8(Cpu* cpu){ RES_INSTR(7, reg_b) }
int8_t opCode0xcbb9(Cpu* cpu){ RES_INSTR(7, reg_c) }
int8_t opCode0xcbba(Cpu* cpu){ RES_INSTR(7, reg_d) }
int8_t opCode0xcbbb(Cpu* cpu){ RES_INSTR(7, reg_e) }
int8_t opCode0xcbbc(Cpu* cpu){ RES_INSTR(7, reg_h) }
int8_t opCode0xcbbd(Cpu* cpu){ RES_INSTR(7, reg_l) }
int8_t opCode0xcbbe(Cpu* cpu){ RES_INSTR_HL(7) }
int8_t opCode0xcbbf(Cpu* cpu){ RES_INSTR(7, reg_a) }

// SET instructions (0xC0-0xFF) - Set bit b in register r
#define SET_INSTR(bit, reg) \
	set_bit(&(cpu->reg), bit); \
	return PC_NO_JMP;

#define SET_INSTR_HL(bit) \
	uint8_t value = read_from_ram(cpu->interconnect, cpu->reg_hl); \
	set_bit(&value, bit); \
	write_to_ram(cpu->interconnect, cpu->reg_hl, value); \
	return PC_NO_JMP;

// SET 0,r (0xC0-0xC7)
int8_t opCode0xcbc0(Cpu* cpu){ SET_INSTR(0, reg_b) }
int8_t opCode0xcbc1(Cpu* cpu){ SET_INSTR(0, reg_c) }
int8_t opCode0xcbc2(Cpu* cpu){ SET_INSTR(0, reg_d) }
int8_t opCode0xcbc3(Cpu* cpu){ SET_INSTR(0, reg_e) }
int8_t opCode0xcbc4(Cpu* cpu){ SET_INSTR(0, reg_h) }
int8_t opCode0xcbc5(Cpu* cpu){ SET_INSTR(0, reg_l) }
int8_t opCode0xcbc6(Cpu* cpu){ SET_INSTR_HL(0) }
int8_t opCode0xcbc7(Cpu* cpu){ SET_INSTR(0, reg_a) }

// SET 1,r (0xC8-0xCF)
int8_t opCode0xcbc8(Cpu* cpu){ SET_INSTR(1, reg_b) }
int8_t opCode0xcbc9(Cpu* cpu){ SET_INSTR(1, reg_c) }
int8_t opCode0xcbca(Cpu* cpu){ SET_INSTR(1, reg_d) }
int8_t opCode0xcbcb(Cpu* cpu){ SET_INSTR(1, reg_e) }
int8_t opCode0xcbcc(Cpu* cpu){ SET_INSTR(1, reg_h) }
int8_t opCode0xcbcd(Cpu* cpu){ SET_INSTR(1, reg_l) }
int8_t opCode0xcbce(Cpu* cpu){ SET_INSTR_HL(1) }
int8_t opCode0xcbcf(Cpu* cpu){ SET_INSTR(1, reg_a) }

// SET 2,r (0xD0-0xD7)
int8_t opCode0xcbd0(Cpu* cpu){ SET_INSTR(2, reg_b) }
int8_t opCode0xcbd1(Cpu* cpu){ SET_INSTR(2, reg_c) }
int8_t opCode0xcbd2(Cpu* cpu){ SET_INSTR(2, reg_d) }
int8_t opCode0xcbd3(Cpu* cpu){ SET_INSTR(2, reg_e) }
int8_t opCode0xcbd4(Cpu* cpu){ SET_INSTR(2, reg_h) }
int8_t opCode0xcbd5(Cpu* cpu){ SET_INSTR(2, reg_l) }
int8_t opCode0xcbd6(Cpu* cpu){ SET_INSTR_HL(2) }
int8_t opCode0xcbd7(Cpu* cpu){ SET_INSTR(2, reg_a) }

// SET 3,r (0xD8-0xDF)
int8_t opCode0xcbd8(Cpu* cpu){ SET_INSTR(3, reg_b) }
int8_t opCode0xcbd9(Cpu* cpu){ SET_INSTR(3, reg_c) }
int8_t opCode0xcbda(Cpu* cpu){ SET_INSTR(3, reg_d) }
int8_t opCode0xcbdb(Cpu* cpu){ SET_INSTR(3, reg_e) }
int8_t opCode0xcbdc(Cpu* cpu){ SET_INSTR(3, reg_h) }
int8_t opCode0xcbdd(Cpu* cpu){ SET_INSTR(3, reg_l) }
int8_t opCode0xcbde(Cpu* cpu){ SET_INSTR_HL(3) }
int8_t opCode0xcbdf(Cpu* cpu){ SET_INSTR(3, reg_a) }

// SET 4,r (0xE0-0xE7)
int8_t opCode0xcbe0(Cpu* cpu){ SET_INSTR(4, reg_b) }
int8_t opCode0xcbe1(Cpu* cpu){ SET_INSTR(4, reg_c) }
int8_t opCode0xcbe2(Cpu* cpu){ SET_INSTR(4, reg_d) }
int8_t opCode0xcbe3(Cpu* cpu){ SET_INSTR(4, reg_e) }
int8_t opCode0xcbe4(Cpu* cpu){ SET_INSTR(4, reg_h) }
int8_t opCode0xcbe5(Cpu* cpu){ SET_INSTR(4, reg_l) }
int8_t opCode0xcbe6(Cpu* cpu){ SET_INSTR_HL(4) }
int8_t opCode0xcbe7(Cpu* cpu){ SET_INSTR(4, reg_a) }

// SET 5,r (0xE8-0xEF)
int8_t opCode0xcbe8(Cpu* cpu){ SET_INSTR(5, reg_b) }
int8_t opCode0xcbe9(Cpu* cpu){ SET_INSTR(5, reg_c) }
int8_t opCode0xcbea(Cpu* cpu){ SET_INSTR(5, reg_d) }
int8_t opCode0xcbeb(Cpu* cpu){ SET_INSTR(5, reg_e) }
int8_t opCode0xcbec(Cpu* cpu){ SET_INSTR(5, reg_h) }
int8_t opCode0xcbed(Cpu* cpu){ SET_INSTR(5, reg_l) }
int8_t opCode0xcbee(Cpu* cpu){ SET_INSTR_HL(5) }
int8_t opCode0xcbef(Cpu* cpu){ SET_INSTR(5, reg_a) }

// SET 6,r (0xF0-0xF7)
int8_t opCode0xcbf0(Cpu* cpu){ SET_INSTR(6, reg_b) }
int8_t opCode0xcbf1(Cpu* cpu){ SET_INSTR(6, reg_c) }
int8_t opCode0xcbf2(Cpu* cpu){ SET_INSTR(6, reg_d) }
int8_t opCode0xcbf3(Cpu* cpu){ SET_INSTR(6, reg_e) }
int8_t opCode0xcbf4(Cpu* cpu){ SET_INSTR(6, reg_h) }
int8_t opCode0xcbf5(Cpu* cpu){ SET_INSTR(6, reg_l) }
int8_t opCode0xcbf6(Cpu* cpu){ SET_INSTR_HL(6) }
int8_t opCode0xcbf7(Cpu* cpu){ SET_INSTR(6, reg_a) }

// SET 7,r (0xF8-0xFF)
int8_t opCode0xcbf8(Cpu* cpu){ SET_INSTR(7, reg_b) }
int8_t opCode0xcbf9(Cpu* cpu){ SET_INSTR(7, reg_c) }
int8_t opCode0xcbfa(Cpu* cpu){ SET_INSTR(7, reg_d) }
int8_t opCode0xcbfb(Cpu* cpu){ SET_INSTR(7, reg_e) }
int8_t opCode0xcbfc(Cpu* cpu){ SET_INSTR(7, reg_h) }
int8_t opCode0xcbfd(Cpu* cpu){ SET_INSTR(7, reg_l) }
int8_t opCode0xcbff(Cpu* cpu){ SET_INSTR(7, reg_a) }
