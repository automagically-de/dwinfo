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
	FILE *f, *r;
	uint32_t offset, len, clen, unknown1;
	uint8_t fb, dw2, buf[1024];

	if(argc != 4)
	{
		fprintf(stderr, "usage: %s <smpfile> <offset> <rawfile>\n", argv[0]);
		return EXIT_FAILURE;
	}

	f = fopen(argv[1], "rb");
	if(f == NULL)
	{
		fprintf(stderr, "%s: failed to read '%s'\n", argv[0], argv[1]);
		return EXIT_FAILURE;
	}

	fread(&fb, 1, 1, f);
	if(fb != 0) dw2 = 1; else dw2 = 0;

	offset = strtoul(argv[2], NULL, 16);
	if(fseek(f, offset, SEEK_SET) != 0)
	{
		fprintf(stderr, "%s: failed to seek to 0x%08x\n", argv[0], offset);
		return EXIT_FAILURE;
	}

	r = fopen(argv[3], "wb");
	if(r == NULL)
	{
		fprintf(stderr, "%s: failed to write to '%s'\n", argv[0], argv[3]);
		return EXIT_FAILURE;
	}

	/* maybe missing */
	if(dw2)
	{
		unknown1 = read_uint32(f);
#if 0
		fwrite(&unknown1, 4, 1, r);
#endif
	}

	len = read_uint32(f);
	printf("%s: raw file is %d bytes long\n", argv[0], len);
#if 0
	if(dw2)
		fwrite(&len, 4, 1, r);
#endif

	while(len > 0)
	{
		clen = (len < 1024) ? len : 1024;
		if(fread(buf, 1, clen, f) != clen)
		{
			fprintf(stderr, "%s: failed to read %d bytes\n", argv[0], clen);
			fclose(r);
			return EXIT_FAILURE;
		}
		fwrite(buf, 1, clen, r);
		len -= clen;
	}

	fclose(f);
	fclose(r);

	return EXIT_SUCCESS;
}

