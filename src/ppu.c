#include "ppu.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void initialize_ppu(PPU** ppu){
	*ppu = (PPU*) malloc(sizeof(PPU));
	memset(*ppu, 0, sizeof(PPU));

	// Initialize registers to Game Boy boot values
	(*ppu)->lcdc = 0x91;  // LCD on, BG on, tile data at 8000-8FFF
	(*ppu)->stat = 0x00;
	(*ppu)->scy = 0x00;
	(*ppu)->scx = 0x00;
	(*ppu)->ly = 0x00;
	(*ppu)->lyc = 0x00;
	(*ppu)->bgp = 0xFC;   // Default palette: 11 11 10 00
	(*ppu)->obp0 = 0xFF;
	(*ppu)->obp1 = 0xFF;
	(*ppu)->wy = 0x00;
	(*ppu)->wx = 0x00;

	(*ppu)->cycles = 0;
	(*ppu)->mode = MODE_OAM;
	(*ppu)->frame_ready = 0;
	(*ppu)->vblank_interrupt_requested = 0;

	// Initialize framebuffer to white
	memset((*ppu)->framebuffer, COLOR_WHITE, sizeof((*ppu)->framebuffer));
}

void ppu_step(PPU* ppu, uint32_t cycles){
	if (!(ppu->lcdc & LCDC_LCD_ENABLE)){
		// LCD is off
		return;
	}

	ppu->cycles += cycles;

	switch(ppu->mode){
		case MODE_OAM:  // OAM Search - 80 cycles
			if (ppu->cycles >= 80){
				ppu->cycles -= 80;
				ppu->mode = MODE_XFER;
				ppu->stat = (ppu->stat & ~STAT_MODE_MASK) | MODE_XFER;
			}
			break;

		case MODE_XFER:  // Pixel Transfer - 172 cycles
			if (ppu->cycles >= 172){
				ppu->cycles -= 172;
				ppu->mode = MODE_HBLANK;
				ppu->stat = (ppu->stat & ~STAT_MODE_MASK) | MODE_HBLANK;

				// Render current scanline
				ppu_render_scanline(ppu);
			}
			break;

		case MODE_HBLANK:  // H-Blank - 204 cycles
			if (ppu->cycles >= 204){
				ppu->cycles -= 204;
				ppu->ly++;

				if (ppu->ly >= VBLANK_START){
					// Enter V-Blank
												ppu->mode = MODE_VBLANK;
					ppu->stat = (ppu->stat & ~STAT_MODE_MASK) | MODE_VBLANK;
					ppu->frame_ready = 1;  // Frame is complete
					ppu->vblank_interrupt_requested = 1;  // Request V-Blank interrupt
				} else {
					// Next scanline
					ppu->mode = MODE_OAM;
					ppu->stat = (ppu->stat & ~STAT_MODE_MASK) | MODE_OAM;
				}

				// Check LYC == LY
				if (ppu->ly == ppu->lyc){
					ppu->stat |= STAT_LYC_EQUAL;
				} else {
					ppu->stat &= ~STAT_LYC_EQUAL;
				}
			}
			break;

		case MODE_VBLANK:  // V-Blank - 4560 cycles (10 scanlines)
			if (ppu->cycles >= CYCLES_PER_SCANLINE){
				ppu->cycles -= CYCLES_PER_SCANLINE;
				ppu->ly++;

				if (ppu->ly >= SCANLINES_PER_FRAME){
					// Frame complete, reset to scanline 0
					ppu->ly = 0;
					ppu->mode = MODE_OAM;
					ppu->stat = (ppu->stat & ~STAT_MODE_MASK) | MODE_OAM;
				}
			}
			break;
	}
}

