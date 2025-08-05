#include "_pch.h"
#include "edf.h"

//-----------------------------------------------------------------------------
static size_t StreamWriteImpl(Stream_t* stream, void const* data, size_t count)
{
	size_t ret = fwrite(data, 1, count, (FILE*)stream->Instance);
	fflush((FILE*)stream);
	return ret;
}
//-----------------------------------------------------------------------------
static size_t StreamWriteFormatImpl(Stream_t* stream, const char* format, ...)
{
#ifdef STREAM_BUF_SIZE
	size_t ret = 0;
	va_list arglist;
	va_start(arglist, format);
	ret = vsprintf_s((char*)stream->Buf, STREAM_BUF_SIZE, format, arglist);
	va_end(arglist);
	return StreamWriteImpl(stream, (void*)stream->Buf, ret);
#else
	va_list arglist;
	va_start(arglist, format);
	size_t ret = vfprintf((FILE*)stream->Instance, format, arglist);
	va_end(arglist);
	fflush((FILE*)stream);
	return ret;
#endif
}
//-----------------------------------------------------------------------------
static size_t StreamReadImpl(Stream_t* stream, void* dst, size_t count)
{
	size_t readed = fread(dst, 1, count, (FILE*)stream->Instance);
	if (readed == count)
		return readed;
	//if (feof(stream))
	//	printf("Error reading : unexpected end of file\n");
	if (ferror((FILE*)stream))
		perror("Error reading ");
	return readed;
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

