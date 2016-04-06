#ifndef gzip_h
#define gzip_h

#include "c_uuid.h"
#include "zlib.h"
#pragma comment (lib, "zlib.lib")

#include <stdlib.h>
#include <string.h>
#include <errno.h>
/* Compress data */
static int zcompress(Bytef *data, uLong ndata,
	Bytef *zdata, uLong *nzdata)
{
	z_stream c_stream;
	int err = 0;
	if(data && ndata > 0)
	{
		c_stream.zalloc = (alloc_func)0;
		c_stream.zfree = (free_func)0;
		c_stream.opaque = (voidpf)0;
		if(deflateInit(&c_stream, Z_DEFAULT_COMPRESSION) != Z_OK) return -1;
			c_stream.next_in = data;
			c_stream.avail_in = ndata;
			c_stream.next_out = zdata;
			c_stream.avail_out = *nzdata;
			while (c_stream.avail_in != 0 && c_stream.total_out < *nzdata)
			{
				if(deflate(&c_stream, Z_NO_FLUSH) != Z_OK) return -1;
			}
		
		if(c_stream.avail_in != 0) return c_stream.avail_in;
		for (;;) {
			if((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END) break;
			if(err != Z_OK) return -1;
		}
		if(deflateEnd(&c_stream) != Z_OK) return -1;
			*nzdata = c_stream.total_out;
			return 0;
	}
	return -1;
}

/* Compress gzip data */
static int gzcompress(Bytef *data, uLong ndata,
	Bytef *zdata, uLong *nzdata)
{
	z_stream c_stream;
	int err = 0;
	if(data && ndata > 0)
	{
		c_stream.zalloc = (alloc_func)0;
		c_stream.zfree = (free_func)0;
		c_stream.opaque = (voidpf)0;
		if(deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
			-MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) return -1;
			c_stream.next_in = data;
			c_stream.avail_in = ndata;
			c_stream.next_out = zdata;
			c_stream.avail_out = *nzdata;
		while (c_stream.avail_in != 0 && c_stream.total_out < *nzdata)
		{
			if(deflate(&c_stream, Z_NO_FLUSH) != Z_OK) return -1;
		}
		if(c_stream.avail_in != 0) return c_stream.avail_in;
		for (;;) {
			if((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END) break;
			if(err != Z_OK) return -1;
		}
		if(deflateEnd(&c_stream) != Z_OK) return -1;
			*nzdata = c_stream.total_out;
			return 0;
	}
	return -1;
}

/* Uncompress data */
static int zdecompress(Byte *zdata, uLong nzdata,
	Byte *data, uLong *ndata)
{
	int err = 0;
	z_stream d_stream; /* decompression stream */
	d_stream.zalloc = (alloc_func)0;
	d_stream.zfree = (free_func)0;
	d_stream.opaque = (voidpf)0;
	d_stream.next_in = zdata;
	d_stream.avail_in = 0;
	d_stream.next_out = data;
	if(inflateInit(&d_stream) != Z_OK) return -1;
	while (d_stream.total_out < *ndata && d_stream.total_in < nzdata) {
		d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
		if((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END) break;
		if(err != Z_OK) return -1;
	}
	if(inflateEnd(&d_stream) != Z_OK) return -1;
		*ndata = d_stream.total_out;
	return 0;
}

/* HTTP gzip decompress */
static int httpgzdecompress(Byte *zdata, uLong nzdata,
	Byte *data, uLong *ndata)
{
	int err = 0;
	z_stream d_stream = {0}; /* decompression stream */
	static char dummy_head[2] =
	{
		0x8 + 0x7 * 0x10,
		(((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
	};
	d_stream.zalloc = (alloc_func)0;
	d_stream.zfree = (free_func)0;
	d_stream.opaque = (voidpf)0;
	d_stream.next_in = zdata;
	d_stream.avail_in = 0;
	d_stream.next_out = data;
	if(inflateInit2(&d_stream, 47) != Z_OK) return -1;
	while (d_stream.total_out < *ndata && d_stream.total_in < nzdata) {
		d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
		if((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END) break;
		if(err != Z_OK )
		{
			if(err == Z_DATA_ERROR)
			{
				d_stream.next_in = (Bytef*) dummy_head;
				d_stream.avail_in = sizeof(dummy_head);
				if((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK)
				{
					return -1;
				}
			}
			else return -1;
		}
	}
	if(inflateEnd(&d_stream) != Z_OK) return -1;
		*ndata = d_stream.total_out;
		return 0;
}

/* Uncompress gzip data */
static int gzdecompress(Byte *zdata, uLong nzdata,
	Byte *data, uLong *ndata)
{
	int err = 0;
	z_stream d_stream = {0}; /* decompression stream */
	static char dummy_head[2] =
	{
		0x8 + 0x7 * 0x10,
		(((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
	};
	d_stream.zalloc = (alloc_func)0;
	d_stream.zfree = (free_func)0;
	d_stream.opaque = (voidpf)0;
	d_stream.next_in = zdata;
	d_stream.avail_in = 0;
	d_stream.next_out = data;
	if(inflateInit2(&d_stream, -MAX_WBITS) != Z_OK) return -1;
		//if(inflateInit2(&d_stream, 47) != Z_OK) return -1;
	while (d_stream.total_out < *ndata && d_stream.total_in < nzdata) {
		d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
		if((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END) break;
		if(err != Z_OK )
		{
			if(err == Z_DATA_ERROR)
			{
				d_stream.next_in = (Bytef*) dummy_head;
				d_stream.avail_in = sizeof(dummy_head);
				if((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK)
				{
					return -1;
				}
			}
			else return -1;
		}
	}
	if(inflateEnd(&d_stream) != Z_OK) return -1;
		*ndata = d_stream.total_out;
		return 0;
}

typedef std::vector<Byte> VBYTE;

static Byte* gzipcompress(Byte *data, uLong ndata, uLong *nzdata, VBYTE& obyte)
{
	_uuid16 uuid16;
	_uuid32 uuid32;
	c_uuid::create_uuid(uuid16);
	c_uuid::uuid16to32(uuid16, uuid32);
	gzFile pf = gzopen(uuid32, "wb");
	if (!pf)
	{
		return NULL;
	}

	if (gzwrite(pf, data, ndata) == -1)
	{
		gzclose(pf);
		return NULL;
	}

	gzclose(pf);
	FILE* f = fopen(uuid32, "rb");
	if (!f)
	{
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);
	//Byte* zdata = (Byte*)malloc(size);
	obyte.resize(size);
	*nzdata = fread(&obyte[0], 1, size, f);
	fclose(f);
	
	remove(uuid32);

	return &obyte[0];
}

#if defined(_WIN32)
#   include <fcntl.h>
#   include <io.h>
#   define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#   define SET_BINARY_MODE(file)
#endif


static Byte* ungzipcompress(Byte *zdata, uLong nzdata, uLong *ndata, VBYTE& outbuffer)
{
	_uuid16 uuid16;
	_uuid32 uuid32;
	c_uuid::create_uuid(uuid16);
	c_uuid::uuid16to32(uuid16, uuid32);
	FILE* pf = fopen(uuid32, "wb");
	if (!pf)
	{
		return NULL;
	}

	if (fwrite(zdata, nzdata, 1, pf) == -1)
	{
		gzclose(pf);
		return NULL;
	}

	fclose(pf);
	gzFile f = gzopen(uuid32, "rb");
	if (!f)
	{
		return NULL;
	}

	 outbuffer;

	while (!gzeof(f))
	{
		outbuffer.push_back(gzgetc(f));
	}

	gzclose(f);
	remove(uuid32);

	*ndata = outbuffer.size();

	return &outbuffer[0];
}
#endif//

