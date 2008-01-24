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
	uint32_t x1, x2, x3, x4, i, num;

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

	num = read_uint32(f);
	printf("num: %d (0x%08x)\n", num, num);

	for(i = 0; i < num; i ++)
	{
		x1 = read_uint32(f);
		x2 = read_uint32(f);
		x3 = read_uint32(f);
		x4 = read_uint32(f);

		printf("#%03d: 0x%08x 0x%08x 0x%08x 0x%08x\n", i, x1, x2, x3, x4);
	}

	fclose(f);

	return EXIT_SUCCESS;
}
