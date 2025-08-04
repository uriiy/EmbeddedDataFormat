#ifndef ISTREAM_H
#define ISTREAM_H

#include "_pch.h"

typedef size_t(*FWriteFn)(void*, const char* format, ...);
typedef size_t(*WriteFn)(void*, void const*, size_t);
typedef size_t(*ReadFn)(void*, void*, size_t);

typedef struct Stream
{
	void* Instance;
	WriteFn Write;
	ReadFn Read;
	FWriteFn FWrite;
} Stream_t;
//-----------------------------------------------------------------------------
int StreamOpen(Stream_t* w, const char* file, const char* mode);
int StreamClose(Stream_t* w);
//-----------------------------------------------------------------------------
#define StreamWrite(s, data, count) (((s)->Write)((s)->Instance, data, count))
#define StreamRead(s, data, count) (((s)->Read)((s)->Instance, data, count))
#define StreamWriteFmt(s,fmt,...) (((s)->FWrite)((s)->Instance, fmt, __VA_ARGS__))

//-----------------------------------------------------------------------------
#endif