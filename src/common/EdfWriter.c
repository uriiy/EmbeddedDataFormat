#include "_pch.h"
#include "edf.h"

int NoWrite(uint8_t** dst, size_t* dstLen, size_t* w);
int BeginStruct(uint8_t** dst, size_t* dstLen, size_t* w);
int EndStruct(uint8_t** dst, size_t* dstLen, size_t* w);
int BeginArray(uint8_t** dst, size_t* dstLen, size_t* w);
int EndArray(uint8_t** dst, size_t* dstLen, size_t* w);
int VarEnd(uint8_t** dst, size_t* dstLen, size_t* w);
int RecBegin(uint8_t** dst, size_t* dstLen, size_t* w);
int RecEnd(uint8_t** dst, size_t* dstLen, size_t* w);

size_t StreamWriteBlockDataBin(Stream_t* s, BlockType t, uint8_t seq, uint8_t* src, size_t len);
size_t StreamWriteBlockDataTxt(Stream_t* s, BlockType t, uint8_t seq, uint8_t* src, size_t len);

int EdfWriteHeaderBin(EdfWriter_t* dw, const EdfHeader_t* h, size_t* writed);
int EdfWriteHeaderTxt(EdfWriter_t* dw, const EdfHeader_t* h, size_t* writed);

int EdfWriteInfoBin(EdfWriter_t* w, const TypeInfo_t* t, size_t* writed);
int EdfWriteInfoTxt(EdfWriter_t* w, const TypeInfo_t* t, size_t* writed);


//-----------------------------------------------------------------------------
int EdfWriteHeader(EdfWriter_t* dw, const EdfHeader_t* h, size_t* writed)
{
	return dw->FlushHeader ? (*dw->FlushHeader)(dw, h, writed) : ERR_FN_NOT_EXIST;
}
//-----------------------------------------------------------------------------
int EdfWriteInfo(EdfWriter_t* dw, const TypeInfo_t* t, size_t* writed)
{
	return (dw->FlushInfo) ? (*dw->FlushInfo)(dw, t, writed) : ERR_FN_NOT_EXIST
}

//-----------------------------------------------------------------------------
int EdfWriteHeaderTxt(EdfWriter_t* dw, const EdfHeader_t* h, size_t* writed)
{
	dw->Seq = 0;
	*writed = StreamWriteFmt(&dw->Stream, "~ version=%d.%d.%d bs=%d encoding=%d flags=%d \n"
		, h->VersMajor, h->VersMinor, h->VersPatch
		, h->Blocksize, h->Encoding, h->Flags);
	dw->Seq++;
	dw->BlockLen = 0;
	dw->h = *h;
	return (0 >= writed) ? -1 : 0;
}
//-----------------------------------------------------------------------------
int EdfWriteHeaderBin(EdfWriter_t* dw, const EdfHeader_t* h, size_t* writed)
{
	dw->Block[0] = (uint8_t)btHeader;
	dw->Block[1] = (uint8_t)dw->Seq;
	size_t data_len = HeaderToBytes(h, &dw->Block[4]);
	*((uint16_t*)&dw->Block[2]) = (uint16_t)data_len;
	dw->BlockLen = 1 + 1 + 2 + data_len;
	*writed = StreamWrite(&dw->Stream, dw->Block, dw->BlockLen);
	if (dw->BlockLen != *writed)
		LOG_ERR();
	dw->Seq++;
	dw->BlockLen = 0;
	dw->h = *h;
	return 0;
}

