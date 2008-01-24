#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define IFF_MKID(a, b, c, d) (a + (b << 8) + (c << 16) + (d << 24))
#define IFF_C4ID(u32) u32&0xFF, (u32>>8)&0xFF, (u32>>16)&0xFF, (u32>>24)&0xFF


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

static int read_cntr(FILE *f, uint32_t cid, int32_t csize, uint32_t level)
{
	int32_t id, size, tmp;
	char *padding = "                  ";
	int32_t si3[3];

	while(csize > 0)
	{
		id = read_uint32(f);
		size = read_uint32(f);
		csize -= 8;

#if 0
		if(id == 0xFFFFFFFF)
		{
			csize = 0;
			break;
		}
#endif
		printf("%s[%c%c%c%c] - %d bytes\n",
			padding + (strlen(padding) - (level * 2)),
			IFF_C4ID(id),
			size);

		switch(id)
		{
			case IFF_MKID('B','O','D','Y'):
				csize -= size;
				if(size == 0) break;
				tmp = read_uint32(f);
				size -= 4;
				if(tmp == 0)
				{
					read_uint32(f);
					size -= 4;
					while(size >= 12)
					{
						fread(si3, 4, 3, f);
						size -= 12;

						printf("VERT: %-4d, %-4d, %-4d\n",
							si3[0], si3[1], si3[2]);
					}
				}

				if(size > 0)
					fseek(f, size, SEEK_CUR);
				break;

			default:
				fseek(f, size, SEEK_CUR);
				csize -= size;
				break;
		}
	}

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	FILE *f;
	uint32_t id, size;

	if(argc < 2)
	{
		fprintf(stderr, "usage: %s <rbhfile>\n", argv[0]);
		return EXIT_FAILURE;
	}

	f = fopen(argv[1], "rb");
	if(f == NULL)
	{
		fprintf(stderr, "%s: failed to read '%s'\n", argv[0], argv[1]);
		return EXIT_FAILURE;
	}

	id = read_uint32(f);
	if(id != IFF_MKID('P', 'I', 'F', 'F'))
	{
		fprintf(stderr, "%s: %s is not a PIFF file\n", argv[0], argv[1]);
		fclose(f);
		return EXIT_FAILURE;
	}

	size = read_uint32(f);
	id = read_uint32(f);

	printf("[PIFF] (%c%c%c%c) - %d bytes\n", IFF_C4ID(id), size);
	read_cntr(f, id, size - 4, 1);

	fclose(f);
	return EXIT_SUCCESS;
}

