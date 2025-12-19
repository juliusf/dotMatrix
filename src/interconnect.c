#include "interconnect.h"
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "cpu.h"
#include "ppu.h"
#include <assert.h>
#include <stdio.h>

void initialize_interconnect(Interconnect** interconnect, struct Cpu_t** cpu){
	*interconnect = (Interconnect*) malloc(sizeof(Interconnect));
	memset(*interconnect, 0, sizeof(Interconnect));
	// Initialize RAM to 0x00 for deterministic behavior (standard for most emulators)
	// Real hardware has undefined RAM state, but 0x00 provides better compatibility
	memset((*interconnect)->ram, 0x00, RAM_SIZE);
	initialize_cpu(cpu, (*interconnect));
	(*interconnect)->cpu = *cpu;
	(*interconnect)->inBios = TRUE;

	// Initialize interrupt registers
	(*interconnect)->interrupt_flag = 0x00;
	(*interconnect)->interrupt_enable = 0x00;

	// Initialize timer registers
	(*interconnect)->div = 0x00;
	(*interconnect)->tima = 0x00;
	(*interconnect)->tma = 0x00;
	(*interconnect)->tac = 0x00;
	(*interconnect)->div_counter = 0;
	(*interconnect)->timer_counter = 0;

	// Initialize joypad (all buttons released)
	(*interconnect)->joyp = 0xFF;
	(*interconnect)->button_a = 1;
	(*interconnect)->button_b = 1;
	(*interconnect)->button_start = 1;
	(*interconnect)->button_select = 1;
	(*interconnect)->button_up = 1;
	(*interconnect)->button_down = 1;
	(*interconnect)->button_left = 1;
	(*interconnect)->button_right = 1;

	// Initialize PPU
	initialize_ppu(&((*interconnect)->ppu));
}

uint8_t read_from_ram(Interconnect* interconnect, uint16_t addr){
	if (interconnect->inBios && interconnect->cpu->reg_pc >= 0x100){ // Init sequence complete, leaving bios
		interconnect->inBios = FALSE;
		debug_print("bios initialization complete%s", "\n");
	}

	if (addr < 0x100 && interconnect->inBios){
		return interconnect->bios[addr];
	}

	// VRAM (0x8000-0x9FFF)
	if (addr >= 0x8000 && addr <= 0x9FFF){
		return ppu_read_vram(interconnect->ppu, addr);
	}

	// OAM (0xFE00-0xFE9F)
	if (addr >= 0xFE00 && addr <= 0xFE9F){
		return ppu_read_oam(interconnect->ppu, addr);
	}

	// Joypad register (0xFF00)
	if (addr == 0xFF00) {
		uint8_t result = interconnect->joyp | 0xC0;  // Bits 7-6 always set

		// Check which button group is selected
		if (!(interconnect->joyp & 0x10)) {
			// Direction keys selected (bit 4 = 0)
			result &= 0xF0;  // Clear lower 4 bits
			result |= (interconnect->button_right & 0x01) << 0;  // Right
			result |= (interconnect->button_left & 0x01) << 1;   // Left
			result |= (interconnect->button_up & 0x01) << 2;     // Up
			result |= (interconnect->button_down & 0x01) << 3;   // Down
		}

		if (!(interconnect->joyp & 0x20)) {
			// Button keys selected (bit 5 = 0)
			result &= 0xF0;  // Clear lower 4 bits
			result |= (interconnect->button_a & 0x01) << 0;      // A
			result |= (interconnect->button_b & 0x01) << 1;      // B
			result |= (interconnect->button_select & 0x01) << 2; // Select
			result |= (interconnect->button_start & 0x01) << 3;  // Start
		}

		return result;
	}

	// LCD Registers (0xFF40-0xFF4B)
	if (addr >= 0xFF40 && addr <= 0xFF4B){
		return ppu_read_register(interconnect->ppu, addr);
	}

	// Timer registers (0xFF04-0xFF07)
	if (addr == 0xFF04) {
		return interconnect->div;
	}
	if (addr == 0xFF05) return interconnect->tima;
	if (addr == 0xFF06) return interconnect->tma;
	if (addr == 0xFF07) return interconnect->tac | 0xF8;  // Upper 5 bits always set

	// Interrupt Flag (IF) - 0xFF0F
	if (addr == 0xFF0F){
		return interconnect->interrupt_flag | 0xE0;  // Upper 3 bits always set
	}

	// Interrupt Enable (IE) - 0xFFFF
	if (addr == 0xFFFF){
		return interconnect->interrupt_enable;
	}

	return interconnect->ram[addr];
}

