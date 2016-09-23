/*
 * zip.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <zlib.h>
#include <sg/sg.h>

int sg_zip_compress(char *in, int in_size, char **dest, int *dest_size)
{

	int level = Z_DEFAULT_COMPRESSION;
	z_stream strm;
	int ret = 0;
	int buf_size = 0;
	char *buf = NULL;
	char out[4 * 1024];
	int out_size = sizeof(out);
	bool is_success = false;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = deflateInit(&strm, level);
	if (ret != Z_OK)
		return ret;
	strm.avail_in = in_size;
	strm.next_in = in;


	do {
		strm.avail_out = out_size;
		strm.next_out = out;
		ret = deflate(&strm, Z_FULL_FLUSH); /* no bad return value */
		/* 如果设置为Z_FINISH，ret如果为Z_OK的，还需要继续，如果为Z_STREAM_END，就不需要了 */
		if (ret != Z_OK)
			goto end;
		int have = out_size - strm.avail_out;
		if (have == 0)
			continue;
		if (!buf) {
			buf = malloc(have);
			memcpy(buf, out, have);
			buf_size = have;
		} else {
			char *new_buf = malloc(buf_size + have);
			memcpy(new_buf, buf, buf_size);
			memcpy(new_buf + buf_size, out, have);
			buf_size += have;
			free(buf);
			buf = new_buf;
		}
	} while (strm.avail_out == 0);
	is_success = true;

	/* clean up and return */
	end: if (is_success) {
		*dest = buf;
		*dest_size = buf_size;
	} else
		free(buf);
	(void)deflateEnd(&strm);
	return is_success ? Z_OK : Z_ERRNO;
}

/*
 Decompress from file source to file dest until stream ends or EOF.
 inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
 allocated for processing, Z_DATA_ERROR if the deflate data is
 invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
 the version of the library linked do not match, or Z_ERRNO if there
 is an error reading or writing the files.
*/
int sg_zip_uncompress(char *in, int in_size, char **dest, int *dest_size)
{

	/* int level = Z_DEFAULT_COMPRESSION; */
	z_stream strm;
	int ret = 0;
	char *buf = NULL;
	int buf_size = 0;
	char out[4 * 1024];
	int out_size = sizeof(out);
	bool is_success = false;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK)
		return ret;
	strm.avail_in = in_size;
	strm.next_in = in;


	do {
		strm.avail_out = out_size;
		strm.next_out = out;
		ret = inflate(&strm, Z_NO_FLUSH); /* no bad return value */
		/* 如果设置为Z_FINISH，ret如果为Z_OK的，还需要继续，如果为Z_STREAM_END，就不需要了 */
		if (ret != Z_OK)
			goto end;
		int have = out_size - strm.avail_out;
		if (have == 0)
			continue;
		if (!buf) {
			buf = malloc(have);
			memcpy(buf, out, have);
			buf_size = have;
		} else {
			char *new_buf = malloc(buf_size + have);
			memcpy(new_buf, buf, buf_size);
			memcpy(new_buf + buf_size, out, have);
			buf_size += have;
			free(buf);
			buf = new_buf;
		}
	} while (strm.avail_out == 0);
	is_success = true;

	/* clean up and return */
	end: if (is_success) {
		*dest = buf;
		*dest_size = buf_size;
	} else
		free(buf);
	(void)inflateEnd(&strm);
	return is_success ? Z_OK : Z_ERRNO;
}