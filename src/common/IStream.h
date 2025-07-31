#ifndef ISTREAM_H
#define ISTREAM_H

#include "_pch.h"

typedef size_t(*WriteFn)(void*, void const*, size_t);
typedef size_t(*ReadFn)(void*, void*, size_t);

typedef struct Stream
{
	void* Instance;
	WriteFn Write;
	ReadFn Read;
} Stream_t;
//-----------------------------------------------------------------------------
static size_t StreamWrite(void* stream, void const* data, size_t count)
{
	size_t ret = fwrite(data, 1, count, stream);
	fflush(stream);
	return ret;
}
//-----------------------------------------------------------------------------
static size_t StreamRead(void* stream, void* dst, size_t count)
{
	size_t readed = fread(dst, 1, count, stream);
	if (readed == count)
		return count;
	
	if (feof(stream))
		printf("Error reading : unexpected end of file\n");
	else if (ferror(stream))
		perror("Error reading ");
	
	return readed;
}
//-----------------------------------------------------------------------------
static Stream_t FileOpen(FILE* f)
{
	return (Stream_t){ .Instance = f, StreamWrite, StreamRead };
}
//-----------------------------------------------------------------------------


#endif