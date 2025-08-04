#include "_pch.h"
#include "edf.h"

//-----------------------------------------------------------------------------
static size_t StreamWriteImpl(void* stream, void const* data, size_t count)
{
	size_t ret = fwrite(data, 1, count, (FILE*)stream);
	fflush((FILE*)stream);
	return ret;
}
//-----------------------------------------------------------------------------
static size_t StreamWriteFormatImpl(void* stream, const char* format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	size_t ret = vfprintf((FILE*)stream, format, arglist);
	va_end(arglist);
	fflush((FILE*)stream);
	return ret;
}
//-----------------------------------------------------------------------------
static size_t StreamReadImpl(void* stream, void* dst, size_t count)
{
	size_t readed = fread(dst, 1, count, (FILE*)stream);
	if (readed == count)
		return readed;
	//if (feof(stream))
	//	printf("Error reading : unexpected end of file\n");
	if (ferror((FILE*)stream))
		perror("Error reading ");
	return readed;
}
//-----------------------------------------------------------------------------
int StreamOpen(Stream_t* s, const char* file, const char* mode)
{
	FILE* f = NULL;
	errno_t err = fopen_s(&f, file, mode);
	if (err)
	{
		LOG_ERR();
		return -1;
	}
	*s = (Stream_t)
	{
		.Instance = (void*)f,
		.Write = StreamWriteImpl,
		.Read = StreamReadImpl,
		.FWrite = StreamWriteFormatImpl,
	};
	return 0;
}
//-----------------------------------------------------------------------------
int StreamClose(Stream_t* w)
{
	return fclose((FILE*)(w->Instance));
}

//-----------------------------------------------------------------------------
int OpenBinWriter(DataWriter_t* w, const char* file)
{
	Stream_t s;
	errno_t err = StreamOpen(&s, file, "wb");
	if (err)
		return -1;
	w->Stream = s;
	w->Seq = 0;
	w->BlockLen = 0;
	w->BufLen = 0;
	w->WritePrimitive = BinToBin;
	w->FlushBlock = FlushBinBlock;
	w->SepBeginStruct = NoWrite;
	w->SepEndStruct = NoWrite;
	w->SepBeginArray = NoWrite;
	w->SepEndArray = NoWrite;
	w->SepVar = NoWrite;
	w->SepRecBegin = NoWrite;
	w->SepRecEnd = NoWrite;
	return 0;
}
//-----------------------------------------------------------------------------
int OpenTextWriter(DataWriter_t* w, const char* file)
{
	Stream_t s;
	errno_t err = StreamOpen(&s, file, "wb");
	if (err)
		return -1;

	w->Stream = s;
	w->Seq = 0;
	w->BlockLen = 0;
	w->BufLen = 0;
	w->WritePrimitive = BinToStr;
	w->FlushBlock = FlushTxtBlock;
	w->SepBeginStruct = SepBeginStruct;
	w->SepEndStruct = SepEndStruct;
	w->SepBeginArray = SepBeginArray;
	w->SepEndArray = SepEndArray;
	w->SepVar = SepVar;
	w->SepRecBegin = SepRecBegin;
	w->SepRecEnd = SepRecEnd;

	return 0;
}
//-----------------------------------------------------------------------------
int OpenBinReader(DataWriter_t* w, const char* file)
{
	Stream_t s;
	errno_t err = StreamOpen(&s, file, "rb");
	if (err)
		return -1;

	w->Stream = s;
	w->Seq = 0;
	w->BlockLen = 0;
	w->BufLen = 0;
	w->WritePrimitive = BinToStr;
	w->FlushBlock = NULL;
	w->SepBeginStruct = SepBeginStruct;
	w->SepEndStruct = SepEndStruct;
	w->SepBeginArray = SepBeginArray;
	w->SepEndArray = SepEndArray;
	w->SepVar = SepVar;
	w->SepRecBegin = SepRecBegin;
	w->SepRecEnd = SepRecEnd;

	return 0;
}
//-----------------------------------------------------------------------------
void EdfClose(DataWriter_t* dw)
{
	if (dw->Stream.Instance)
	{
		FlushDataBlock(dw);
		fclose((FILE*)dw->Stream.Instance);
		dw->Stream.Instance = NULL;
	}
}
//-----------------------------------------------------------------------------