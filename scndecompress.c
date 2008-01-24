#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define min(a, b) (((a) > (b)) ? (b) : (a))

static int read_bits(FILE *f, uint8_t nbits)
{
	uint8_t retval = 0, i, shift, bit, fbits;
	static uint8_t lastbit = 0;
	static int8_t c = -1;

	if(c == -1)
		fread(&c, 1, 1, f);

	if(lastbit > 7)
	{
		lastbit = 0;
		if(fread(&c, 1, 1, f) < 1) return -1;
	}

	fbits = min(nbits, (8 - lastbit));
	for(i = 0; i < fbits; i ++)
	{
		shift = 7 - (lastbit + i);
		bit = (c >> shift) & 1;
		printf("%i", bit);

		shift = nbits - i - 1;
		retval += bit << shift;
	}

	printf("|");

	if((lastbit + nbits) > 7)
	{
		lastbit = 0;
		if(fread(&c, 1, 1, f) < 1) return -1;

		for(i = 0; i < (nbits - fbits); i ++)
		{
			shift = 7 - (lastbit + i);
			bit = (c >> shift) & 1;
			printf("%i", bit);

			shift = ((nbits - 1) - fbits) - i;
			retval += bit << shift;
		}

		lastbit = (nbits - fbits);
	}
	else
	{
		lastbit += fbits;
	}

	printf(" (retval: %d\n", retval);

	return retval;
}

static int32_t read_uint8(FILE *f)
{
	int c;
	c = read_bits(f, 1);
	switch(c)
	{
		case 0:
			/* doesn't work */
			return read_bits(f, 2);
			break;
		case 1:
			return read_bits(f, 8);
			break;
		default:
			return -1;
			break;
	}
}

int main(int argc, char *argv[])
{
	int32_t c;
	FILE *in, *out;

	if(argc < 3)
	{
		fprintf(stderr, "usage: %s <input> <output>\n", argv[0]);
		return EXIT_FAILURE;
	}

	in = fopen(argv[1], "rb");
	if(in == NULL)
	{
		fprintf(stderr, "%s: failed to open infile '%s'\n", argv[0], argv[1]);
		return EXIT_FAILURE;
	}

	out = fopen(argv[2], "wb");
	if(out == NULL)
	{
		fprintf(stderr, "%s: failed to open infile '%s'\n", argv[0], argv[2]);
		return EXIT_FAILURE;
	}

	c = read_uint8(in);
	while(c != -1)
	{
		fwrite(&c, 1, 1, out);

		c = read_uint8(in);
	}

	return EXIT_SUCCESS;
}
