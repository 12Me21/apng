#include <stdio.h>
#include <stdint.h>
#include <byteswap.h>
#include <zlib.h>
#include <stdlib.h>
#include <errno.h>

typedef uint32_t U32;
typedef uint8_t U8;
typedef uint16_t U16;

U8 temp[100000]; //hopefully no png chunks are larger than this!

U32 read32be(void) {
	U32 res;
	if (read(0, &res, 4)==4) {// ?: exit(2);
		return bswap_32(res);
	} else {
		return 0;
	}
}

void write32be(U32 d) {
	d = bswap_32(d);
	write(1, &d, 4);
}

void writecrc(void* data, ssize_t size, U32* crc) {
	if (crc)
		*crc = crc32_z(*crc, data, size);
	while (size)
		size -= write(1, data, size);
}

void skip(ssize_t b) {
	U8* temp2 = temp;
	while (b) {
		ssize_t amt = read(0, temp2, b);
		if (amt < 0)
			exit(2);
		temp2 += amt;
		b -= amt;
		if (errno) exit(1);
	}
}

void trans(ssize_t b) {
	while (b) {
		ssize_t amt = read(0, temp, b);
		write(1, temp, amt);
		b -= amt;
	}
}

void writeFctl(U32 num, U32 width, U32 height, U16 dn, U16 dd) {
	num = bswap_32(num);
	width = bswap_32(width);
	height = bswap_32(height);
	U32 c = crc32_z(0, Z_NULL, 0);
	U32 len = bswap_32(26);
	writecrc(&len, 4, NULL);
	writecrc("fcTL", 4, &c);
	writecrc(&num, 4, &c);
	writecrc(&width, 4, &c);
	writecrc(&height, 4, &c);
	writecrc("\0\0\0\0\0\0\0\0", 8, &c);
	dn = bswap_16(dn);
	dd = bswap_16(dd);
	writecrc(&dn, 2, &c);
	writecrc(&dd, 2, &c);
	writecrc("\0\0", 2, &c);
	c = bswap_32(c);
	writecrc(&c, 4, NULL);
}

int main(int argc, char** argv) {
	if (argc != 4) {
		dprintf(2, "Usage: %s nframes delayn delayd\ngenerates animation with `nframes` frames, and a frame delay of `delayn`/`delayd` seconds\n", argv[0]);
		return 1;
	}
	trans(8);
	U32 frames = atoi(argv[1]);
	U32 frame = 0;
	U32 seq = 0;
	U16 dn = atoi(argv[2]);
	U16 dd = atoi(argv[3]);
	for (;;) {
		U32 len = read32be();
		U32 type = read32be();
		switch (type) {
		case 'IHDR':;
			U32 width = read32be();
			U32 height = read32be();
			if (frame==0) {
				write32be(len);
				write32be(type);
				write32be(width);
				write32be(height);
				trans(len-4-4+4);
				
				U32 c = crc32_z(0, Z_NULL, 0);;
				U32 len = bswap_32(8);
				writecrc(&len, 4, NULL);
				frames = bswap_32(frames);
				writecrc("acTL", 4, &c);
				writecrc(&frames, 4, &c);
				frames = bswap_32(frames);
				writecrc("\0\0\0\0", 4, &c);
				c = bswap_32(c);
				writecrc(&c, 4, NULL);
			} else {
				skip(len-4-4+4);
			}
			writeFctl(seq++, width, height, dn, dd);
			break;
		case 'IEND':
			frame++;
			if (frame>=frames)
				goto done;
			skip(len+4+8);
			break;
		case 'IDAT':
			if (frame==0) {
				write32be(len);
				write32be(type);
				trans(len + 4);
			} else {
				U32 c = crc32_z(0, Z_NULL, 0);
				write32be(len+4);
				writecrc("fdAT", 4, &c);
				U32 sframe=bswap_32(seq);
				seq++;
				writecrc(&sframe, 4, &c);
				skip(len);
				writecrc(temp, len, &c);
				c = bswap_32(c);
				writecrc(&c, 4, NULL);
				skip(4);
			}
			break;
		case 0:
			goto done;
		default:
			dprintf(2, "idk chunk: %4.4s\n", (char*)&type);
			skip(len+4);
		}
	}
 done:
	dprintf(2, "END!");
	U32 c = crc32_z(0, Z_NULL, 0);;
	writecrc("\0\0\0\0", 4, NULL);
	writecrc("IEND", 4, &c);
	c = bswap_32(c);
	writecrc(&c, 4, NULL);
	return 0;
}
