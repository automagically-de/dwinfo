#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

static int32_t read_uint16(FILE *f)
{
	uint16_t val;
	uint8_t b2[2];

	if(fread(b2, 1, 2, f) < 2) return -1;
	val = b2[0] + (b2[1] << 8);

	return val;
}

int main(int argc, char *argv[])
{
	FILE *f;
	int32_t len;
	long int off_end, off;

	if(argc < 2)
	{
		fprintf(stderr, "usage: %s <file.mus>\n", argv[0]);
		return EXIT_FAILURE;
	}

	f = fopen(argv[1], "rb");
	if(f == NULL)
	{
		fprintf(stderr, "%s: failed to read '%s'\n", argv[0], argv[1]);
		return EXIT_FAILURE;
	}

	fseek(f, 0, SEEK_END);
	off_end = ftell(f);
	fseek(f, 0, SEEK_SET);

	len = read_uint16(f);
	while(len != -1)
	{
		printf("%d\n", len);

		off = ftell(f);
		if(off + len > off_end)
		{
			fprintf(stderr, "E: trying to read %li bytes over end of file\n",
				off + len - off_end);
			return EXIT_FAILURE;
		}

		fseek(f, len, SEEK_CUR);

		len = read_uint16(f);
	}

	fclose(f);

	return EXIT_SUCCESS;
}