inline uint16_t read_addr_from_ram(Interconnect* interconnect, uint16_t addr)
{
	return   (read_from_ram(interconnect, addr+1) << 8) | (read_from_ram(interconnect, addr) );
}


void write_to_ram(Interconnect* interconnect, uint16_t addr, uint8_t value)
{
	// VRAM (0x8000-0x9FFF)
	if (addr >= 0x8000 && addr <= 0x9FFF){
		ppu_write_vram(interconnect->ppu, addr, value);
		return;
	}

	// OAM (0xFE00-0xFE9F)
	if (addr >= 0xFE00 && addr <= 0xFE9F){
		ppu_write_oam(interconnect->ppu, addr, value);
		return;
	}

	// Joypad register (0xFF00)
	if (addr == 0xFF00) {
		// Only bits 5-4 are writable (select button group)
		interconnect->joyp = (value & 0x30) | 0xCF;
		return;
	}

	// LCD Registers (0xFF40-0xFF4B)
	if (addr >= 0xFF40 && addr <= 0xFF4B){
		ppu_write_register(interconnect->ppu, addr, value);
		return;
	}

	// Timer registers (0xFF04-0xFF07)
	if (addr == 0xFF04) {
		// Writing to DIV resets it to 0
		interconnect->div = 0;
		interconnect->div_counter = 0;
		return;
	}
	if (addr == 0xFF05) {
		interconnect->tima = value;
		return;
	}
	if (addr == 0xFF06) {
		interconnect->tma = value;
		return;
	}
	if (addr == 0xFF07) {
		interconnect->tac = value & 0x07;  // Only lower 3 bits are writable
		return;
	}

	// Interrupt Flag (IF) - 0xFF0F
	if (addr == 0xFF0F){
		interconnect->interrupt_flag = value & 0x1F;  // Only lower 5 bits are writable
		return;
	}

	// Interrupt Enable (IE) - 0xFFFF
	if (addr == 0xFFFF){
		interconnect->interrupt_enable = value;
		return;
	}

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

void load_cartridge_rom(Interconnect* interconnect, uint64_t romLen, unsigned char* rom){
	debug_print("loading cartridge rom, size: %llu bytes%s", romLen, "\n");

	if (romLen > RAM_SIZE){
		fprintf(stderr, "Warning: ROM size (%llu bytes) exceeds addressable memory (%d bytes). Truncating.\n", romLen, RAM_SIZE);
		romLen = RAM_SIZE;
	}

	memcpy(interconnect->ram, rom, romLen);
	debug_print("cartridge rom loaded to address 0x0000%s", "\n");
}

// Update timer registers - called with number of T-cycles elapsed
void timer_step(Interconnect* interconnect, uint32_t cycles) {
	// Update DIV register (increments at 16384 Hz = every 256 T-cycles)
	interconnect->div_counter += cycles;
	while (interconnect->div_counter >= 256) {
		interconnect->div_counter -= 256;
		interconnect->div++;
	}

	// Check if timer is enabled (bit 2 of TAC)
	if (!(interconnect->tac & 0x04)) {
		return;
	}

	// Determine timer frequency based on TAC bits 0-1
	uint16_t timer_threshold;
	switch (interconnect->tac & 0x03) {
		case 0: timer_threshold = 1024; break;  // 4096 Hz
		case 1: timer_threshold = 16;   break;  // 262144 Hz
		case 2: timer_threshold = 64;   break;  // 65536 Hz
		case 3: timer_threshold = 256;  break;  // 16384 Hz
		default: timer_threshold = 1024; break;
	}

	// Update TIMA
	interconnect->timer_counter += cycles;
	while (interconnect->timer_counter >= timer_threshold) {
		interconnect->timer_counter -= timer_threshold;
		interconnect->tima++;

		// Check for overflow
		if (interconnect->tima == 0) {
			// TIMA overflowed, reload from TMA and trigger interrupt
			interconnect->tima = interconnect->tma;
			interconnect->interrupt_flag |= INT_TIMER;
		}
	}
}
