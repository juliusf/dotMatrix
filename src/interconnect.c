#include "interconnect.h"
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "cpu.h"
#include <assert.h>
#include <stdio.h>

void initialize_interconnect(Interconnect** interconnect, struct Cpu_t** cpu){
	*interconnect = (Interconnect*) malloc(sizeof(Interconnect));
	memset((*interconnect)->ram, 0, RAM_SIZE);
	initialize_cpu(cpu, (*interconnect));
	(*interconnect)->cpu = *cpu;
	(*interconnect)->inBios = TRUE;
}

uint8_t read_from_ram(Interconnect* interconnect, uint16_t addr){
	if (interconnect->cpu->reg_pc >= 0x100){ // Init sequence complete, leaving bios
		interconnect->inBios = FALSE;
		debug_print("bios initilization complete%s", "\n");
	}

	if (addr <= 0xFFFF && interconnect->inBios){
		return interconnect->bios[addr];	
	}

	return interconnect->ram[addr];
}

inline uint16_t read_addr_from_ram(Interconnect* interconnect, uint16_t addr)
{
	return   (read_from_ram(interconnect, addr) << 8) | read_from_ram(interconnect, addr + 1);
}

void load_dmg_rom(Interconnect* interconnect, uint64_t romLen, unsigned char* rom){
	debug_print("mapping bios rom%s", "\n");
	assert(romLen == 256);
	memcpy(interconnect->bios, rom, romLen);
}
