
#ifndef CPU_INLINE_H
#define	CPU_INLINE_H

#include "interconnect.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

static inline uint16_t get_two_byte_parameter(Cpu* cpu){
	uint16_t addr = cpu->reg_pc + 1;
	return read_addr_from_ram(cpu->interconnect, addr);
}

static inline uint8_t get_one_byte_parameter(Cpu* cpu){
	return read_addr_from_ram(cpu->interconnect, cpu->reg_pc +1);
}


static inline void set_bit(uint8_t* x, int bit_num ){
	*x |= (1L << bit_num);
}

static inline int get_bit(uint8_t* x, int bit_num){
	return ((*x & (1<<(bit_num))));
}

static inline void clear_bit(uint8_t* x, int bit_num){
	*x &= ~(1 << bit_num);
}

static inline void pop_stack(Cpu* cpu, uint16_t* target_reg){

	*target_reg = read_addr_from_ram(cpu->interconnect, cpu->reg_sp);
	debug_print("popping from stack:: 0x%x\n", *target_reg);
	cpu->reg_sp++;
	cpu->reg_sp++;
}

static inline void push_stack(Cpu* cpu, uint16_t addr){
	debug_print("pushing to stack: 0x%x\n", addr);
	cpu->reg_sp--;
	cpu->reg_sp--;
	write_addr_to_ram(cpu->interconnect, cpu->reg_sp, addr);
}

static inline void clear_all_flags(Cpu* cpu){
	cpu->reg_f = FLAG_NONE;
}

static inline void toggle_flag(Cpu* cpu, uint8_t flag){
	if( get_bit(&(cpu->reg_f), flag) ){
		clear_bit( &(cpu->reg_f), flag);
	}else{
		set_bit( &(cpu->reg_f), flag);
	}
}

static inline void toggle_zero_flag(Cpu* cpu){
	toggle_flag(cpu, FLAG_BIT_Z);
}

static inline void toggle_zero_flag_from_result(Cpu* cpu, uint8_t result){
	if(! result){
		toggle_zero_flag(cpu);
	}
}

static inline uint8_t opcodes_rl_rotate(Cpu* cpu, uint8_t* reg){
	// RL: Rotate left through carry
	// Old bit 7 -> C flag, C flag -> bit 0

	uint8_t old_carry = get_bit(&(cpu->reg_f), FLAG_BIT_C) ? 1 : 0;
	uint8_t result = *reg;

	// Set C flag to old bit 7
	if ((result & 0x80) != 0) {
		set_bit(&(cpu->reg_f), FLAG_BIT_C);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_C);
	}

	// Rotate left and insert old carry at bit 0
	result <<= 1;
	result |= old_carry;
	*reg = result;

	// Clear N and H flags
	clear_bit(&(cpu->reg_f), FLAG_BIT_N);
	clear_bit(&(cpu->reg_f), FLAG_BIT_H);

	return result;
}

static inline void opcodes_rl(Cpu* cpu, uint8_t* reg){
	uint8_t result = opcodes_rl_rotate(cpu, reg);
	// Z flag handling differs between RLA and CB RL
	// RLA (0x17): Z=0 (always cleared)
	// CB RL r: Z=1 if result is 0, else Z=0
	if (reg == &(cpu->reg_a)){
		// RLA - always clear Z flag
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	} else {
		// CB prefixed RL - set Z flag based on result
		if (result == 0) {
			set_bit(&(cpu->reg_f), FLAG_BIT_Z);
		} else {
			clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
		}
	}
}

static inline void opcodes_cp(Cpu* cpu, uint8_t number){
	// CP: Compare A with number (A - number)
	// Z=1 if A==number, N=1, H=1 if borrow from bit 4, C=1 if A<number

	set_bit(&(cpu->reg_f), FLAG_BIT_N);

	// Set Z flag if A == number
	if (cpu->reg_a == number){
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	// Set C flag if A < number (borrow occurred)
	if (cpu->reg_a < number){
		set_bit(&(cpu->reg_f), FLAG_BIT_C);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_C);
	}

	// Set H flag if borrow from bit 4
	if ( ((cpu->reg_a - number) & 0xf) > (cpu->reg_a & 0xF)){
		set_bit(&(cpu->reg_f), FLAG_BIT_H);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_H);
	}
}

