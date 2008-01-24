#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

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
	uint32_t offset, i;

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

	do
	{
		offset = read_uint32(f);
		if(offset != 0xFFFFFFFF)
		{
			printf("%04d, 0x%04X: 0x%08x\n", i, i, offset);
			i ++;
		}
	}
	while(offset != 0xFFFFFFFF);

	fclose(f);

	return EXIT_SUCCESS;
}
