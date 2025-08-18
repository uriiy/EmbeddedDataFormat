#include "_pch.h"
#include "edf.h"


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
static int SeekEnd(EdfWriter_t* f)
{
	int err = 0;
	while (!(err = EdfReadBlock(f)))
	{
		switch (f->BlockType)
		{
		default: break;
		case btHeader:
			if (16 == f->BlockLen)
			{
				err = MakeHeaderFromBytes(f->Block, f->BlockLen, &f->h);
			}
			break;
		case btVarInfo:
		{
		}
		break;
		case btVarData:
		{
		}
		break;
		}//switch
		if (0 != err)
		{
			LOG_ERR();
			break;
		}
	}//while
	if (EOF == err)
		err = 0;
	return err;
}
//-----------------------------------------------------------------------------
static int OpenBinReader(EdfWriter_t* w, const char* file)
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
static int OpenTextReader(EdfWriter_t* w, const char* file)
{
	UNUSED(w);
	UNUSED(file);
	return -1;
}
//-----------------------------------------------------------------------------
int EdfOpen(EdfWriter_t* f, const char* file, const char* mode)
{
	if (2 > strnlen(mode, 2))
		return -1;
	int err = 0;
	if (0 == strncmp("wb", mode, 2) || 0 == strncmp("ab", mode, 2))
	{
		err = FileStreamOpen(&f->Stream, file, mode);
		if (err)
			return -1;
		f->Seq = 0;
		f->Skip = 0;
		f->BlockLen = 0;
		f->BufLen = 0;
		f->WritePrimitive = BinToBin;
		f->FlushHeader = EdfWriteHeaderBin;
		f->FlushInfo = EdfWriteInfoBin;
		f->FlushData = StreamWriteBlockDataBin;
		f->BeginStruct = NULL;
		f->EndStruct = NULL;
		f->BeginArray = NULL;
		f->EndArray = NULL;
		f->SepVarEnd = NULL;
		f->RecBegin = NULL;
		f->RecEnd = NULL;
		if (!err && 'a' == mode[0])
		{
			err = SeekEnd(f);
		}
		return err;
	}
	if (0 == strncmp("wt", mode, 2) || 0 == strncmp("at", mode, 2))
	{
		char* filemode;
		if (0 == strcmp("wt", mode))
			filemode = "wb";
		else if (0 == strcmp("at", mode))
			filemode = "ab";
		else
			return -1;
		err = FileStreamOpen(&f->Stream, file, filemode);
		if (err)
			return -1;
		f->Seq = 0;
		f->Skip = 0;
		f->BlockLen = 0;
		f->BufLen = 0;
		f->WritePrimitive = BinToStr;
		f->FlushHeader = EdfWriteHeaderTxt;
		f->FlushInfo = EdfWriteInfoTxt;
		f->FlushData = StreamWriteBlockDataTxt;
		f->BeginStruct = SepBeginStruct;
		f->EndStruct = SepEndStruct;
		f->BeginArray = SepBeginArray;
		f->EndArray = SepEndArray;
		f->SepVarEnd = SepVarEnd;
		f->RecBegin = SepRecBegin;
		f->RecEnd = SepRecEnd;
		return err;
	}
	if (0 == strncmp("rb", mode, 2))
		return OpenBinReader(f, file);
	if (0 == strncmp("rt", mode, 2))
		return OpenTextReader(f, file);
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
	size_t srcLen = src ? strnlen(src, 10) : 0;
	if (!srcLen)
	{
		(*wqty)++;
		return 0;
	}
	if (srcLen > *dstSize)
		return 1;
	(*wqty)++;
	memcpy(*dst, src, srcLen);
	(*dstSize) -= srcLen;
	(*writed) += srcLen;
	(*dst) += srcLen;
	return 0;
}
//-----------------------------------------------------------------------------
int EdfWriteInfData(EdfWriter_t* dw, PoType pt, char* name, void* data)
{
	int err;
	size_t writed = 0;
	if ((err = EdfWriteInfo(dw, &((TypeInfo_t) { .Type = pt, .Name = name }), &writed)))
		return err;
	if (String == pt)
	{
		uint8_t strBuf[256] = { 0 };
		uint8_t len = (uint8_t)GetBString(data, strBuf, sizeof(strBuf));
		return EdfWriteDataBlock(dw, strBuf, len);
	}
	return EdfWriteDataBlock(dw, data, GetSizeOf(pt));
}
//-----------------------------------------------------------------------------
int EdfWriteStringBytes(EdfWriter_t* dw, char* name, void* str, size_t len)
{
	int err;
	size_t writed = 0;
	if ((err = EdfWriteInfo(dw, &((TypeInfo_t) { .Type = String, .Name = name }), &writed)))
		return err;

	uint8_t strBuf[256] = { 0 };
	if (NULL == str)
		return 0;
	len = MIN(len, strnlen(str, 0xFE));
	strBuf[0] = (uint8_t)len;
	memcpy(strBuf + 1, str, len);
	return EdfWriteDataBlock(dw, strBuf, len + 1);
}
