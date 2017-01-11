
#ifndef CPU_INLINE_H
#define	CPU_INLINE_H

#include "interconnect.h"
#include "util.h"
#include <stdio.h>

static inline uint16_t get_two_byte_parameter(Cpu* cpu){
	uint8_t addr = cpu->reg_pc + 1;
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
	clear_bit(&(cpu->reg_f), FLAG_BIT_Z);
	clear_bit(&(cpu->reg_f), FLAG_BIT_N);
	clear_bit(&(cpu->reg_f), FLAG_BIT_H);
	clear_bit(&(cpu->reg_f), FLAG_BIT_C);
}

static inline void toggle_flag(Cpu* cpu, uint8_t flag){
	if( get_bit(&(cpu->reg_f), flag) ){
		set_bit( &(cpu->reg_f), flag);
	}else{
		clear_bit( &(cpu->reg_f), flag);
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

	uint8_t carry = get_bit(&(cpu->reg_f), FLAG_BIT_C) ? 1 : 0;

	uint8_t result = *reg;
	
	(( result & 0x80) != 0) ? set_bit( &(cpu->reg_f), FLAG_BIT_C) : clear_all_flags(cpu);
	result <<=1;
	result |= carry;
	*reg = result;
	return result;
}

static inline void opcodes_rl(Cpu* cpu, uint8_t* reg){
	uint8_t result = opcodes_rl_rotate(cpu, reg);
	toggle_zero_flag_from_result(cpu, result);
}

static inline void opcodes_rl_reg_a(Cpu* cpu, uint8_t* reg){
	opcodes_rl_rotate(cpu, reg);
}

static inline void opcodes_cp(Cpu* cpu, uint8_t number){
	set_bit(&(cpu->reg_f), FLAG_BIT_N);

	if (cpu->reg_a < number){
		toggle_flag(cpu, FLAG_BIT_C);
	}

	if (cpu->reg_a == number){
		toggle_flag(cpu, FLAG_BIT_Z);
	}

	if ( ((cpu->reg_a - number) & 0xf) > (cpu->reg_a & 0xF)){
		toggle_flag(cpu, FLAG_BIT_H);
	}
}

static inline void cpu_inc_toggle_bits(Cpu* cpu, uint8_t* reg){

	get_bit( &(cpu->reg_f), FLAG_BIT_C) ? set_bit(&(cpu->reg_f), FLAG_BIT_C) : clear_all_flags(cpu);
	toggle_zero_flag_from_result(cpu, *reg);
	if ((*reg & 0x0F) == 0x00){
		toggle_flag(cpu, FLAG_BIT_H);
	}
}

#endif /*CPU_INLINE_H*/
