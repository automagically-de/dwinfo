#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#ifdef USE_ICONV
#	include <iconv.h>
#endif

#define MAX_CHUNK_ID 0x0060

static int verbose = 0;
static int dw_version = 1;

static uint8_t read_uint8(FILE *f)
{
	uint8_t val;

	fread(&val, 1, 1, f);
	return val;
}

static uint16_t read_uint16(FILE *f)
{
	uint16_t val;
	uint8_t b2[2];

	fread(b2, 1, 2, f);
	val = b2[0] + (b2[1] << 8);

	return val;
}

static uint32_t read_uint32(FILE *f)
{
	uint32_t val;
	uint8_t b4[4];

	fread(b4, 1, 4, f);
	val = b4[0] +
		(b4[1] << 8) +
		(b4[2] << 16) +
		(b4[3] << 24);

	return val;
}

static uint8_t *get_bitmap(uint16_t *blist,	uint8_t *blocks,
	uint32_t nblocks, uint32_t width)
{
	uint32_t bi, xo, yo, xi, yi, x, y, height, blocks_x;
	uint16_t b;
	uint8_t index, *bitmap;

	assert(blist != NULL);
	assert(blocks != NULL);

	height = (nblocks * 16 / width);

	blocks_x = width / 4;
	if(width % 4) blocks ++;

	printf("%dx%d\n", width, height);

	bitmap = calloc(width * height, sizeof(uint8_t));

	for(yo = 0; yo < height; yo += 4)
	{
		for(xo = 0; xo < width; xo += 4)
		{
			for(yi = 0; yi < 4; yi ++)
			{
				if((yo + yi) >= height) break;

				for(xi = 0; xi < 4; xi ++)
				{
					if((xo + xi) >= width) break;

					x = xo + xi;
					y = yo + yi;

					bi = ((yo / 4) * blocks_x + (xo / 4));

					b = blist[bi] & 0x7FFF;

					assert(b < nblocks);

					index = blocks[b * 16 + yi * 4 + xi];

					bitmap[y * width + x] = index;
				}
			}
		}
	}

	return bitmap;
}

static void hexdump(FILE *f, uint32_t len, uint8_t b32)
{
	uint32_t q, i, pos;

	pos = ftell(f);

	for(i = 0; i < len; i += 4)
	{
		if((i % 16) == 0)
		{
			if(i) printf("\n");

			printf("%08x:", pos + i);
		}

		if(b32)
		{
			q = read_uint32(f);
			printf(" 0x%08x", q);
		}
		else
		{
			q = read_uint8(f);
			printf(" %02x", q);
			if((i + 1) == len) { puts("\n"); return; }
			q = read_uint8(f);
			printf(" %02x", q);
			if((i + 2) == len) { puts("\n"); return; }
			q = read_uint8(f);
			printf(" %02x", q);
			if((i + 3) == len) { puts("\n"); return; }
			q = read_uint8(f);
			printf(" %02x", q);
		}
	}
	printf("\n");
}

static uint16_t decode_16bit(uint16_t op)
{
	return (op & 0x003F) + ((op & 0x3FC0) << 2);
}

static void parse_0x000A(FILE *f, uint32_t len)
{
	uint32_t i, off;
	uint8_t code;
	uint8_t op8;
	uint16_t op16;
	uint32_t op32;
	uint32_t stats[256];

	for(i = 0; i < 256; i++) stats[i] = 0;

	i = 0;
	off = ftell(f);

	while(i < len)
	{
		code = read_uint8(f);
		i += 1;

		switch(code)
		{
			/* no operand */
			case 0x00: /* padding, end... */

			case 0x02:
			case 0x03:
			case 0x04:

			case 0x14:
			case 0x15:
			case 0x16:
			case 0x17:
			case 0x18:
			case 0x19:
			case 0x1a:
			case 0x1b:
			case 0x1c:
			case 0x1d:
			case 0x1e: /* DW2 */

			case 0x23:
			case 0x24:
			case 0x26:
			case 0x27:
			case 0x28:
			case 0x29:
				printf("OP: [0x%08x] %02X                            "
					"(%dx)\n", (off + i - 1),
					code, ++ stats[code]);
				break;

			/* 8 bit operand */
			case 0x41:
			case 0x4a:
			case 0x4c:
			case 0x4e:
			case 0x50:
				op8 = read_uint8(f);
				i += 1;
				printf("OP: [0x%08x] %02X 0x%02x                       "
					"(%i, %i) (%dx)\n",
					(off + i - 2),
					code, op8,
					op8, (int8_t)op8,
					++ stats[code]);
				break;

			/* 16 bit operand */
			case 0x81:
			case 0x8a:
			case 0x8c:
			case 0x8e: /* DW2 */
			case 0x91:
			case 0x92:
				op16 = read_uint16(f);
				i += 2;
				printf("OP: [0x%08x] %02X 0x%04x                     "
					"(%i) (%dx)\n",
					(off + i - 3),
					code, op16, op16, ++ stats[code]);
				break;

			/* 32 bit operand */
			case 0x01:
			case 0x06:
			case 0x07:
			case 0x08:

			case 0x2B: /* DW2, ?? */
				op32 = read_uint32(f);
				i += 4;
				printf("OP: [0x%08x] %02X 0x%08x                 "
					"(%dx)\n",
					(off + i - 5), code, op32, ++ stats[code]);
				break;

			default:
				printf("OP: [0x%08x] %02X (unknown opcode)\n",
					(int)ftell(f) - 1, code);
				return;
				break;
		}
	}
}

