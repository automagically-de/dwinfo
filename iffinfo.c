#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

/* DW3: .bmv */

static uint32_t read_uint32(FILE *f)
{
	uint32_t val, r;
	uint8_t b4[4];

	r = fread(b4, 1, 4, f);

	if(r < 4) return 0xFFFFFFFF;

	val = b4[0] +
		(b4[1] << 8) +
		(b4[2] << 16) +
		(b4[3] << 24);

	return val;
}

int main(int argc, char *argv[])
{
	FILE *f;
	uint32_t name, size, i;

	if(argc < 2)
	{
		fprintf(stderr, "usage: %s midi.dat\n", argv[0]);
	}

	f = fopen(argv[1], "rb");
	if(!f)
	{
		fprintf(stderr, "failed to open '%s'\n", argv[1]);
		return EXIT_FAILURE;
	}

	i = 0;

	while(1)
	{
		name = read_uint32(f);

		if(name == 0xFFFFFFFF) break;

		size = read_uint32(f);

		printf("#%02d: %c%c%c%c %d bytes\n", i,
			name & 0xFF, (name & 0xFF00) >> 8,
			(name & 0xFF0000) >> 16, (name & 0xFF000000) >> 24,
			size);

		fseek(f, size, SEEK_CUR);

		i ++;
	}

	fclose(f);

	return EXIT_SUCCESS;
}
