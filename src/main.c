#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "cpu.h"
#include "interconnect.h"

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

    load_dmg_rom(interconnect, dmgRomFileLen, dmgRom);
    free(dmgRom);

    run(cpu);

    return 0;
}

