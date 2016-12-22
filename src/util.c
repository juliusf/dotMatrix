#include "util.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>



int32_t read_from_disk(const char* path, uint64_t* length, unsigned char* rom[]){

	FILE* file;
	unsigned char* buffer;
	uint64_t fileLen;

	file = fopen(path, "r");
	if (!file){
		fprintf(stderr, "unable to open ROM file %s!\n", path);
		return 1;
	}
	fseek(file, 0, SEEK_END);
	fileLen = ftell(file);
	fseek(file, 0, SEEK_SET);
	buffer = (unsigned char *) malloc(fileLen +1);

	if (!buffer){
		fprintf(stderr, "unable to allocate %" PRIu64  " bytes for ROMs\n", fileLen);
		return 1;
	}
	fread(buffer, fileLen, 1, file);
	fclose(file);
	*length = fileLen;
	*rom = buffer;
	fprintf(stdout, "successfully loaded ROM file %s\n", path);
	return 0;
}

