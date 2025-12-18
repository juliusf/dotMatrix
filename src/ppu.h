#ifndef PPU_H
#define PPU_H

#include <stdint.h>

// Game Boy screen dimensions
#define LCD_WIDTH 160
#define LCD_HEIGHT 144

// PPU Memory regions
#define VRAM_SIZE 0x2000      // 8KB (0x8000-0x9FFF)
#define OAM_SIZE 0xA0         // 160 bytes (0xFE00-0xFE9F)
#define VRAM_START 0x8000
#define VRAM_END 0x9FFF
#define OAM_START 0xFE00
#define OAM_END 0xFE9F

// LCD Control Register (LCDC) bits - 0xFF40
#define LCDC_BG_WIN_ENABLE    0x01  // Bit 0: BG and Window enable
#define LCDC_OBJ_ENABLE       0x02  // Bit 1: OBJ enable
#define LCDC_OBJ_SIZE         0x04  // Bit 2: OBJ size (0=8x8, 1=8x16)
#define LCDC_BG_TILEMAP       0x08  // Bit 3: BG tile map area (0=9800-9BFF, 1=9C00-9FFF)
#define LCDC_BG_WIN_TILEDATA  0x10  // Bit 4: BG & Window tile data area (0=8800-97FF, 1=8000-8FFF)
#define LCDC_WIN_ENABLE       0x20  // Bit 5: Window enable
#define LCDC_WIN_TILEMAP      0x40  // Bit 6: Window tile map area (0=9800-9BFF, 1=9C00-9FFF)
#define LCDC_LCD_ENABLE       0x80  // Bit 7: LCD enable

// LCD Status Register (STAT) bits - 0xFF41
#define STAT_MODE_MASK        0x03  // Bits 0-1: Mode flag
#define STAT_LYC_EQUAL        0x04  // Bit 2: LYC == LY flag
#define STAT_HBLANK_INT       0x08  // Bit 3: Mode 0 H-Blank interrupt
#define STAT_VBLANK_INT       0x10  // Bit 4: Mode 1 V-Blank interrupt
#define STAT_OAM_INT          0x20  // Bit 5: Mode 2 OAM interrupt
#define STAT_LYC_INT          0x40  // Bit 6: LYC == LY interrupt

// PPU Modes
#define MODE_HBLANK  0  // H-Blank
#define MODE_VBLANK  1  // V-Blank
#define MODE_OAM     2  // OAM Search
#define MODE_XFER    3  // Pixel Transfer

// PPU timing (in T-cycles)
#define CYCLES_PER_SCANLINE 456
#define SCANLINES_PER_FRAME 154
#define VBLANK_START 144

// Color palette
#define COLOR_WHITE      0
#define COLOR_LIGHT_GRAY 1
#define COLOR_DARK_GRAY  2
#define COLOR_BLACK      3

typedef struct PPU_t {
	// Video RAM
	uint8_t vram[VRAM_SIZE];
	uint8_t oam[OAM_SIZE];

	// Framebuffer (160x144, 2 bits per pixel = 4 colors)
	uint8_t framebuffer[LCD_WIDTH * LCD_HEIGHT];

	// LCD Registers
	uint8_t lcdc;      // 0xFF40 - LCD Control
	uint8_t stat;      // 0xFF41 - LCD Status
	uint8_t scy;       // 0xFF42 - Scroll Y
	uint8_t scx;       // 0xFF43 - Scroll X
	uint8_t ly;        // 0xFF44 - LCD Y coordinate (current scanline)
	uint8_t lyc;       // 0xFF45 - LY Compare
	uint8_t dma;       // 0xFF46 - DMA Transfer
	uint8_t bgp;       // 0xFF47 - BG Palette
	uint8_t obp0;      // 0xFF48 - OBJ Palette 0
	uint8_t obp1;      // 0xFF49 - OBJ Palette 1
	uint8_t wy;        // 0xFF4A - Window Y
	uint8_t wx;        // 0xFF4B - Window X

	// Internal state
	uint32_t cycles;       // Cycle counter for current scanline
	uint8_t mode;          // Current PPU mode
	int frame_ready;       // Flag: new frame is ready to display
	int vblank_interrupt_requested;  // Flag: V-Blank interrupt requested
} PPU;

// PPU Functions
void initialize_ppu(PPU** ppu);
void ppu_step(PPU* ppu, uint32_t cycles);
void ppu_render_scanline(PPU* ppu);

// Register access
uint8_t ppu_read_register(PPU* ppu, uint16_t addr);
void ppu_write_register(PPU* ppu, uint16_t addr, uint8_t value);

// Memory access
uint8_t ppu_read_vram(PPU* ppu, uint16_t addr);
void ppu_write_vram(PPU* ppu, uint16_t addr, uint8_t value);
uint8_t ppu_read_oam(PPU* ppu, uint16_t addr);
void ppu_write_oam(PPU* ppu, uint16_t addr, uint8_t value);

#endif /* PPU_H */
