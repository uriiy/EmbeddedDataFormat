#include "_pch.h"
#include "edf.h"

//-----------------------------------------------------------------------------
static int MemStreamWriteImpl(void* stream, size_t* writed, void const* data, size_t len)
{
	MemStream_t* s = (MemStream_t*)stream;
	if (len > s->Size - s->Pos)
		return (size_t)-1;
	memcpy(&s->Buffer[s->Pos], data, len);
	s->Pos += len;
	*writed += len;
	return 0;
}
//-----------------------------------------------------------------------------
static int MemStreamWriteFormatImpl(void* stream, size_t* writed, const char* format, ...)
{
	MemStream_t* s = (MemStream_t*)stream;
	size_t bufFreeLen = s->Size - s->Pos;
	va_list arglist;
	va_start(arglist, format);
	size_t ret = vsnprintf((char*)&s->Buffer[s->Pos], bufFreeLen - 1, format, arglist);
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
	len = MIN(len, s->Pos);
	memcpy(dst, s->Buffer, len);
	s->Pos -= len;
	*readed += len;
	if (s->Pos)
		memcpy(s->Buffer, &s->Buffer[s->Pos], s->Pos);//memmove
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
int MemStreamOpen(MemStream_t* s, uint8_t* buf, size_t size, const char* inMode)
{
	if (NULL == inMode)
	{
		s->Write = MemStreamWriteImpl;
		s->Read = MemStreamReadImpl;
		s->WriteFmt = MemStreamWriteFormatImpl;
		s->Close = MemStreamClose;
		s->Buffer = buf;
		s->Size = size;
		s->Pos = 0;
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
		s->Pos = 0;
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
		s->Pos = 0;
		return 0;
	}
	return -1;
}
