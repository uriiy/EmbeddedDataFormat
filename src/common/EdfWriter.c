#include "_pch.h"
#include "edf.h"

int NoWrite(uint8_t** dst, size_t* dstLen, size_t* w);
int SepBeginStruct(uint8_t** dst, size_t* dstLen, size_t* w);
int SepEndStruct(uint8_t** dst, size_t* dstLen, size_t* w);
int SepBeginArray(uint8_t** dst, size_t* dstLen, size_t* w);
int SepEndArray(uint8_t** dst, size_t* dstLen, size_t* w);
int SepVar(uint8_t** dst, size_t* dstLen, size_t* w);
int SepRecBegin(uint8_t** dst, size_t* dstLen, size_t* w);
int SepRecEnd(uint8_t** dst, size_t* dstLen, size_t* w);

size_t StreamWriteBlockDataBin(Stream_t* s, BlockType t, uint8_t seq, uint8_t* src, size_t len);
size_t StreamWriteBlockDataTxt(Stream_t* s, BlockType t, uint8_t seq, uint8_t* src, size_t len);

int EdfWriteHeaderBin(DataWriter_t* dw, const EdfHeader_t* h, size_t* writed);
int EdfWriteHeaderTxt(DataWriter_t* dw, const EdfHeader_t* h, size_t* writed);

int EdfWriteInfoBin(DataWriter_t* w, const TypeInfo_t* t, size_t* writed);
int EdfWriteInfoTxt(DataWriter_t* w, const TypeInfo_t* t, size_t* writed);



//-----------------------------------------------------------------------------
int EdfWriteHeader(DataWriter_t* dw, const EdfHeader_t* h, size_t* writed)
{
	if (NULL == dw->FlushHeader)
		return ERR_FN_NOT_EXIST;
	return (*dw->FlushHeader)(dw, h, writed);
}
//-----------------------------------------------------------------------------
int EdfWriteInfo(DataWriter_t* dw, const TypeInfo_t* t, size_t* writed)
{
	if (NULL == dw->FlushInfo)
		return ERR_FN_NOT_EXIST;
	return (*dw->FlushInfo)(dw, t, writed);
}

//-----------------------------------------------------------------------------
int EdfWriteHeaderTxt(DataWriter_t* dw, const EdfHeader_t* h, size_t* writed)
{
	dw->Seq = 0;
	*writed = StreamWriteFmt(&dw->Stream, "~ version=%d.%d.%d bs=%d encoding=%d flags=%d \n"
		, h->VersMajor, h->VersMinor, h->VersPatch
		, h->Blocksize, h->Encoding, h->Flags);
	dw->Seq++;
	dw->BlockLen = 0;
	return (0 >= writed) ? -1 : 0;
}
//-----------------------------------------------------------------------------
int EdfWriteHeaderBin(DataWriter_t* dw, const EdfHeader_t* h, size_t* writed)
{
	dw->Block[0] = (uint8_t)btHeader;
	dw->Block[1] = (uint8_t)dw->Seq++;
	size_t data_len = HeaderToBytes(h, &dw->Block[4]);
	*((uint16_t*)&dw->Block[2]) = (uint16_t)data_len;
	dw->BlockLen = 1 + 1 + 2 + data_len;
	if (dw->BlockLen != StreamWrite(&dw->Stream, dw->Block, dw->BlockLen))
		LOG_ERR();
	data_len = dw->BlockLen;
	dw->BlockLen = 0;
	dw->h = *h;
	*writed = data_len;
	return 0;
}

