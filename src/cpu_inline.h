
#ifndef CPU_INLINE_H
#define	CPU_INLINE_H

#include "interconnect.h"

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

#endif /*CPU_INLINE_H*/