static inline void cpu_inc_toggle_bits(Cpu* cpu, uint8_t* reg){
	// INC: Z if result is 0, N=0, H if overflow from bit 3, C unchanged

	// Set Z flag if result is 0
	if (*reg == 0) {
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	// Clear N flag (INC always clears N)
	clear_bit(&(cpu->reg_f), FLAG_BIT_N);

	// Set H flag if lower nibble is 0 (overflow from bit 3)
	if ((*reg & 0x0F) == 0x00) {
		set_bit(&(cpu->reg_f), FLAG_BIT_H);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_H);
	}

	// C flag is unchanged (don't touch it)
}

static inline void cpu_dec_toggle_bits(Cpu* cpu, uint8_t* reg){
	// DEC: Z if result is 0, N=1, H if borrow from bit 4, C unchanged

	// Set Z flag if result is 0
	if (*reg == 0) {
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	// Set N flag (DEC always sets N)
	set_bit(&(cpu->reg_f), FLAG_BIT_N);

	// Set H flag if lower nibble is 0xF (borrow from bit 4)
	if ((*reg & 0x0F) == 0x0F) {
		set_bit(&(cpu->reg_f), FLAG_BIT_H);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_H);
	}

	// C flag is unchanged (don't touch it)
}

static inline void opcodes_add(Cpu* cpu, uint8_t value){
	// ADD: Z if result is 0, N=0, H if carry from bit 3, C if carry from bit 7
	uint16_t result = cpu->reg_a + value;

	// Set H flag if carry from bit 3 (half carry)
	if (((cpu->reg_a & 0x0F) + (value & 0x0F)) > 0x0F) {
		set_bit(&(cpu->reg_f), FLAG_BIT_H);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_H);
	}

	// Set C flag if carry from bit 7 (result > 0xFF)
	if (result > 0xFF) {
		set_bit(&(cpu->reg_f), FLAG_BIT_C);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_C);
	}

	cpu->reg_a = (uint8_t)result;

	// Set Z flag if result is 0
	if (cpu->reg_a == 0) {
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}

	// Clear N flag (ADD always clears N)
	clear_bit(&(cpu->reg_f), FLAG_BIT_N);
}

static inline void opcodes_sub(Cpu* cpu, uint8_t value){
	// SUB: Z if result is 0, N=1, H if borrow from bit 4, C if borrow occurred (A < value)

	// Set N flag (SUB always sets N)
	set_bit(&(cpu->reg_f), FLAG_BIT_N);

	// Set C flag if borrow occurred (A < value)
	if (cpu->reg_a < value) {
		set_bit(&(cpu->reg_f), FLAG_BIT_C);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_C);
	}

	// Set H flag if borrow from bit 4
	if ((cpu->reg_a & 0x0F) < (value & 0x0F)) {
		set_bit(&(cpu->reg_f), FLAG_BIT_H);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_H);
	}

	cpu->reg_a -= value;

	// Set Z flag if result is 0
	if (cpu->reg_a == 0) {
		set_bit(&(cpu->reg_f), FLAG_BIT_Z);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	}
}

static inline void opcodes_add_hl(Cpu* cpu, uint16_t value){
	// ADD HL, reg16: N=0, H if carry from bit 11, C if carry from bit 15, Z unchanged
	uint32_t result = cpu->reg_hl + value;

	// Set H flag if carry from bit 11 (half carry for 16-bit)
	if (((cpu->reg_hl & 0x0FFF) + (value & 0x0FFF)) > 0x0FFF) {
		set_bit(&(cpu->reg_f), FLAG_BIT_H);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_H);
	}

	// Set C flag if carry from bit 15 (result > 0xFFFF)
	if (result > 0xFFFF) {
		set_bit(&(cpu->reg_f), FLAG_BIT_C);
	} else {
		clear_bit(&(cpu->reg_f), FLAG_BIT_C);
	}

	cpu->reg_hl = (uint16_t)result;

	// Clear N flag (ADD always clears N)
	clear_bit(&(cpu->reg_f), FLAG_BIT_N);

	// Z flag is unchanged (don't touch it)
}

#endif /*CPU_INLINE_H*/
