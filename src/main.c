#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "cpu.h"
#include "interconnect.h"

int main(int argc, const char* argv[]){
    if(argc < 2 || argc > 3) {
        fprintf(stderr, "Missing first argument (the rom to load). Alternatively use --debug to enable debug mode\n");
        return 1;
    }

    uint64_t romFileLen = 0;
    unsigned char* rom = NULL;
    int8_t debug_enabled = FALSE;

    uint64_t dmgRomFileLen = 0;
    unsigned char* dmgRom = NULL;

    read_from_disk("roms/DMG_ROM.bin", &dmgRomFileLen, &dmgRom);

    //debug_print("This is a test!%s", "\n");
    
    if(strcmp(argv[1], "--debug") == 0){
        debug_enabled = TRUE;
        read_from_disk(argv[2], &romFileLen, &rom);
    }else{
        read_from_disk(argv[1], &romFileLen, &rom);
    }

    Interconnect* interconnect = NULL;
    Cpu* cpu = NULL;
    initialize_interconnect(&interconnect, &cpu);

    load_dmg_rom(interconnect, dmgRomFileLen, dmgRom);
    free(dmgRom);

    run(cpu);

    return 0;
}

