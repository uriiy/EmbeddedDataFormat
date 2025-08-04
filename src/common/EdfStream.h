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
static inline size_t StreamWrite(Stream_t* s, void const* data, size_t count)
{
	return (s->Write)(s->Instance, data, count);
}
static inline size_t StreamWriteFmt(Stream_t* s, const char* format, ...)
{
	size_t ret = 0;
	va_list argptr;
	va_start(argptr, format);
	ret += (s->FWrite)(s->Instance, format, argptr);
	va_end(argptr);
	return ret;
}
//-----------------------------------------------------------------------------
static inline size_t StreamRead(Stream_t* s, void* data, size_t count)
{
	return (s->Read)(s->Instance, data, count);
}
//-----------------------------------------------------------------------------
#endif