#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

static int32_t read_int24(FILE *f)
{
	int32_t val, r;
	uint8_t b3[3];

	r = fread(b3, 1, 3, f);

	if(r < 3) return 1;

#if 0
	val =
		(b3[0] << 8) +
		(b3[1] << 16) +
		((b3[2] & 0x7F) << 24);
#else
	val =
		(b3[1] << 16) + (b3[2] << 24);
#endif

#if 1
	if(b3[2] & 0x80)
		val = 0; /* 0xFFFFFFFF ^ val; */
#endif
	return val;
}

int main(int argc, char *argv[])
{
	int32_t b;
	FILE *f, *r;

	if(argc < 3)
	{
		fprintf(stderr, "usage; %s <musfile> <rawfile>\n", argv[0]);
		return EXIT_FAILURE;
	}

	f = fopen(argv[1], "rb");
	if(f == NULL)
	{
		fprintf(stderr, "%s: failed to read '%s'\n", argv[0], argv[1]);
		return EXIT_FAILURE;
	}

	r = fopen(argv[2], "wb");
	if(r == NULL)
	{
		fprintf(stderr, "%s: failed to write '%s'\n", argv[0], argv[2]);
		return EXIT_FAILURE;
	}

	b = read_int24(f);
	while(b != 1)
	{
		fwrite(&b, 4, 1, r);
		b = read_int24(f);
	}

	fclose(f);
	fclose(r);

	return EXIT_SUCCESS;
}
