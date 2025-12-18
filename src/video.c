#include "video.h"
#include <stdlib.h>
#include <string.h>
#include <raylib.h>

void initialize_video(Video** video){
	*video = (Video*) malloc(sizeof(Video));
	memset(*video, 0, sizeof(Video));
	(*video)->window_width = GB_SCREEN_WIDTH * SCREEN_SCALE;
	(*video)->window_height = GB_SCREEN_HEIGHT * SCREEN_SCALE;
}

void run_video_loop(Video* video){
	InitWindow(video->window_width, video->window_height, "dotMatrix - GameBoy Emulator");
	SetTargetFPS(60);

	while (!WindowShouldClose()){
		BeginDrawing();
		ClearBackground(DARKGRAY);

		// Placeholder: Draw a simple pattern to show the window is working
		DrawRectangle(
			video->window_width / 2 - 50,
			video->window_height / 2 - 50,
			100,
			100,
			RAYWHITE
		);

		DrawText("dotMatrix",
			video->window_width / 2 - MeasureText("dotMatrix", 20) / 2,
			video->window_height / 2 - 10,
			20,
			BLACK
		);

		EndDrawing();
	}

	CloseWindow();
}
