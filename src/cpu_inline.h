
#ifndef CPU_INLINE_H
#define	CPU_INLINE_H

#include "interconnect.h"

static inline uint16_t get_two_byte_parameter(Cpu* cpu){
	uint8_t addr = cpu->reg_pc + 1;
	return read_addr_from_ram(cpu->interconnect, addr);
}

#endif /*CPU_INLINE_H*/
