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
	size_t ret = fwrite(data, 1, count, (FILE*)stream);
	fflush((FILE*)stream);
	return ret;
}
//-----------------------------------------------------------------------------
static size_t StreamRead(void* stream, void* dst, size_t count)
{
	size_t readed = fread(dst, 1, count, (FILE*)stream);
	if (readed == count)
		return readed;

	//if (feof(stream))
	//	printf("Error reading : unexpected end of file\n");

	if (ferror((FILE*)stream))
		perror("Error reading ");

	return readed;
}
//-----------------------------------------------------------------------------
static Stream_t FileOpen(FILE* f)
{
	Stream_t s =
	{
		.Instance = (void*)f,
			.Write = StreamWrite,
			.Read = StreamRead,
	};
	return s;
}
//-----------------------------------------------------------------------------


#endif