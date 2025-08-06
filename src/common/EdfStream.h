#ifndef EDFSTREAM_H
#define EDFSTREAM_H

#include "_pch.h"

typedef int(*WriteFn)	(void* stream, size_t* writed, void const* data, size_t len);
typedef int(*ReadFn)	(void* stream, size_t* readed, void* dst, size_t);
typedef int(*WriteFmtFn)(void* stream, size_t* writed, const char* format, ...);

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
#define StreamWrite(s, w, data, count) (((s)->Write)((s), (w), data, count))
#define StreamRead(s, r, data, count) (((s)->Read)((s), (r), data, count))
#define StreamWriteFmt(s, w, fmt,...) (((s)->WriteFmt)((s), (w), fmt, __VA_ARGS__))

typedef struct MemStream
{
	uint8_t* Buf;
	size_t Size;
	size_t Pos;
	WriteFn Write;
	ReadFn Read;
	WriteFmtFn WriteFmt;
} MemStream_t;

int MemStreamOpen(MemStream_t* s, uint8_t mod, uint8_t* buf, size_t size);

//-----------------------------------------------------------------------------
#endif