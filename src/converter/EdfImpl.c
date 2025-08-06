#include "_pch.h"
#include "edf.h"

//-----------------------------------------------------------------------------
static int StreamWriteImpl(void* stream, size_t* writed, void const* data, size_t len)
{
	FILE* f = (FILE*)((Stream_t*)stream)->Instance;
	size_t ret = fwrite(data, 1, len, f);
	if (ret != len)
	{
		if (ferror(f))
			perror("Error writing");
		return -1;
	}
	writed += ret;
	fflush(f);
	return 0;
}
//-----------------------------------------------------------------------------
static int StreamReadImpl(void* stream, size_t* readed, void* dst, size_t len)
{
	FILE* f = (FILE*)((Stream_t*)stream)->Instance;
	size_t ret = fread(dst, 1, len, f);
	if (ret != len)
	{
		if (feof(f))
			return -1;
		//	printf("Error reading : unexpected end of file\n");
		if (ferror(f))
			perror("Error reading ");
	}
	readed += ret;
	return 0;
}
//-----------------------------------------------------------------------------
static int StreamWriteFormatImpl(void* stream, size_t* writed, const char* format, ...)
{
#ifdef STREAM_BUF_SIZE
	size_t ret = 0;
	va_list arglist;
	va_start(arglist, format);
	ret = vsprintf_s((char*)stream->Buf, STREAM_BUF_SIZE, format, arglist);
	va_end(arglist);
	return StreamWriteImpl(stream, (void*)stream->Buf, ret);
#else
	FILE* f = (FILE*)((Stream_t*)stream)->Instance;
	va_list arglist;
	va_start(arglist, format);
	size_t ret = vfprintf(f, format, arglist);
	va_end(arglist);
	if (-1 == ret)
	{
		if (ferror(f))
			perror("Error writing fmt");
		return -1;
	}
	writed += ret;
	fflush(f);
	return 0;
#endif
}
//-----------------------------------------------------------------------------
int StreamOpen(Stream_t* s, const char* file, const char* inMode)
{
	const char w[] = "wb";
	const char r[] = "rb";
	const char* mode = NULL;

	int  err = -1;

	if (0 == strcmp("wb", inMode))
		mode = w;
	else if (0 == strcmp("rb", inMode))
		mode = r;

	if (mode)
	{
		FILE* f = NULL;
		err = fopen_s(&f, file, mode);
		if (!err)
		{
			*s = (Stream_t)
			{
				.Instance = (void*)f,
				.Write = StreamWriteImpl,
				.Read = StreamReadImpl,
				.WriteFmt = StreamWriteFormatImpl,
			};
			return 0;
		}
	}
	LOG_ERR();
	return err;
}
//-----------------------------------------------------------------------------
int StreamClose(Stream_t* w)
{
	fflush((FILE*)(w->Instance));
	return fclose((FILE*)(w->Instance));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static int MemStreamWriteImpl(void* stream, size_t* writed, void const* data, size_t count)
{
	MemStream_t* s = (MemStream_t*)stream;
	if (count > s->Size - s->Pos)
		return (size_t)-1;
	memcpy(&s->Buf[s->Pos], data, count);
	s->Pos += count;
	writed += count;
	return 0;
}
//-----------------------------------------------------------------------------
int MemStreamOpen(MemStream_t* s, uint8_t mod, uint8_t* buf, size_t size)
{
	MemStream_t stream = (MemStream_t)
	{
		.Buf = buf,
		.Size = size,
		.Pos = 0,
		.Write = MemStreamWriteImpl,
		.Read = NULL,
		.WriteFmt = NULL,
	};
	*s = stream;
	return 0;
}