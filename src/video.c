#include "video.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <raylib.h>

// DMG color palette (grayscale)
static Color dmg_palette[4] = {
	{155, 188, 15, 255},   // Color 0: Lightest (White)
	{139, 172, 15, 255},   // Color 1: Light Gray
	{48, 98, 48, 255},     // Color 2: Dark Gray
	{15, 56, 15, 255}      // Color 3: Darkest (Black)
};

void initialize_video(Video** video, PPU* ppu){
	*video = (Video*) malloc(sizeof(Video));
	memset(*video, 0, sizeof(Video));
	(*video)->window_width = GB_SCREEN_WIDTH * SCREEN_SCALE;
	(*video)->window_height = GB_SCREEN_HEIGHT * SCREEN_SCALE;
	(*video)->ppu = ppu;
}

void run_video_loop(Video* video){
	InitWindow(video->window_width, video->window_height, "dotMatrix - GameBoy Emulator");
	SetTargetFPS(60);

	// Create texture for rendering
	Image image = GenImageColor(GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT, BLANK);
	Texture2D texture = LoadTextureFromImage(image);
	UnloadImage(image);

	while (!WindowShouldClose()){
		// Check if new frame is ready
		if (video->ppu->frame_ready){
			video->ppu->frame_ready = 0;

			// Convert framebuffer to RGBA
			Color* pixels = (Color*)malloc(GB_SCREEN_WIDTH * GB_SCREEN_HEIGHT * sizeof(Color));
			for (int y = 0; y < GB_SCREEN_HEIGHT; y++){
				for (int x = 0; x < GB_SCREEN_WIDTH; x++){
					uint8_t color_index = video->ppu->framebuffer[y * GB_SCREEN_WIDTH + x];
					pixels[y * GB_SCREEN_WIDTH + x] = dmg_palette[color_index & 0x03];
				}
			}

			// Update texture
			UpdateTexture(texture, pixels);
			free(pixels);
		}

		BeginDrawing();
		ClearBackground(BLACK);

		// Draw scaled texture
		DrawTexturePro(
			texture,
			(Rectangle){0, 0, GB_SCREEN_WIDTH, GB_SCREEN_HEIGHT},
			(Rectangle){0, 0, video->window_width, video->window_height},
			(Vector2){0, 0},
			0.0f,
			WHITE
		);

		EndDrawing();
	}

	UnloadTexture(texture);
	CloseWindow();
}
