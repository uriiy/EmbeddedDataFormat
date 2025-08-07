#include "_pch.h"
#include "edf.h"

//-----------------------------------------------------------------------------
static int MemStreamWriteImpl(void* stream, size_t* writed, void const* data, size_t count)
{
	MemStream_t* s = (MemStream_t*)stream;
	if (count > s->Size - s->Pos)
		return (size_t)-1;
	memcpy(&s->Buf[s->Pos], data, count);
	s->Pos += count;
	*writed += count;
	return 0;
}
//-----------------------------------------------------------------------------
int MemStreamOpen(MemStream_t* s, uint8_t* buf, size_t size, const char* inMode)
{
	if (0 == strcmp("w", inMode) || 0 == strcmp("wb", inMode))
	{
		s->Write = MemStreamWriteImpl;
		s->Read = NULL;
		s->WriteFmt = NULL;
		s->Buf = buf;
		s->Size = size;
		s->Pos = 0;
		return 0;
	}
	else if (0 == strcmp("r", inMode) || 0 == strcmp("rb", inMode))
	{

	}
	return -1;
}