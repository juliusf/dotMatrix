#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "util.h"
#include "cpu.h"
#include "interconnect.h"
#include "video.h"

#ifdef DEBUG
void sigterm_handler(int signum){
	fprintf(stderr, "\nReceived signal %d (SIGTERM), printing last instructions...\n", signum);
	print_instruction_buffer();
	exit(0);
}
#endif

int main(int argc, const char* argv[]){
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <rom_file>\n", argv[0]);
        fprintf(stderr, "Use 'make debug' to build with debug output enabled\n");
        return 1;
    }

    uint64_t romFileLen = 0;
    unsigned char* rom = NULL;

    uint64_t dmgRomFileLen = 0;
    unsigned char* dmgRom = NULL;

    read_from_disk("roms/DMG_ROM.bin", &dmgRomFileLen, &dmgRom);

    read_from_disk(argv[1], &romFileLen, &rom);

    Interconnect* interconnect = NULL;
    Cpu* cpu = NULL;
    initialize_interconnect(&interconnect, &cpu);

    load_cartridge_rom(interconnect, romFileLen, rom);
    free(rom);

    load_dmg_rom(interconnect, dmgRomFileLen, dmgRom);
    free(dmgRom);

#ifdef DEBUG
    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);
#endif

    Video* video = NULL;
    initialize_video(&video, interconnect);

    fprintf(stderr, "Starting CPU thread...\n");
    pthread_t cpu_thread = start_cpu_thread(cpu);

    fprintf(stderr, "Starting video loop in main thread. Close the window to exit.\n");
    run_video_loop(video);

    fprintf(stderr, "Window closed. Stopping CPU thread...\n");
    stop_cpu(cpu);
    pthread_join(cpu_thread, NULL);
    fprintf(stderr, "CPU thread stopped cleanly.\n");

    free(video);

    return 0;
}

