#ifndef VIDEO_H
#define VIDEO_H

#include <stdint.h>
#include "ppu.h"

#define GB_SCREEN_WIDTH 160
#define GB_SCREEN_HEIGHT 144
#define SCREEN_SCALE 4

typedef struct Video_t {
	int window_width;
	int window_height;
	PPU* ppu;
} Video;

void initialize_video(Video** video, PPU* ppu);
void run_video_loop(Video* video);

#endif /* VIDEO_H */