//-----------------------------------------------------------------------------
int EdfWriteInfoTxt(EdfWriter_t* w, const TypeInfo_t* t, size_t* writed)
{
	*writed = StreamWrite(&w->Stream, "\n\n? ", 4);
	*writed += InfToString(t, &w->Stream, 0);
	w->Seq++;
	w->BlockLen = 0;
	w->t = t;
	return (0 >= writed) ? -1 : 0;
}
//-----------------------------------------------------------------------------
int EdfWriteInfoBin(EdfWriter_t* dw, const TypeInfo_t* t, size_t* writed)
{
	dw->Block[0] = (uint8_t)btVarInfo;
	dw->Block[1] = (uint8_t)dw->Seq;
	size_t data_len = ToBytes(t, &dw->Block[4]);
	*((uint16_t*)&dw->Block[2]) = (uint16_t)data_len;
	dw->BlockLen = 1 + 1 + 2 + data_len;
	*writed = StreamWrite(&dw->Stream, dw->Block, dw->BlockLen);
	if (dw->BlockLen != *writed)
		LOG_ERR();
	dw->Seq++;
	dw->BlockLen = 0;
	dw->t = t;
	return 0;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int OpenBinWriter(EdfWriter_t* w, const char* file)
{
	errno_t err = StreamOpen(&w->Stream, file, "wb");
	if (err)
		return -1;
	w->Seq = 0;
	w->BlockLen = 0;
	w->BufLen = 0;
	w->WritePrimitive = BinToBin;

	w->FlushHeader = EdfWriteHeaderBin;
	w->FlushInfo = EdfWriteInfoBin;

	w->FlushData = StreamWriteBlockDataBin;
	w->BeginStruct = NoWrite;
	w->EndStruct = NoWrite;
	w->BeginArray = NoWrite;
	w->EndArray = NoWrite;
	w->SepVarEnd = NoWrite;
	w->RecBegin = NoWrite;
	w->RecEnd = NoWrite;
	return 0;
}
//-----------------------------------------------------------------------------
int OpenTextWriter(EdfWriter_t* w, const char* file)
{
	errno_t err = StreamOpen(&w->Stream, file, "wb");
	if (err)
		return -1;
	w->Seq = 0;
	w->BlockLen = 0;
	w->BufLen = 0;
	w->WritePrimitive = BinToStr;

	w->FlushHeader = EdfWriteHeaderTxt;
	w->FlushInfo = EdfWriteInfoTxt;

	w->FlushData = StreamWriteBlockDataTxt;
	w->BeginStruct = BeginStruct;
	w->EndStruct = EndStruct;
	w->BeginArray = BeginArray;
	w->EndArray = EndArray;
	w->SepVarEnd = VarEnd;
	w->RecBegin = RecBegin;
	w->RecEnd = RecEnd;
	return 0;
}
//-----------------------------------------------------------------------------
int OpenBinReader(EdfWriter_t* w, const char* file)
{
	errno_t err = StreamOpen(&w->Stream, file, "rb");
	if (err)
		return -1;
	w->Seq = 0;
	w->BlockLen = 0;
	w->BufLen = 0;
	w->WritePrimitive = BinToStr;
	w->FlushHeader = NULL;
	w->FlushInfo = NULL;
	w->FlushData = NULL;
	w->BeginStruct = BeginStruct;
	w->EndStruct = EndStruct;
	w->BeginArray = BeginArray;
	w->EndArray = EndArray;
	w->SepVarEnd = VarEnd;
	w->RecBegin = RecBegin;
	w->RecEnd = RecEnd;
	return 0;
}
//-----------------------------------------------------------------------------
int OpenTextReader(EdfWriter_t* w, const char* file)
{
	UNUSED(w);
	UNUSED(file);
	return -1;
}
//-----------------------------------------------------------------------------
size_t StreamWriteBlockDataBin(Stream_t* s, BlockType t, uint8_t seq, uint8_t* src, size_t len)
{
	if (0 == len)
		return 0;
	uint8_t h[4] = { t, seq };
	*((uint16_t*)&h[2]) = (uint16_t)len;
	if (sizeof(h) != StreamWrite(s, h, sizeof(h)))
		LOG_ERR();
	if (len != StreamWrite(s, src, len))
		LOG_ERR();
	return 1 + 1 + 2 + len;
}
//-----------------------------------------------------------------------------
size_t StreamWriteBlockDataTxt(Stream_t* s, BlockType t, uint8_t seq, uint8_t* src, size_t len)
{
	UNUSED(t);
	UNUSED(seq);

	if (0 == len)
		return 0;
	if (len != StreamWrite(s, src, len))
		LOG_ERR();
	return len;
}
//-----------------------------------------------------------------------------
size_t EdfFlushDataBlock(EdfWriter_t* dw)
{
	if (NULL == dw->FlushData || 0 == dw->BlockLen)
		return 0;
	size_t ret = (*dw->FlushData)(&dw->Stream, btVarData, dw->Seq, dw->Block, dw->BlockLen);
	dw->Seq++;
	dw->BlockLen = 0;
	return ret;
}
//-----------------------------------------------------------------------------
void EdfClose(EdfWriter_t* dw)
{
	if (dw->Stream.Instance)
	{
		EdfFlushDataBlock(dw);
		StreamClose(&dw->Stream);
		dw->Stream.Instance = NULL;
	}
}
//-----------------------------------------------------------------------------
static int EdfWriteSeparator(const char* const src, size_t srcLen, uint8_t** dst, size_t* dstSize, size_t* writed)
{
	if(!srcLen)
		return 0;
	if (srcLen > *dstSize)
		return 1;
	memcpy(*dst, src, srcLen);
	(*dstSize) -= srcLen;
	(*writed) += srcLen;
	(*dst) += srcLen;
	return 0;
}
//-----------------------------------------------------------------------------
int NoWrite(uint8_t** dst, size_t* dstLen, size_t* w)
{
	UNUSED(dst);
	UNUSED(dstLen);
	UNUSED(w);
	return 0;
}
//-----------------------------------------------------------------------------
int BeginStruct(uint8_t** dst, size_t* dstLen, size_t* w)
{
	return EdfWriteSeparator(SepBeginStruct, sizeof(SepBeginStruct)-1, dst, dstLen, w);
}
//-----------------------------------------------------------------------------
int EndStruct(uint8_t** dst, size_t* dstLen, size_t* w)
{
	return EdfWriteSeparator(SepEndStruct, sizeof(SepEndStruct)-1, dst, dstLen, w);
}
//-----------------------------------------------------------------------------
int BeginArray(uint8_t** dst, size_t* dstLen, size_t* w)
{
	return EdfWriteSeparator(SepBeginArray, sizeof(SepBeginArray)-1, dst, dstLen, w);
}
//-----------------------------------------------------------------------------
int EndArray(uint8_t** dst, size_t* dstLen, size_t* w)
{
	return EdfWriteSeparator(SepEndArray, sizeof(SepEndArray)-1, dst, dstLen, w);
}
//-----------------------------------------------------------------------------
int VarEnd(uint8_t** dst, size_t* dstLen, size_t* w)
{
	return EdfWriteSeparator(SepVarEnd, sizeof(SepVarEnd)-1, dst, dstLen, w);
}
//-----------------------------------------------------------------------------
int RecBegin(uint8_t** dst, size_t* dstLen, size_t* w)
{
	return EdfWriteSeparator(SepRecBegin, sizeof(SepRecBegin)-1, dst, dstLen, w);
}
//-----------------------------------------------------------------------------
int RecEnd(uint8_t** dst, size_t* dstLen, size_t* w)
{
	return EdfWriteSeparator(SepRecEnd, sizeof(SepRecEnd)-1, dst, dstLen, w);
}