#include "_pch.h"
#include "edf.h"
//-----------------------------------------------------------------------------
static void MemStreamMove(MemStream_t* s)
{
	if (s->RPos)
	{
		s->WPos -= s->RPos;//dataLen
		memcpy(s->Buffer, &s->Buffer[s->RPos], s->WPos);
		s->RPos = 0;
	}
}
//-----------------------------------------------------------------------------
static int MemStreamWriteImpl(void* stream, size_t* writed, void const* data, size_t len)
{
	MemStream_t* s = (MemStream_t*)stream;
	size_t rempty = s->RPos;
	size_t wempty = s->Size - s->WPos;
	if (len > rempty + wempty)
		return EOF;
	if (len > wempty)
		MemStreamMove(s);
	memcpy(&s->Buffer[s->WPos], data, len);
	s->WPos += len;
	if (writed)
		*writed += len;
	return 0;
}
//-----------------------------------------------------------------------------
static int MemStreamWriteFormatImpl(void* stream, size_t* writed, const char* format, ...)
{
	MemStream_t* s = (MemStream_t*)stream;
	MemStreamMove(s);
	size_t bufFreeLen = s->Size - s->WPos;
	if (0 == bufFreeLen)
		return EOF;
	va_list arglist;
	va_start(arglist, format);
	size_t ret = vsnprintf((char*)&s->Buffer[s->WPos], bufFreeLen - 1, format, arglist);
	va_end(arglist);
	if (bufFreeLen < ret)
		return EOF;
	s->WPos += ret;
	if (writed)
		*writed += ret;
	return 0;
}
//-----------------------------------------------------------------------------
static int MemStreamReadImpl(void* stream, size_t* readed, void* dst, size_t len)
{
	MemStream_t* s = (MemStream_t*)stream;
	if (len > s->WPos - s->RPos)
		return EOF;
	memcpy(dst, &s->Buffer[s->RPos], len);
	s->RPos += len;
	if (readed)
		*readed += len;
	return 0;
}
//-----------------------------------------------------------------------------
static int MemStreamClose(void* stream)
{
	MemStream_t* s = (MemStream_t*)stream;
	memset(s, 0, sizeof(MemStream_t));
	return 0;
}
//-----------------------------------------------------------------------------
int MemAlloc(MemStream_t* s, size_t len, void** pptr)
{
	if (len > s->Size - s->WPos)
		return (size_t)-1;
	*pptr = &s->Buffer[s->WPos];
	memset(&s->Buffer[s->WPos], 0, len);
	s->WPos += len;
	return 0;
}
//-----------------------------------------------------------------------------
size_t StreamLen(const MemStream_t* s)
{
	return s->WPos - s->RPos;
}
//-----------------------------------------------------------------------------
size_t StreamEmptyLen(const MemStream_t* s)
{
	return s->Size - (s->WPos - s->RPos);
}
//-----------------------------------------------------------------------------
int StreamCpy(MemStream_t* src, MemStream_t* dst, size_t len)
{
	if (StreamLen(src) < len)
		return -1;
	if (StreamEmptyLen(dst) < len)
		return 1;
	MemStreamMove(dst);
	memcpy(&dst->Buffer[dst->WPos], &src->Buffer[src->RPos], len);
	dst->WPos += len;
	src->RPos += len;
	return 0;
}
//-----------------------------------------------------------------------------
int MemStreamInOpen(MemStream_t* s, uint8_t* buf, size_t size)
{
	return MemStreamOpen(s, buf, size, size, "r");
}
//-----------------------------------------------------------------------------
int MemStreamOutOpen(MemStream_t* s, uint8_t* buf, size_t size)
{
	return MemStreamOpen(s, buf, size, 0, "w");
}
//-----------------------------------------------------------------------------
int MemStreamOpen(MemStream_t* s, uint8_t* buf, size_t size, size_t datalen, const char* inMode)
{
	if (NULL == inMode || 0 == strcmp("rw", inMode) || 0 == strcmp("wr", inMode))
	{
		s->Write = MemStreamWriteImpl;
		s->Read = MemStreamReadImpl;
		s->WriteFmt = MemStreamWriteFormatImpl;
		s->Close = MemStreamClose;
		s->Buffer = buf;
		s->Size = size;
		s->RPos = 0;
		s->WPos = datalen;
		return 0;
	}
	else if (0 == strcmp("w", inMode) || 0 == strcmp("wb", inMode))
	{
		s->Write = MemStreamWriteImpl;
		s->Read = NULL;
		s->WriteFmt = MemStreamWriteFormatImpl;
		s->Close = MemStreamClose;
		s->Buffer = buf;
		s->Size = size;
		s->RPos = 0;
		s->WPos = 0;
		return 0;
	}
	else if (0 == strcmp("r", inMode) || 0 == strcmp("rb", inMode))
	{
		s->Write = NULL;
		s->Read = MemStreamReadImpl;
		s->Close = MemStreamClose;
		s->WriteFmt = NULL;
		s->Buffer = buf;
		s->Size = size;
		s->RPos = 0;
		s->WPos = size;
		return 0;
	}
	return -1;
}