void ppu_render_sprites(PPU* ppu, uint8_t scanline){
	// Check if sprites are enabled
	if (!(ppu->lcdc & LCDC_OBJ_ENABLE)){
		return;
	}

	// Determine sprite height (8x8 or 8x16)
	uint8_t sprite_height = (ppu->lcdc & LCDC_OBJ_SIZE) ? 16 : 8;

	// Find sprites that intersect with this scanline
	// OAM has 40 sprites, each is 4 bytes
	Sprite* sprites = (Sprite*)ppu->oam;
	int sprite_count = 0;
	int sprite_indices[MAX_SPRITES_PER_LINE];

	// Scan through all sprites in OAM
	for (int i = 0; i < SPRITES_IN_OAM; i++){
		Sprite* sprite = &sprites[i];

		// Sprite Y is offset by 16
		int sprite_y = sprite->y - 16;

		// Check if sprite intersects with current scanline
		if (scanline >= sprite_y && scanline < sprite_y + sprite_height){
			sprite_indices[sprite_count] = i;
			sprite_count++;

			// Game Boy can only display 10 sprites per scanline
			if (sprite_count >= MAX_SPRITES_PER_LINE){
				break;
			}
		}
	}

	// Sort sprites by X coordinate (lower X = higher priority)
	// If X is equal, lower OAM index (earlier in list) = higher priority
	for (int i = 0; i < sprite_count - 1; i++){
		for (int j = i + 1; j < sprite_count; j++){
			Sprite* sprite_i = &sprites[sprite_indices[i]];
			Sprite* sprite_j = &sprites[sprite_indices[j]];

			// Sort by X coordinate (ascending)
			// If X equal, sort by OAM index (ascending, already in order)
			if (sprite_j->x < sprite_i->x){
				// Swap
				int temp = sprite_indices[i];
				sprite_indices[i] = sprite_indices[j];
				sprite_indices[j] = temp;
			}
		}
	}

	// Track which pixels have been drawn by sprites (for priority between sprites)
	uint8_t sprite_drawn[LCD_WIDTH] = {0};

	// Render sprites in forward order (lower X coord = higher priority, drawn first and not overwritten)
	for (int i = 0; i < sprite_count; i++){
		Sprite* sprite = &sprites[sprite_indices[i]];

		// Sprite coordinates are offset
		int sprite_y = sprite->y - 16;
		int sprite_x = sprite->x - 8;

		// Get sprite attributes
		uint8_t palette_num = (sprite->attributes & SPRITE_ATTR_PALETTE) ? 1 : 0;
		uint8_t x_flip = sprite->attributes & SPRITE_ATTR_X_FLIP;
		uint8_t y_flip = sprite->attributes & SPRITE_ATTR_Y_FLIP;
		uint8_t priority = sprite->attributes & SPRITE_ATTR_PRIORITY;

		// Calculate which line of the sprite to render
		uint8_t sprite_line = scanline - sprite_y;
		if (y_flip){
			sprite_line = sprite_height - 1 - sprite_line;
		}

		// Get tile number
		uint8_t tile_num = sprite->tile;

		// In 8x16 mode, bit 0 of tile number is ignored
		if (sprite_height == 16){
			tile_num &= 0xFE;  // Clear bit 0
			// If we're rendering the bottom half, use next tile
			if (sprite_line >= 8){
				tile_num |= 0x01;
				sprite_line -= 8;
			}
		}

		// Sprites always use 8000-8FFF addressing mode
		uint16_t tile_addr = 0x8000 + (tile_num * 16);
		uint16_t tile_line_addr = tile_addr + (sprite_line * 2);
		uint8_t low_byte = ppu->vram[tile_line_addr - VRAM_START];
		uint8_t high_byte = ppu->vram[tile_line_addr + 1 - VRAM_START];

		// Select palette
		uint8_t palette = palette_num ? ppu->obp1 : ppu->obp0;

		// Render 8 pixels of the sprite
		for (int x = 0; x < 8; x++){
			int screen_x = sprite_x + x;

			// Skip if off-screen
			if (screen_x < 0 || screen_x >= LCD_WIDTH){
				continue;
			}

			// Skip if this pixel was already drawn by a higher-priority sprite
			if (sprite_drawn[screen_x]){
				continue;
			}

			// Calculate bit position (with x flip)
			int bit_pos = x_flip ? x : (7 - x);

			// Get color value (2 bits)
			uint8_t color_num = ((high_byte >> bit_pos) & 1) << 1 | ((low_byte >> bit_pos) & 1);

			// Color 0 is transparent for sprites
			if (color_num == 0){
				continue;
			}

			// Check priority vs background
			if (priority){
				// Sprite behind BG color indices 1-3 (but above BG color index 0)
				uint8_t bg_color_index = ppu->bg_colors[scanline * LCD_WIDTH + screen_x];
				if (bg_color_index != 0){
					continue;  // BG pixel is not color 0, sprite is behind
				}
			}

			// Apply palette
			uint8_t color = (palette >> (color_num * 2)) & 0x03;

			// Write to framebuffer
			ppu->framebuffer[scanline * LCD_WIDTH + screen_x] = color;

			// Mark this pixel as drawn by a sprite
			sprite_drawn[screen_x] = 1;
		}
	}
}

