#include "_pch.h"
#include "edf.h"

//-----------------------------------------------------------------------------
static int StreamWriteImpl(void* stream, size_t* writed, void const* data, size_t len)
{
	FILE* f = (FILE*)((FileStream_t*)stream)->Instance;
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
	FILE* f = (FILE*)((FileStream_t*)stream)->Instance;
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
	FILE* f = (FILE*)((FileStream_t*)stream)->Instance;
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
int FileStreamOpen(FileStream_t* s, const char* file, const char* inMode)
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
			*s = (FileStream_t)
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
int FileStreamClose(FileStream_t* w)
{
	fflush((FILE*)(w->Instance));
	return fclose((FILE*)(w->Instance));
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