int main(int argc, char *argv[])
{
	FILE *f;
	uint16_t type;
	uint32_t magic;
	uint32_t next_offset = 0L, this_offset = 0L, flen, blen, bread;
	uint8_t tlen, buf[8192+1];
	uint8_t palette[256][3];
	uint8_t scnfile = 0;

	uint16_t *block_list = NULL;
	uint32_t block_list_len;
	uint8_t *blocks;
	uint32_t blocks_num;
	uint16_t dump_width = 320, dump_height = 200;
	uint16_t i, chunk_stats[MAX_CHUNK_ID];

#ifdef USE_ICONV
	iconv_t icv;
	char *ic_outbuf, *ic_inbuf, *ic_outp, *ic_inp;
	size_t ic_inlen, ic_outlen = 2048;
#endif

	memset(palette, 0, 256 * 3);
	memset(chunk_stats, 0, (MAX_CHUNK_ID + 1) * 2);

	if(argc < 2)
	{
		fprintf(stderr, "usage: %s <scnfile>\n", argv[0]);
		return EXIT_FAILURE;
	}

	if(argc > 2)
	{
		verbose = (int)strtoul(argv[2], NULL, 10);
	}

	f = fopen(argv[1], "rb");
	if(f == NULL)
	{
		fprintf(stderr, "%s: error reading file '%s'\n", argv[0], argv[1]);
		return EXIT_FAILURE;
	}

#ifdef USE_ICONV
	icv = iconv_open("utf-8", "cp1252");
#endif

	fseek(f, 0, SEEK_END);
	flen = ftell(f);

	/* printf("length of file: %u\n", flen); */

	do
	{
		this_offset = next_offset;

		fseek(f, next_offset, SEEK_SET);

		type = read_uint16(f);
		magic = read_uint16(f);
		next_offset = read_uint32(f);
		if(next_offset > flen)
		{
			fprintf(stderr, "%s: next offset is %u\n",  argv[0], next_offset);
			next_offset = 0;
		}

		if(next_offset <= this_offset)
		{
			fprintf(stderr, "%s: next offset is 0x%08x (this: 0x%08x)\n",
				argv[0], next_offset, this_offset);
			next_offset = 0;
		}

		if(magic != 0x3334)
		{
			fprintf(stderr, "%s: magic != 34 33 (0x3334)\n", argv[0]);
			break;
		}

		blen = (next_offset != 0) ?
			(next_offset - this_offset) :
			(flen - this_offset);

		printf("CHUNK: 0x%8.8X: type 0x%4.4X (next: 0x%8.8X), %u (%u) bytes\n",
			this_offset, type, next_offset,
			blen, blen - 8);

		/* do chunk statistics */
		if(type <= MAX_CHUNK_ID)
			chunk_stats[type] ++;
		else
			fprintf(stderr, "E: yet undiscovered chunk id: 0x%04X\n", type);

		/* handle chunk types */
		switch(type)
		{
			case 0x0001: /* text */
				if(scnfile)
				{
					char *buf;
					buf = calloc(sizeof(char), blen - 7);
					fread(buf, 1, blen - 8, f);
					printf("TEXT: %s\n", buf);
					free(buf);
				}

				else if(verbose)
				{
					uint32_t off;
					uint32_t local_index = 0;
					static uint32_t index = 0;
					static uint32_t chunk_index = 0;

					printf("DIALOGUE (0x%04X):\n", chunk_index ++);
					bread = 0;
					do
					{
						off = (uint32_t)ftell(f);

						tlen = read_uint8(f);
						bread += 1;

						if(tlen == 0x83)
						{
							tlen = read_uint8(f);
							bread += 1;
						}

						if(tlen == 0x82)
						{
							tlen = read_uint8(f);
							bread += 1;
						}

						if(tlen == 0x80)
						{
							tlen = read_uint8(f);
							bread += 1;
						}

						fread(buf, 1, tlen, f);
						bread += tlen;
						buf[tlen] = '\0';

#ifdef USE_ICONV
						if(tlen > 0)
						{
							ic_inlen = tlen;
							ic_inp = ic_inbuf =
								calloc(ic_inlen + 1, sizeof(char));

							strcpy(ic_inbuf, (char *)buf);

							ic_outlen = 2048;
							ic_outp = ic_outbuf =
								calloc(ic_outlen, sizeof(char));

							iconv(icv,
								&ic_inp, &ic_inlen,
								&ic_outp, &ic_outlen);

							strcpy((char *)buf, ic_outbuf);
						}
#endif

						printf(" 0x%08x >> [%02X, 0x%04X] %s\n",
							off - 1, local_index ++, index, buf);

#ifdef USE_ICONV
						if(tlen > 0)
						{
							free(ic_outbuf);
							free(ic_inbuf);
						}
#endif

						index ++;
					}
					while(bread < (blen - 8));
				}
				break;

			case 0x0002:
				scnfile = 1;
				break;

			case 0x0003:
				/* block list */
				{
					uint32_t i;

					block_list_len = ((blen - 8) - 8) / 2;
					read_uint32(f); /* blocks base */
					read_uint32(f); /* unknown */

					block_list = calloc(block_list_len, sizeof(uint16_t));

					for(i = 0; i < block_list_len; i ++)
					{
						block_list[i] = read_uint16(f);
#if 0
						if(i < 10) printf("#%u: 0x%04x (0x%8.8X)\n",
							i, block_list[i], (unsigned int)ftell(f) - 2);
#endif
					}

					printf("BLOCK LIST\n");
				}
				break;

			case 0x0004:
				/* graphics? */
				printf("GRAPHICS BLOCKS\n");
				if(verbose)
				{
					FILE *pf;
					uint32_t i, j;
					uint8_t index;
					uint16_t width, height, bx, by;
					uint8_t *bitmap;

					pf = fopen("dump.ppm", "w");
					if(pf == NULL)
					{
						fprintf(stderr, "%s: failed to write dump.ppm\n",
							argv[0]);
						break;
					}

					blocks_num = (blen - 8) / 16;
					blocks = calloc(blocks_num * 16, sizeof(uint8_t));

					for(i = 0; i < blocks_num; i ++)
					{
						for(j = 0; j < 16; j ++)
						{
							blocks[i * 16 + j] = read_uint8(f);
						}
					}

					width = dump_width;
					height = dump_height;

					bx = width / 4;
					if(width % 4) bx ++;

					by = height / 4;
					if(height % 4) by ++;

					printf("bx=%u, by=%u\n", bx, by);

					bitmap = get_bitmap(block_list, blocks, bx * by,
						width);

					fprintf(pf,
						"P3\n"
						"# CREATOR: scninfo\n"
						"%u %u\n"
						"255\n",
						width, height);

					for(i = 0; i < (width * height); i ++)
					{
						index = bitmap[i];
						fprintf(pf, "%u\n%u\n%u\n",
							palette[index][0],
							palette[index][1],
							palette[index][2]);
					}

					fclose(pf);
				}
				break;

			case 0x0005:
				/* palette */
				printf("PALETTE\n");
				if(verbose)
				{
					FILE *pf;
					int i;
					uint8_t r,g,b;
					char fname[2048];

					sprintf(fname, "%s/.gimp-2.2/palettes/dw-%s.gpl",
						getenv("HOME"), "001");

					pf = fopen(fname, "w");
					if(pf == NULL)
					{
						fprintf(stderr, "%s: failed to write palette\n",
							argv[0]);
						break;
					}

					fprintf(pf,
						"GIMP Palette\n"
						"Name: Discworld\n"
						"Columns: 4\n"
						"#\n");

					for(i = 0; i < 256; i ++)
					{
						r = read_uint8(f);
						g = read_uint8(f);
						b = read_uint8(f);
						read_uint8(f); /* ignored */

						palette[i][0] = r;
						palette[i][1] = g;
						palette[i][2] = b;

						fprintf(pf, "%3u %3u %3u color%d\n",
							r, g, b, i);
					}

					fclose(pf);
				}
				break;

			case 0x0006:
				/* graphics descriptions */
				{
					uint16_t i, num, width, height;
					uint32_t bexp;
					long pos;

					printf("GRAPHICS:\n num    offset  size    flags?\n");

					num = (blen - 8) / 16;
					bexp = 0;

					for(i = 0; i < num; i ++)
					{
						uint32_t offset;
						uint32_t a1, off_pal;
						int16_t off_x, off_y;

						pos = ftell(f);

						width = read_uint16(f);
						height = read_uint16(f);

						bexp += width * height;

						off_x = read_uint16(f);
						off_y = read_uint16(f);
						a1 = read_uint32(f);
						off_pal = read_uint32(f);

						if(verbose) printf("GFX: #%04d: 0x%08x, %03ux%03u "
							"O:%4d,%4d A:0x%08x P:0x%08x\n",
							i, (unsigned)pos, width, height,
							off_x, off_y, a1, off_pal);

						offset = a1 & 0x007FFFFF;

						if(offset == 0x18)
						{
							dump_width = width;
							dump_height = height;
						}
					}
				}
				break;

			case 0x0007:
				/* IMAGE LIST */
				{
					uint16_t i, tmp1, tmp2;
					uint32_t off;

					printf("OFFSETS?:\n");

					for(i = 0; i < ((blen - 8) / 8); i ++)
					{
#if 0
						off1 = read_uint16(f);
						off2 = read_uint16(f);
#else
						off = read_uint32(f) & 0x7FFFFF;
#endif
						tmp1 = read_uint16(f);
						tmp2 = read_uint16(f);

						if(verbose) printf(
							" #%04u: 0x%08x (0x%4.4X, 0x%4.4X)\n",
							i, off, tmp1, tmp2);
					}
				}
				break;

			case 0x0008:
				hexdump(f, (blen - 8), 1);
				break;

			case 0x000A:
				parse_0x000A(f, (blen - 8));
				break;

			case 0x000B:
				/* some flags? */
				printf("SCRIPT LIST 1\n");
				{
					uint32_t i, offset, flags;
					for(i = 0; i < ((blen - 8) / 8); i ++)
					{
						flags = read_uint32(f);
						offset = read_uint32(f);

						printf("0x000B: %04d: 0x%08x, 0x%08x\n",
							i, flags, offset);
					}
				}
				break;

			case 0x000C:
				/* positions? */
				hexdump(f, (blen - 8), 1);
				break;

			case 0x000D:
				printf("SCRIPT LIST 2\n");
				{
					uint32_t i, flags, id, offset;

					for(i = 0; i < ((blen - 8) / 12); i ++)
					{
						flags = read_uint32(f);
						id = read_uint32(f);
						offset = read_uint32(f);

						printf("0x000D: %04d: 0x%08x, 0x%08x, 0x%08x\n",
							i, flags, id, offset);
					}
				}
				break;

			case 0x000E:
				/* DIRECTORY? */
				{
					uint32_t off, i;

					for(i = 0; i < ((blen - 8) / 4); i ++)
					{
						off = read_uint32(f);
						if(off & 0xFF800000)
							printf("0x000E: 0x%08x (0x%08X)\n",
								off, (off - 8) & 0x007FFFFF);
						else
							printf("0x000E: 0x%08x\n", off);
					}
				}
				break;

			case 0x0011:
				if(dw_version == 1)
				{
					/* DW 1 */
					printf("NUMBER OF OBJECTS: %d\n", read_uint32(f));
				}
				break;

			case 0x0012:
				if(dw_version == 1)
				{
					uint32_t id, a1, a2, flags, i;

					printf("OBJECTS\n");

					/* DW 1 */
					for(i = 0; i < (blen - 8) / 16; i ++)
					{
						id = read_uint32(f);
						a1 = read_uint32(f);
						a2 = read_uint32(f);
						flags = read_uint32(f);

						if(verbose)
						{
							printf("OBJ: %03d: 0x%04x (0x%04x) "
								"0x%08x 0x%08x [0x%08x]\n",
								i, id, decode_16bit((uint16_t)(id & 0xFFFF)),
								a1, a2, flags);
						}
					}
				}
				break;

			case 0x0019:
				/* DW2 BITMAP */
				printf("DW2 BITMAP\n");
				break;

			default:
				hexdump(f, (blen - 8), 0);
				/* unknown */
				break;
		}
	}
	while(next_offset != 0);

	fclose(f);

#ifdef USE_ICONV
	iconv_close(icv);
#endif

	/* output chunk statistics */
	for(i = 0; i <= MAX_CHUNK_ID; i ++)
		if(chunk_stats[i] > 0)
			printf("CHUNK STATS: 0x%04X: %d\n", i, chunk_stats[i]);

	return EXIT_SUCCESS;
}