void ppu_render_scanline(PPU* ppu){
	uint8_t ly = ppu->ly;
	if (ly >= LCD_HEIGHT){
		return;  // Don't render V-Blank lines
	}

	// Render background
	if (!(ppu->lcdc & LCDC_BG_WIN_ENABLE)){
		// BG disabled, fill with white
		for (int x = 0; x < LCD_WIDTH; x++){
			ppu->framebuffer[ly * LCD_WIDTH + x] = COLOR_WHITE;
			ppu->bg_colors[ly * LCD_WIDTH + x] = 0;  // BG color index 0
		}
	} else {
		// Determine tile map base address
		uint16_t tilemap_base = (ppu->lcdc & LCDC_BG_TILEMAP) ? 0x9C00 : 0x9800;

		// Determine tile data base address
		int signed_tile_nums = !(ppu->lcdc & LCDC_BG_WIN_TILEDATA);
		uint16_t tiledata_base = signed_tile_nums ? 0x9000 : 0x8000;

		// Calculate Y position in background map (with scroll)
		uint8_t bg_y = (ly + ppu->scy) & 0xFF;
		uint8_t tile_row = bg_y / 8;
		uint8_t tile_y_offset = bg_y % 8;

		// Render each pixel in the scanline
		for (int x = 0; x < LCD_WIDTH; x++){
			// Calculate X position in background map (with scroll)
			uint8_t bg_x = (x + ppu->scx) & 0xFF;
			uint8_t tile_col = bg_x / 8;
			uint8_t tile_x_offset = bg_x % 8;

			// Get tile number from tile map
			uint16_t tilemap_addr = tilemap_base + (tile_row * 32) + tile_col;
			uint8_t tile_num = ppu->vram[tilemap_addr - VRAM_START];

			// Calculate tile data address
			uint16_t tile_addr;
			if (signed_tile_nums){
				// Signed tile numbers (-128 to 127)
				int8_t signed_tile = (int8_t)tile_num;
				tile_addr = tiledata_base + (signed_tile * 16);
			} else {
				// Unsigned tile numbers (0 to 255)
				tile_addr = tiledata_base + (tile_num * 16);
			}

			// Each tile is 16 bytes (8x8 pixels, 2 bits per pixel)
			// Each row is 2 bytes (low bit and high bit)
			uint16_t tile_line_addr = tile_addr + (tile_y_offset * 2);
			uint8_t low_byte = ppu->vram[tile_line_addr - VRAM_START];
			uint8_t high_byte = ppu->vram[tile_line_addr + 1 - VRAM_START];

			// Get color value (2 bits)
			int bit_pos = 7 - tile_x_offset;
			uint8_t color_num = ((high_byte >> bit_pos) & 1) << 1 | ((low_byte >> bit_pos) & 1);

			// Store original BG color index (for sprite priority)
			ppu->bg_colors[ly * LCD_WIDTH + x] = color_num;

			// Apply palette
			uint8_t color = (ppu->bgp >> (color_num * 2)) & 0x03;

			// Write to framebuffer
			ppu->framebuffer[ly * LCD_WIDTH + x] = color;
		}
	}

	// Render sprites on top of background
	ppu_render_sprites(ppu, ly);
}

// Register read/write
uint8_t ppu_read_register(PPU* ppu, uint16_t addr){
	switch(addr){
		case 0xFF40: return ppu->lcdc;
		case 0xFF41: return ppu->stat | 0x80;  // Bit 7 always set
		case 0xFF42: return ppu->scy;
		case 0xFF43: return ppu->scx;
		case 0xFF44: return ppu->ly;
		case 0xFF45: return ppu->lyc;
		case 0xFF46: return ppu->dma;
		case 0xFF47: return ppu->bgp;
		case 0xFF48: return ppu->obp0;
		case 0xFF49: return ppu->obp1;
		case 0xFF4A: return ppu->wy;
		case 0xFF4B: return ppu->wx;
		default: return 0xFF;
	}
}

void ppu_write_register(PPU* ppu, uint16_t addr, uint8_t value){
	switch(addr){
		case 0xFF40:
			ppu->lcdc = value;
			if (!(value & LCDC_LCD_ENABLE)){
				// LCD turned off, reset state
				ppu->ly = 0;
				ppu->mode = MODE_OAM;
				ppu->cycles = 0;
			}
			break;
		case 0xFF41:
			// Only bits 3-6 are writable
			ppu->stat = (ppu->stat & 0x87) | (value & 0x78);
			break;
		case 0xFF42: ppu->scy = value; break;
		case 0xFF43: ppu->scx = value; break;
		case 0xFF44: /* LY is read-only */ break;
		case 0xFF45: ppu->lyc = value; break;
		case 0xFF46: ppu->dma = value; break;  // DMA handled elsewhere
		case 0xFF47: ppu->bgp = value; break;
		case 0xFF48: ppu->obp0 = value; break;
		case 0xFF49: ppu->obp1 = value; break;
		case 0xFF4A: ppu->wy = value; break;
		case 0xFF4B: ppu->wx = value; break;
	}
}

// VRAM access
uint8_t ppu_read_vram(PPU* ppu, uint16_t addr){
	if (addr >= VRAM_START && addr <= VRAM_END){
		return ppu->vram[addr - VRAM_START];
	}
	return 0xFF;
}

void ppu_write_vram(PPU* ppu, uint16_t addr, uint8_t value){
	if (addr >= VRAM_START && addr <= VRAM_END){
		ppu->vram[addr - VRAM_START] = value;
	}
}

// OAM access
uint8_t ppu_read_oam(PPU* ppu, uint16_t addr){
	if (addr >= OAM_START && addr <= OAM_END){
		return ppu->oam[addr - OAM_START];
	}
	return 0xFF;
}

void ppu_write_oam(PPU* ppu, uint16_t addr, uint8_t value){
	if (addr >= OAM_START && addr <= OAM_END){
		ppu->oam[addr - OAM_START] = value;
	}
}
