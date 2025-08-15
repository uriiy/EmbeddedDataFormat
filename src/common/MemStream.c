#include "_pch.h"
#include "edf.h"

//-----------------------------------------------------------------------------
static int MemStreamWriteImpl(void* stream, size_t* writed, void const* data, size_t len)
{
	MemStream_t* s = (MemStream_t*)stream;
	if (len > s->Size - s->Pos)
		return (size_t)-1;
	memcpy(&s->Mem[s->Pos], data, len);
	s->Pos += len;
	*writed += len;
	return 0;
}
//-----------------------------------------------------------------------------
static int StreamWriteFormatImpl(void* stream, size_t* writed, const char* format, ...)
{
	MemStream_t* s = (MemStream_t*)stream;
	size_t bufFreeLen = s->Size - s->Pos;
	va_list arglist;
	va_start(arglist, format);
	size_t ret = vsnprintf((char*)& s->Mem[s->Pos], bufFreeLen - 1, format, arglist);
	va_end(arglist);
	if (bufFreeLen < ret)
		return -1;
	s->Pos += ret - 1;
	*writed += ret - 1;
	return 0;
}
//-----------------------------------------------------------------------------
static int MemStreamReadImpl(void* stream, size_t* readed, void* dst, size_t len)
{
	MemStream_t* s = (MemStream_t*)stream;
	if (len > s->Size - s->Pos)
		return (size_t)-1;
	memcpy(dst, &s->Mem[s->Pos], len);
	s->Pos += len;
	*readed += len;
	return 0;
}
//-----------------------------------------------------------------------------
int MemStreamOpen(MemStream_t* s, uint8_t* buf, size_t size, const char* inMode)
{
	if (0 == strcmp("w", inMode) || 0 == strcmp("wb", inMode))
	{
		s->Write = MemStreamWriteImpl;
		s->Read = NULL;
		s->WriteFmt = StreamWriteFormatImpl;
		s->Mem = buf;
		s->Size = size;
		s->Pos = 0;
		return 0;
	}
	else if (0 == strcmp("r", inMode) || 0 == strcmp("rb", inMode))
	{
		s->Write = NULL;
		s->Read = MemStreamReadImpl;
		s->WriteFmt = NULL;
		s->Mem = buf;
		s->Size = size;
		s->Pos = 0;
		return 0;
	}
	return -1;
}
//-----------------------------------------------------------------------------
int MemStreamClose(MemStream_t* stream)
{
	MemStream_t* s = (MemStream_t*)stream;
	memset(s, 0, sizeof(MemStream_t));
	return 0;
}