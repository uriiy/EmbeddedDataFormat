#include "_pch.h"
#include "edf.h"

int StreamWriteBlockDataTxt(EdfWriter_t* dw, size_t* writed);
int StreamWriteBlockDataBin(EdfWriter_t* dw, size_t* writed);

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
	int err = 0;
	size_t flushed = 0;
	if ((err = EdfFlushDataBlock(dw, &flushed)))
		return err;
	dw->Skip = 0;
	return (dw->FlushInfo) ? (*dw->FlushInfo)(dw, t, writed) : ERR_FN_NOT_EXIST
}
//-----------------------------------------------------------------------------
int EdfWriteHeaderTxt(EdfWriter_t* dw, const EdfHeader_t* h, size_t* writed)
{
	dw->Seq = 0;
	int err = StreamWriteFmt(&dw->Stream, writed, "~ version=%d.%d.%d bs=%d encoding=%d flags=%d \n"
		, h->VersMajor, h->VersMinor, h->VersPatch
		, h->Blocksize, h->Encoding, h->Flags);
	if (err)
		return err;
	dw->Seq++;
	dw->BlockLen = 0;
	dw->h = *h;
	return 0;
}
//-----------------------------------------------------------------------------
int EdfWriteHeaderBin(EdfWriter_t* dw, const EdfHeader_t* h, size_t* writed)
{
	dw->Block[0] = (uint8_t)btHeader;
	dw->Block[1] = (uint8_t)dw->Seq;
	size_t data_len = HeaderToBytes(h, &dw->Block[4]);
	*((uint16_t*)&dw->Block[2]) = (uint16_t)data_len;
	dw->BlockLen = 1 + 1 + 2 + data_len;
	int err = StreamWrite(&dw->Stream, writed, dw->Block, dw->BlockLen);
	if (err)
		return err;
	dw->Seq++;
	dw->BlockLen = 0;
	dw->h = *h;
	return 0;
}
//-----------------------------------------------------------------------------
int EdfWriteInfoTxt(EdfWriter_t* w, const TypeInfo_t* t, size_t* writed)
{
	int err = 0;
	if ((err = StreamWrite(&w->Stream, writed, "\n\n? ", 4)) ||
		(err = StreamWriteInfoTxt((Stream_t*)&w->Stream, t, 0, writed)))
		return err;
	w->Seq++;
	w->BlockLen = 0;
	w->t = t;
	return err;
}
//-----------------------------------------------------------------------------
int EdfWriteInfoBin(EdfWriter_t* dw, const TypeInfo_t* t, size_t* writed)
{
	int err = 0;
	dw->Block[0] = (uint8_t)btVarInfo;
	dw->Block[1] = (uint8_t)dw->Seq;
	MemStream_t ms = { 0 };
	if ((err = MemStreamOpen(&ms, &dw->Block[4], sizeof(dw->Block) - 4, "wb")) ||
		(err = StreamWriteInfoBin((Stream_t*)&ms, t, writed)))
		return err;
	*((uint16_t*)&dw->Block[2]) = (uint16_t)ms.Pos;
	dw->BlockLen = 1 + 1 + 2 + ms.Pos;
	if ((err = StreamWrite(&dw->Stream, writed, dw->Block, dw->BlockLen)))
		return err;
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
	int err = FileStreamOpen(&w->Stream, file, "wb");
	if (err)
		return -1;
	w->Seq = 0;
	w->Skip = 0;
	w->BlockLen = 0;
	w->BufLen = 0;
	w->WritePrimitive = BinToBin;
	w->FlushHeader = EdfWriteHeaderBin;
	w->FlushInfo = EdfWriteInfoBin;
	w->FlushData = StreamWriteBlockDataBin;
	w->BeginStruct = NULL;
	w->EndStruct = NULL;
	w->BeginArray = NULL;
	w->EndArray = NULL;
	w->SepVarEnd = NULL;
	w->RecBegin = NULL;
	w->RecEnd = NULL;
	return 0;
}
//-----------------------------------------------------------------------------
int OpenTextWriter(EdfWriter_t* w, const char* file)
{
	int err = FileStreamOpen(&w->Stream, file, "wb");
	if (err)
		return -1;
	w->Seq = 0;
	w->Skip = 0;
	w->BlockLen = 0;
	w->BufLen = 0;
	w->WritePrimitive = BinToStr;
	w->FlushHeader = EdfWriteHeaderTxt;
	w->FlushInfo = EdfWriteInfoTxt;
	w->FlushData = StreamWriteBlockDataTxt;
	w->BeginStruct = SepBeginStruct;
	w->EndStruct = SepEndStruct;
	w->BeginArray = SepBeginArray;
	w->EndArray = SepEndArray;
	w->SepVarEnd = SepVarEnd;
	w->RecBegin = SepRecBegin;
	w->RecEnd = SepRecEnd;
	return 0;
}
//-----------------------------------------------------------------------------
int OpenBinReader(EdfWriter_t* w, const char* file)
{
	int err = FileStreamOpen(&w->Stream, file, "rb");
	if (err)
		return -1;
	w->Seq = 0;
	w->Skip = 0;
	w->BlockLen = 0;
	w->BufLen = 0;
	w->WritePrimitive = BinToStr;
	w->FlushHeader = NULL;
	w->FlushInfo = NULL;
	w->FlushData = NULL;
	w->BeginStruct = NULL;
	w->EndStruct = NULL;
	w->BeginArray = NULL;
	w->EndArray = NULL;
	w->SepVarEnd = NULL;
	w->RecBegin = NULL;
	w->RecEnd = NULL;
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
int StreamWriteBlockDataBin(EdfWriter_t* dw, size_t* writed)
{
	if (0 == dw->BlockLen)
		return 0;
	uint8_t h[4] = { btVarData, dw->Seq };
	*((uint16_t*)&h[2]) = (uint16_t)dw->BlockLen;
	int err = 0;
	size_t blockHeader = 0;
	if ((err = StreamWrite((Stream_t*)&dw->Stream, &blockHeader, h, sizeof(h))) ||
		(err = StreamWrite((Stream_t*)&dw->Stream, writed, dw->Block, dw->BlockLen)))
	{
		LOG_ERR();
		return err;
	}
	return 0;
}
//-----------------------------------------------------------------------------
int StreamWriteBlockDataTxt(EdfWriter_t* dw, size_t* writed)
{
	if (0 == dw->BlockLen)
		return 0;
	int err = 0;
	if ((err = StreamWrite((Stream_t*)&dw->Stream, writed, dw->Block, dw->BlockLen)))
	{
		LOG_ERR();
		return err;
	}
	return err;
}
//-----------------------------------------------------------------------------
int EdfFlushDataBlock(EdfWriter_t* dw, size_t* writed)
{
	if (NULL == dw->FlushData || 0 == dw->BlockLen)
		return 0;
	int err = (*dw->FlushData)(dw, writed);
	if (err)
	{
		LOG_ERR();
		return err;
	}
	dw->Seq++;
	dw->BlockLen = 0;
	return err;
}
//-----------------------------------------------------------------------------
void EdfClose(EdfWriter_t* dw)
{
	if (dw->Stream.Instance)
	{
		size_t w = 0;
		EdfFlushDataBlock(dw, &w);
		FileStreamClose(&dw->Stream);
		dw->Stream.Instance = NULL;
	}
}
//-----------------------------------------------------------------------------
int EdfWriteSep(const char* const src,
	uint8_t** dst, size_t* dstSize,
	size_t* skip, size_t* wqty,
	size_t* writed)
{
	if (0 < (*skip))
	{
		(*skip)--;
		return 0;
	}
	(*wqty)++;
	size_t srcLen = src ? strlen(src) : 0;
	if (!srcLen)
		return 0;
	if (srcLen > *dstSize)
		return 1;
	memcpy(*dst, src, srcLen);
	(*dstSize) -= srcLen;
	(*writed) += srcLen;
	(*dst) += srcLen;
	return 0;
}
