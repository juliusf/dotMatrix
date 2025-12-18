#ifndef INTERCONNECT_H
#define INTERCONNECT_H
#define RAM_SIZE 65536 //Addressable Memory
#define BIOS_SIZE 256

#include <stdint.h>
#include "ppu.h"

// Interrupt bits
#define INT_VBLANK  0x01  // Bit 0: V-Blank
#define INT_LCD     0x02  // Bit 1: LCD STAT
#define INT_TIMER   0x04  // Bit 2: Timer
#define INT_SERIAL  0x08  // Bit 3: Serial
#define INT_JOYPAD  0x10  // Bit 4: Joypad

typedef struct Interconnect_t{
	uint8_t ram[RAM_SIZE];
	uint8_t bios[BIOS_SIZE];
	struct Cpu_t* cpu;
	struct PPU_t* ppu;
	uint8_t inBios;
	uint8_t interrupt_flag;    // IF register (0xFF0F)
	uint8_t interrupt_enable;  // IE register (0xFFFF)

	// Timer registers (0xFF04-0xFF07)
	uint8_t div;   // 0xFF04 - Divider Register (increments at 16384 Hz)
	uint8_t tima;  // 0xFF05 - Timer Counter
	uint8_t tma;   // 0xFF06 - Timer Modulo
	uint8_t tac;   // 0xFF07 - Timer Control
	uint16_t div_counter;   // Internal counter for DIV
	uint16_t timer_counter; // Internal counter for TIMA
} Interconnect;


void initialize_interconnect(Interconnect** interconnect, struct Cpu_t** cpu);
void load_dmg_rom(Interconnect* interconnect, uint64_t romLen, unsigned char* rom);
void load_cartridge_rom(Interconnect* interconnect, uint64_t romLen, unsigned char* rom);

uint8_t read_from_ram(Interconnect* interconnect, uint16_t addr);
uint16_t read_addr_from_ram(Interconnect* interconnect, uint16_t addr);

void write_to_ram(Interconnect* interconnect, uint16_t addr, uint8_t value);

void write_addr_to_ram(Interconnect* interconnect, uint16_t addr, uint16_t value);

void timer_step(Interconnect* interconnect, uint32_t cycles);

#endif /*INTERCONNECT_H*/

