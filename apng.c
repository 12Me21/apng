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

U8* read2(ssize_t n) {
	U8* dest = temp;
	while (n) {
		ssize_t amt = read(0, dest, n);
		n -= amt;
		dest += amt;
	}
	return temp;
}

void write2(U8* buffer, ssize_t n) {
	while (n) {
		ssize_t amt = write(1, buffer, n);
		n -= amt;
		buffer += amt;
	}
}

U32 read32be(void) {
	read2(4);
	U32 res = *(U32*)temp; // uh idk if this is allowed... alignment and all
	return bswap_32(res);
}

void write32be(U32 d) {
	d = bswap_32(d);
	write2((U8*)&d, 4);
}

U32 crc;

void crc_start() {
	crc = crc32_z(0, Z_NULL, 0);
}

void writecrc(void* data, ssize_t size) {
	crc = crc32_z(crc, data, size);
	write2(data, size);
}

void write32be_crc(U32 d) {
	d = bswap_32(d);
	writecrc(&d, 4);
}

void skip(ssize_t b) {
	read2(b);
}

void trans(ssize_t b) {
	read2(b);
	write2(temp, b);
}

U32 bswap32(U32 x) {
	return bswap_32(x);
}

void writeFctl(U32 num, U32 width, U32 height, U16 dn, U16 dd) {
	write32be(26);
	crc_start();
	writecrc("fcTL", 4);
	write32be_crc(num);
	write32be_crc(width);
	write32be_crc(height);
	writecrc("\0\0\0\0\0\0\0\0", 8);
	dn = bswap_16(dn);
	dd = bswap_16(dd);
	writecrc(&dn, 2);
	writecrc(&dd, 2);
	writecrc("\0\0", 2);
	write32be(crc);
}

int main(int argc, char** argv) {
	if (argc != 4) {
		dprintf(2, "Usage: %s nframes delayn delayd\ngenerates animation with `nframes` frames, and a frame delay of `delayn`/`delayd` seconds\n", argv[0]);
		dprintf(2, "input png files should be sent on stdin, and output is to stdout.");
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
				
				crc_start();
				write32be(8);
				writecrc("acTL", 4);
				write32be_crc(frames);
				writecrc("\0\0\0\0", 4);
				write32be(crc);
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
				crc_start();
				write32be(len+4);
				writecrc("fdAT", 4);
				write32be_crc(seq++);
				read2(len);
				writecrc(temp, len);
				write32be(crc);
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
	write32be(0);
	crc_start();
	writecrc("IEND", 4);
	write32be(crc);
	return 0;
}