//-----------------------------------------------------------------------------
int EdfWriteInfoTxt(DataWriter_t* w, const TypeInfo_t* t, size_t* writed)
{
	w->t = t;
	*writed = StreamWrite(&w->Stream, "\n\n? ", 4);
	*writed += InfToString(t, &w->Stream, 0);
	return (0 >= writed) ? -1 : 0;
}
//-----------------------------------------------------------------------------
int EdfWriteInfoBin(DataWriter_t* dw, const TypeInfo_t* t, size_t* writed)
{
	dw->Block[0] = (uint8_t)btVarInfo;
	dw->Block[1] = (uint8_t)dw->Seq++;
	size_t data_len = ToBytes(t, &dw->Block[4]);
	*((uint16_t*)&dw->Block[2]) = (uint16_t)data_len;
	dw->BlockLen = 1 + 1 + 2 + data_len;
	if (dw->BlockLen != StreamWrite(&dw->Stream, dw->Block, dw->BlockLen))
		LOG_ERR();
	data_len = dw->BlockLen;
	dw->t = t;
	dw->Seq++;
	dw->BlockLen = 0;
	*writed = data_len;
	return 0;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int OpenBinWriter(DataWriter_t* w, const char* file)
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
int OpenTextReader(DataWriter_t* w, const char* file)
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
size_t EdfFlushDataBlock(DataWriter_t* dw)
{
	if (NULL == dw->FlushData || 0 == dw->BlockLen)
		return 0;
	size_t ret = (*dw->FlushData)(&dw->Stream, btVarData, dw->Seq, dw->Block, dw->BlockLen);
	dw->Seq++;
	dw->BlockLen = 0;
	return ret;
}
//-----------------------------------------------------------------------------
void EdfClose(DataWriter_t* dw)
{
	if (dw->Stream.Instance)
	{
		EdfFlushDataBlock(dw);
		StreamClose(&dw->Stream);
		dw->Stream.Instance = NULL;
	}
}
/*
//-----------------------------------------------------------------------------
int EdfWriteSeparator(const uint8_t* const src, size_t srcLen, uint8_t** dst, size_t* dstSize, size_t* writed)
{
	if (srcLen > *dstSize)
		return 1;
	memcpy(*dst, src, srcLen);
	(*dstSize) -= srcLen;
	(*writed) += srcLen;
	(*dst) += srcLen;
	return 0;
}
*/
//-----------------------------------------------------------------------------
int NoWrite(uint8_t** dst, size_t* dstLen, size_t* w)
{
	UNUSED(dst);
	UNUSED(dstLen);
	UNUSED(w);
	return 0;
}
//-----------------------------------------------------------------------------
int SepBeginStruct(uint8_t** dst, size_t* dstLen, size_t* w)
{
	if (1 > *dstLen)
		return 1;
	--(*dstLen);
	++(*w);
	(*dst)[0] = '{';
	++(*dst);
	return 0;
}
//-----------------------------------------------------------------------------
int SepEndStruct(uint8_t** dst, size_t* dstLen, size_t* w)
{
	if (1 > *dstLen)
		return 1;
	--(*dstLen);
	++(*w);
	(*dst)[0] = '}';
	++(*dst);
	return 0;
}
//-----------------------------------------------------------------------------
int SepBeginArray(uint8_t** dst, size_t* dstLen, size_t* w)
{
	if (1 > *dstLen)
		return 1;
	--(*dstLen);
	++(*w);
	(*dst)[0] = '[';
	++(*dst);
	return 0;
}
//-----------------------------------------------------------------------------
int SepEndArray(uint8_t** dst, size_t* dstLen, size_t* w)
{
	if (1 > *dstLen)
		return 1;
	--(*dstLen);
	++(*w);
	(*dst)[0] = ']';
	++(*dst);
	return 0;
}
//-----------------------------------------------------------------------------
int SepVar(uint8_t** dst, size_t* dstLen, size_t* w)
{
	if (1 > *dstLen)
		return 1;
	--(*dstLen);
	++(*w);
	(*dst)[0] = ';';
	++(*dst);
	return 0;
}
//-----------------------------------------------------------------------------
int SepRecBegin(uint8_t** dst, size_t* dstLen, size_t* w)
{
	if (1 > *dstLen)
		return 1;
	(*dstLen) -= 3;
	(*w) += 3;
	(*dst)[0] = '\n';
	(*dst)[1] = '=';
	(*dst)[2] = ' ';
	(*dst) += 3;
	return 0;
}
//-----------------------------------------------------------------------------
int SepRecEnd(uint8_t** dst, size_t* dstLen, size_t* w)
{
	UNUSED(dst);
	UNUSED(dstLen);
	UNUSED(w);
	return 0;
}