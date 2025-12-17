#include "interconnect.h"
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "cpu.h"
#include <assert.h>
#include <stdio.h>

void initialize_interconnect(Interconnect** interconnect, struct Cpu_t** cpu){
	*interconnect = (Interconnect*) malloc(sizeof(Interconnect));
	memset((*interconnect)->ram, 0xce, RAM_SIZE);
	initialize_cpu(cpu, (*interconnect));
	(*interconnect)->cpu = *cpu;
	(*interconnect)->inBios = TRUE;
}

uint8_t read_from_ram(Interconnect* interconnect, uint16_t addr){
	if (interconnect->inBios && interconnect->cpu->reg_pc >= 0x100){ // Init sequence complete, leaving bios
		interconnect->inBios = FALSE;
		debug_print("bios initialization complete%s", "\n");
	}

	if (addr < 0x100 && interconnect->inBios){
		return interconnect->bios[addr];	
	}

	return interconnect->ram[addr];
}

inline uint16_t read_addr_from_ram(Interconnect* interconnect, uint16_t addr)
{
	return   (read_from_ram(interconnect, addr+1) << 8) | (read_from_ram(interconnect, addr) );
}


void write_to_ram(Interconnect* interconnect, uint16_t addr, uint8_t value)
{
	interconnect->ram[addr] = value;
}

void write_addr_to_ram(Interconnect* interconnect, uint16_t addr, uint16_t value){
	write_to_ram(interconnect, addr+1, value >> 8);
	write_to_ram(interconnect, addr, value & 0x00FF);
}

void load_dmg_rom(Interconnect* interconnect, uint64_t romLen, unsigned char* rom){
	debug_print("mapping bios rom%s", "\n");
	assert(romLen == 256);
	memcpy(interconnect->bios, rom, romLen);
}
