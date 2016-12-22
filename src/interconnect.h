#ifndef INTERCONNECT_H
#define INTERCONNECT_H
#define RAM_SIZE 65536 //Addressable Memory
#define BIOS_SIZE 256

#include <stdint.h>


typedef struct Interconnect_t{
	uint8_t ram[RAM_SIZE];
	uint8_t bios[BIOS_SIZE];
	struct Cpu_t* cpu;
	uint8_t inBios;
} Interconnect;


void initialize_interconnect(Interconnect** interconnect, struct Cpu_t** cpu);
void load_dmg_rom(Interconnect* interconnect, uint64_t romLen, unsigned char* rom);

uint8_t read_from_ram(Interconnect* interconnect, uint16_t addr);


#endif /*INTERCONNECT_H*/

