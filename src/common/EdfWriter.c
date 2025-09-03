#include "_pch.h"
#include "edf.h"

//-----------------------------------------------------------------------------
int EdfWriteHeader(EdfWriter_t* dw, const EdfHeader_t* h, size_t* writed)
{
	return dw->WriteHeader ? (*dw->WriteHeader)(dw, h, writed) : ERR_FN_NOT_EXIST;
}
//-----------------------------------------------------------------------------
int EdfWriteInfo(EdfWriter_t* dw, const TypeRec_t* t, size_t* writed)
{
	int err = 0;
	size_t flushed = 0;
	if ((err = EdfFlushDataBlock(dw, &flushed)))
		return err;
	dw->Skip = 0;
	return (dw->WriteInfo) ? (*dw->WriteInfo)(dw, t, writed) : ERR_FN_NOT_EXIST
}
//-----------------------------------------------------------------------------
static int EdfWriteHeaderTxt(EdfWriter_t* dw, const EdfHeader_t* h, size_t* writed)
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
static int EdfWriteHeaderBin(EdfWriter_t* dw, const EdfHeader_t* h, size_t* writed)
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
static int EdfWriteInfoTxt(EdfWriter_t* w, const TypeRec_t* t, size_t* writed)
{
	int err = 0;
	if (t->Id)
	{
		if ((err = StreamWriteFmt(&w->Stream, writed, "\n\n?<%lu> ", t->Id)))
			return err;
	}
	else
	{
		if ((err = StreamWrite(&w->Stream, writed, "\n\n? ", 4)))
			return err;
	}
	if ((err = StreamWriteInfoTxt(&w->Stream, &t->Inf, 0, writed)))
		return err;
	w->Seq++;
	w->BlockLen = 0;
	w->t = t;
	return err;
}
//-----------------------------------------------------------------------------
static int EdfWriteInfoBin(EdfWriter_t* dw, const TypeRec_t* t, size_t* writed)
{
	int err = 0;
	dw->Block[0] = (uint8_t)btVarInfo;
	dw->Block[1] = (uint8_t)dw->Seq;
	MemStream_t ms = { 0 };
	if ((err = MemStreamOutOpen(&ms, &dw->Block[4], sizeof(dw->Block) - 4)) ||
		(err = StreamWrite(&ms, writed, &t->Id, FIELD_SIZEOF(TypeRec_t, Id))) ||
		(err = StreamWriteInfoBin((Stream_t*)&ms, &t->Inf, writed)))
		return err;
	*((uint16_t*)&dw->Block[2]) = (uint16_t)ms.WPos;
	dw->BlockLen = 1 + 1 + 2 + ms.WPos;
	if ((err = StreamWrite(&dw->Stream, writed, dw->Block, dw->BlockLen)))
		return err;
	dw->Seq++;
	dw->BlockLen = 0;
	dw->t = t;
	return 0;
}
//-----------------------------------------------------------------------------
static int StreamWriteBlockDataBin(EdfWriter_t* dw, size_t* writed)
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
static int StreamWriteBlockDataTxt(EdfWriter_t* dw, size_t* writed)
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
int EdfOpenStream(EdfWriter_t* f, Stream_t* stream, const char* mode)
{
	if (2 > strnlength(mode, 2))
		return -1;
	int err = 0;
	if (0 == strncmp("wb", mode, 2) || 0 == strncmp("ab", mode, 2))
	{
		f->Stream = *stream;
		f->Seq = 0;
		f->Skip = 0;
		f->BlockLen = 0;
		f->BufLen = 0;
		f->WritePrimitive = strchr(mode, 'c') ? BinToBin : CBinToBin;
		f->WriteHeader = EdfWriteHeaderBin;
		f->WriteInfo = EdfWriteInfoBin;
		f->FlushData = StreamWriteBlockDataBin;
		f->BeginStruct = NULL;
		f->EndStruct = NULL;
		f->BeginArray = NULL;
		f->EndArray = NULL;
		f->SepVarEnd = NULL;
		f->RecBegin = NULL;
		f->RecEnd = NULL;
		if (strchr(mode, 'a'))
		{
			err = SeekEnd(f);
		}
	}
	else if (0 == strncmp("wt", mode, 2) || 0 == strncmp("at", mode, 2))
	{
		f->Stream = *stream;
		f->Seq = 0;
		f->Skip = 0;
		f->BlockLen = 0;
		f->BufLen = 0;
		f->WritePrimitive = strchr(mode, 'c') ? BinToStr : CBinToStr;
		f->WriteHeader = EdfWriteHeaderTxt;
		f->WriteInfo = EdfWriteInfoTxt;
		f->FlushData = StreamWriteBlockDataTxt;
		f->BeginStruct = SepBeginStruct;
		f->EndStruct = SepEndStruct;
		f->BeginArray = SepBeginArray;
		f->EndArray = SepEndArray;
		f->SepVarEnd = SepVarEnd;
		f->RecBegin = SepRecBegin;
		f->RecEnd = SepRecEnd;
		if (strchr(mode, 'a'))
		{
			//err = SreamSeekEnd(stream);
		}
		return err;
	}
	else if (0 == strncmp("rb", mode, 2))
	{
		f->Stream = *stream;
		f->Seq = 0;
		f->Skip = 0;
		f->BlockLen = 0;
		f->BufLen = 0;
		f->WritePrimitive = BinToBin;
		f->WriteHeader = NULL;
		f->WriteInfo = NULL;
		f->FlushData = NULL;
		f->BeginStruct = NULL;
		f->EndStruct = NULL;
		f->BeginArray = NULL;
		f->EndArray = NULL;
		f->SepVarEnd = NULL;
		f->RecBegin = NULL;
		f->RecEnd = NULL;
	}
	if (0 == strncmp("rt", mode, 2))
	{
		err = -1;
	}
	return err;
}
//-----------------------------------------------------------------------------
int EdfOpen(EdfWriter_t* f, const char* file, const char* mode)
{
	if (2 > strnlength(mode, 2))
		return -1;
	int err = 0;
	if (0 == strncmp("wb", mode, 2) || 0 == strncmp("ab", mode, 2))
	{
		err = FileStreamOpen((FileStream_t*)&f->Stream, file, mode);
		if (err)
			return -1;
		return EdfOpenStream(f, &f->Stream, mode);
	}
	else if (0 == strncmp("wt", mode, 2) || 0 == strncmp("at", mode, 2))
	{
		char* filemode;
		if (0 == strncmp("wt", mode, 2))
			filemode = "wb";
		else if (0 == strncmp("at", mode, 2))
			filemode = "ab";
		else
			return -1;
		err = FileStreamOpen((FileStream_t*)&f->Stream, file, filemode);
		if (err)
			return -1;
		return EdfOpenStream(f, &f->Stream, mode);
	}
	else if (0 == strncmp("rb", mode, 2))
	{
		err = FileStreamOpen((FileStream_t*)&f->Stream, file, "rb");
		if (err)
			return -1;
		return EdfOpenStream(f, &f->Stream, mode);
	}
	else if (0 == strncmp("rt", mode, 2))
	{
		return -1;
	}
	return -1;
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
int EdfClose(EdfWriter_t* dw)
{
	int err = 0;
	size_t w = 0;
	if ((err = EdfFlushDataBlock(dw, &w)))
		return err;
	if (dw->Stream.Close)
		err = (*dw->Stream.Close)(&dw->Stream);
	return err;
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
	size_t srcLen = src ? strnlength(src, 10) : 0;
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
int EdfWriteInfRecData(EdfWriter_t* dw, uint32_t id, PoType pt, char* name, void* d)
{
	int err;
	size_t writed = 0;
	if ((err = EdfWriteInfo(dw, &((const TypeRec_t) { id, { .Type = pt, .Name = name } }), &writed)))
		return err;
	void* data = NULL;
	if (String == pt)
		data = &d;
	else
		data = d;
	return EdfWriteDataBlock(dw, data, GetSizeOf(pt));
}
//-----------------------------------------------------------------------------
int EdfWriteInfData(EdfWriter_t* dw, PoType pt, char* name, void* d)
{
	return EdfWriteInfRecData(dw, 0, pt, name, d);
}
//-----------------------------------------------------------------------------
int EdfWriteInfRecStringData(EdfWriter_t* dw, uint32_t id, char* name, void* str, size_t len)
{
	int err;
	size_t writed = 0;
	if ((err = EdfWriteInfo(dw, &((const TypeRec_t) { id, { .Type = String, .Name = name } }), &writed)))
		return err;
	if (NULL == str)
		return 0;
	void* data = NULL;
	uint8_t strBuf[256] = { 0 };
	const size_t dataLen = strnlength(str, 0xFE);
	if (dataLen > len)
	{
		memcpy(strBuf, str, len);
		strBuf[len] = '\0';
		data = strBuf;
	}
	else
		data = str;
	return EdfWriteDataBlock(dw, &data, GetSizeOf(String));
}
//-----------------------------------------------------------------------------
int EdfWriteStringBytes(EdfWriter_t* dw, char* name, void* str, size_t len)
{
	return EdfWriteInfRecStringData(dw, 0, name, str, len);
}
//-----------------------------------------------------------------------------