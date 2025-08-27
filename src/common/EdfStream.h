#ifndef EDFSTREAM_H
#define EDFSTREAM_H

#include "_pch.h"
//-----------------------------------------------------------------------------
// Common stream

typedef int(*WriteFn)	(void* stream, size_t* writed, void const* data, size_t len);
typedef int(*ReadFn)	(void* stream, size_t* readed, void* dst, size_t);
typedef int(*WriteFmtFn)(void* stream, size_t* writed, const char* format, ...);
typedef int(*CloseFn)	(void* stream);

typedef struct Stream
{
	WriteFn Write;
	ReadFn Read;
	WriteFmtFn WriteFmt;
	CloseFn Close;
	union StreamImpl
	{
		struct FileImpl
		{
			void* Instance;
		} File;
		struct MemImpl
		{
			uint8_t* Buffer;
			size_t Size;
			size_t Pos;
		} Mem;
	} Impl;
} Stream_t;

#define StreamWrite(s, w, data, count) (((s)->Write)((s), (w), data, count))
#define StreamRead(s, r, data, count) (((s)->Read)((s), (r), data, count))
#define StreamWriteFmt(s, w, fmt,...) (((s)->WriteFmt)((s), (w), fmt, __VA_ARGS__))
#define StreamClose(s) (((s)->Close)((s))

//-----------------------------------------------------------------------------
// FileStream

typedef struct FileStream
{
	WriteFn Write;
	ReadFn Read;
	WriteFmtFn WriteFmt;
	CloseFn Close;
	void* Instance;
#ifdef STREAM_BUF_SIZE
	uint8_t Buf[STREAM_BUF_SIZE];
#endif // 
} FileStream_t;

int FileStreamOpen(FileStream_t* w, const char* file, const char* mode);

//-----------------------------------------------------------------------------
//Memory Stream

typedef struct MemStream
{
	WriteFn Write;
	ReadFn Read;
	WriteFmtFn WriteFmt;
	CloseFn Close;
	uint8_t* Buffer;
	size_t Size;
	size_t Pos;
} MemStream_t;

int MemStreamOpen(MemStream_t* s, uint8_t* buf, size_t size, const char* mode);
//-----------------------------------------------------------------------------
static int MemAlloc(MemStream_t* s, size_t len, void** pptr)
{
	if (len > s->Size - s->Pos)
		return (size_t)-1;
	*pptr = &s->Buffer[s->Pos];
	memset(&s->Buffer[s->Pos], 0, len);
	s->Pos += len;
	return 0;
}
#endif