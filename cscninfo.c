#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

/*
 * lastbyte : 00000000 11111111
 * lastbit  : 01234567 01234567
 */

static uint32_t lastbyte = 0;
static uint8_t lastbit = 0;

#define min(a, b) (((a) > (b)) ? (b) : (a))

static int read_bits(FILE *f, uint8_t nbits)
{
	uint8_t retval = 0, i, c, shift, bit, fbits;

	assert(f);
	assert(nbits <= 8);

	if(lastbit > 7)
	{
		lastbit = 0;
		lastbyte ++;
	}

	printf("%04u:%i ", lastbyte, lastbit);

	fseek(f, lastbyte, SEEK_SET);
	if(fread(&c, 1, 1, f) < 1) return -1;

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
		lastbyte ++;

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

	for(i = nbits; i < 8; i++) printf(" ");

	printf(" (retval=0x%02X)\n", retval);

	return retval;
}

static uint32_t read_uint8(FILE *f)
{
	int c;

	c = read_bits(f, 1);
	switch(c)
	{
		case 0:
			return read_bits(f, 2);
			break;

		case 1:
			return read_bits(f, 8);
			break;

		default:
			return (uint32_t)-1;
			break;
	}
}

static uint32_t read_uint16(FILE *f)
{
	return read_uint8(f) + (read_uint8(f) << 8);
}

static uint32_t read_uint32(FILE *f)
{
	return read_uint16(f) + (read_uint16(f) << 16);
}

int main(int argc, char *argv[])
{
	FILE *f;
	uint32_t chunkid, magic, offset, len, loffset = 0, i;

	if(argc < 2)
	{
		fprintf(stderr, "usage: %s <scnfile>\n", argv[0]);
		return EXIT_FAILURE;
	}

	f = fopen(argv[1], "rb");
	if(f == NULL)
	{
		fprintf(stderr, "%s: failed to read %s\n", argv[0], argv[1]);
		return EXIT_FAILURE;
	}

	do
	{
		chunkid = read_uint16(f);
		magic = read_uint16(f);
		offset = read_uint32(f);

		len = offset - loffset - 8;

		printf("0x%04X: 0x%04X: 0x%08x (%d bytes)\n",
			chunkid, magic, offset, len);

		for(i = 0; i < len; i ++); read_uint8(f);

		/* lastbyte = offset; */
		lastbit = 0;

		loffset = offset;
	}
	while(magic == 0x3334);

	fclose(f);

	return EXIT_SUCCESS;
}
