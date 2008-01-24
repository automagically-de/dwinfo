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
	char filename[13];
	uint8_t buf[12];
	int r, i, n;
	uint8_t buflen = 4;
	uint8_t shift = 23;
	uint32_t fsize, sizemask = 0x00FFFFFF;

	if(argc < 2)
	{
		fprintf(stderr, "usage: %s <indexfile>\n", argv[0]);
	}

	f = fopen(argv[1], "rb");
	if(!f)
	{
		fprintf(stderr, "failed to open '%s'\n", argv[1]);
		return EXIT_FAILURE;
	}

	n = 0;

	do
	{
		memset(filename, 0, 13);
		r = fread(filename, 1, 12, f);
		if(r)
		{
			if(strcmp(filename, "dw2.scn") == 0)
			{
				buflen = 8; /* Discworld 2 */
				shift = 25;
			}
			else if(strcmp(filename, "dw3.scn") == 0)
			{
				buflen = 8; /* Discworld Noir */
				shift = 24; /* untested */
				sizemask = 0xFFFFFFFF;
			}

			fsize = read_uint32(f);
			r = fread(buf, 1, buflen, f);

			printf("%-12s: %8u bytes [%u]", filename, fsize,
				(fsize & (0xFFFFFFFF ^ sizemask)) >> 24);

			for(i = 0; i < buflen; i ++)
			{
				printf(" %02x", buf[i]);
			}
			printf(" (%03i: 0x%08x)\n", n, n << shift);
		}

		n ++;
	}
	while(r);

	fclose(f);

	return EXIT_SUCCESS;
}
