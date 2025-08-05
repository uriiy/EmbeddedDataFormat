#ifndef EDFSTREAM_H
#define EDFSTREAM_H

#include "_pch.h"

typedef struct Stream Stream_t;

typedef size_t(*WriteFmtFn)(Stream_t*, const char* format, ...);
typedef size_t(*WriteFn)(Stream_t*, void const*, size_t);
typedef size_t(*ReadFn)(Stream_t*, void*, size_t);

typedef struct Stream
{
	void* Instance;
	WriteFn Write;
	ReadFn Read;
	WriteFmtFn WriteFmt;
#ifdef STREAM_BUF_SIZE
	uint8_t Buf[STREAM_BUF_SIZE];
#endif // 


} Stream_t;
//-----------------------------------------------------------------------------
int StreamOpen(Stream_t* w, const char* file, const char* mode);
int StreamClose(Stream_t* w);
//-----------------------------------------------------------------------------
#define StreamWrite(s, data, count) (((s)->Write)((s), data, count))
#define StreamRead(s, data, count) (((s)->Read)((s), data, count))
#define StreamWriteFmt(s, fmt,...) (((s)->WriteFmt)((s), fmt, __VA_ARGS__))

//-----------------------------------------------------------------------------
#endif