#include "_pch.h"
#include "edf.h"

//-----------------------------------------------------------------------------
typedef struct EdfBlock
{
	uint8_t Type;
	uint8_t Seq;
	uint16_t Len;
	uint8_t* Data;
} EdfBlock_t;
//-----------------------------------------------------------------------------
static int EdfWriteBlockBin(Stream_t* st, const EdfHeader_t* cfg, const EdfBlock_t* blk, size_t* writed)
{
	int err = 0;
	if ((err = StreamWrite(st, NULL, blk, 4 + blk->Len)))
		return err;
	if (cfg->Flags & UseCrc)
	{
		uint16_t crc = MbCrc16(blk, 4 + blk->Len);
		if ((err = StreamWrite(st, NULL, &crc, sizeof(uint16_t))))
			return err;
	}
	*writed = blk->Len;
	return 0;
}
//-----------------------------------------------------------------------------

// Write Header
//-----------------------------------------------------------------------------
int EdfWriteHeader(EdfWriter_t* dw, const EdfHeader_t* h, size_t* writed)
{
	if (!dw->WriteHeader || !h)
		return ERR_FN_NOT_EXIST;
	dw->BlkSeq = 0;
	dw->h = *h;
	int err = (*dw->WriteHeader)(dw, h, writed);
	if (err)
	{
		LOG_ERR();
		return err;
	}
	dw->BlkSeq++;
	dw->DatLen = 0;
	return err;
}
//-----------------------------------------------------------------------------
static int EdfWriteHeaderBin(EdfWriter_t* dw, const EdfHeader_t* h, size_t* writed)
{
	dw->BlkType = (uint8_t)btHeader;
	dw->DatLen = (uint16_t)HeaderToBytes(h, dw->Block);
	return EdfWriteBlockBin(&dw->Stream, h, (EdfBlock_t*)&dw->BlkType, writed);
}
//-----------------------------------------------------------------------------
static int EdfWriteHeaderTxt(EdfWriter_t* dw, const EdfHeader_t* h, size_t* writed)
{
	return StreamWriteFmt(&dw->Stream, writed, "<~ {version=%d.%d; bs=%d; encoding=%d; flags=%d; } >\n"
		, h->VersMajor, h->VersMinor
		, h->Blocksize, h->Encoding, h->Flags);
}

// Write Info
//-----------------------------------------------------------------------------
int EdfWriteInfo(EdfWriter_t* dw, const TypeRec_t* t, size_t* writed)
{
	int err = 0;
	size_t flushed = 0;
	if ((err = EdfFlushDataBlock(dw, &flushed)))
		return err;
	dw->Skip = 0;

	if (!dw->WriteInfo || !t)
		return ERR_FN_NOT_EXIST;
	err = (*dw->WriteInfo)(dw, t, writed);
	if (err)
	{
		LOG_ERR();
		return err;
	}
	dw->t = t;
	//dw->TypeFlag |= HasDynamicFields(&t->Inf);
	//dw->TypeLen = GetTypeCSize(&t->Inf);
	dw->BlkSeq++;
	dw->DatLen = 0;
	return err;
}
//-----------------------------------------------------------------------------
static int EdfWriteInfoBin(EdfWriter_t* dw, const TypeRec_t* t, size_t* writed)
{
	int err = 0;
	dw->BlkType = (uint8_t)btVarInfo;
	MemStream_t ms = { 0 };
	size_t w = 0;
	if ((err = MemStreamOutOpen(&ms, dw->Block, sizeof(dw->Block))) ||
		(err = StreamWriteInfBin((Stream_t*)&ms, t, &w)))
		return err;
	dw->DatLen = (uint16_t)w;// (uint16_t)ms.WPos;
	if ((EdfWriteBlockBin(&dw->Stream, &dw->h, (EdfBlock_t*)&dw->BlkType, writed)))
		return err;
	return 0;
}
//-----------------------------------------------------------------------------
static int EdfWriteInfoTxt(EdfWriter_t* w, const TypeRec_t* t, size_t* writed)
{
	return StreamWriteInfTxt(&w->Stream, t, writed);
}

// Write Data
//-----------------------------------------------------------------------------
int EdfFlushDataBlock(EdfWriter_t* dw, size_t* writed)
{
	if (NULL == dw->FlushData || 0 == dw->DatLen)
		return 0;
	int err = (*dw->FlushData)(dw, writed);
	if (err)
	{
		LOG_ERR();
		return err;
	}
	dw->BlkSeq++;
	dw->DatLen = 0;
	return err;
}
//-----------------------------------------------------------------------------
static int StreamWriteBlockDataBin(EdfWriter_t* dw, size_t* writed)
{
	dw->BlkType = (uint8_t)btVarData;
	return EdfWriteBlockBin(&dw->Stream, &dw->h, (EdfBlock_t*)&dw->BlkType, writed);
}
//-----------------------------------------------------------------------------
static int StreamWriteBlockDataTxt(EdfWriter_t* dw, size_t* writed)
{
	return StreamWrite((Stream_t*)&dw->Stream, writed, dw->Block, dw->DatLen);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static int SeekEnd(EdfWriter_t* f)
{
	int err = 0;
	while (!(err = EdfReadBlock(f)))
	{
		switch (f->BlkType)
		{
		default: break;
		case btHeader:
			if (16 == f->DatLen)
			{
				err = MakeHeaderFromBytes(f->Block, f->DatLen, &f->h);
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
	f->t = NULL;
	memset(&f->h, 0, sizeof(EdfHeader_t));
	if (0 == strncmp("wb", mode, 2) || 0 == strncmp("ab", mode, 2))
	{
		f->Stream = *stream;
		f->BlkSeq = 0;
		f->Skip = 0;
		f->DatLen = 0;
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
		f->BlkSeq = 0;
		f->Skip = 0;
		f->DatLen = 0;
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
		f->BlkSeq = 0;
		f->Skip = 0;
		f->DatLen = 0;
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
int EdfClose(EdfWriter_t* dw)
{
	int err = 0;
	size_t w = 0;
	if ((err = EdfFlushDataBlock(dw, &w)))
		return err;
	return StreamClose(&dw->Stream);
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
int EdfWriteInfData(EdfWriter_t* dw, uint32_t id, PoType pt, char* name, void* d)
{
	int err;
	size_t writed = 0;
	if ((err = EdfWriteInfo(dw, &((const TypeRec_t)
	{
		.Id = id, .Inf = (TypeInfo_t){ .Type = pt, }, .Name = name
	}), &writed)))
		return err;
	void* data = NULL;
	if (String == pt)
		data = &d;
	else
		data = d;
	return EdfWriteDataBlock(dw, data, GetSizeOf(pt));
}
//-----------------------------------------------------------------------------
int EdfWriteInfDataString(EdfWriter_t* dw, uint32_t id, char* name, void* str, size_t len)
{
	int err;
	size_t writed = 0;
	if ((err = EdfWriteInfo(dw, &((const TypeRec_t)
	{
		.Id = id, .Inf = (TypeInfo_t){ .Type = String, }, .Name = name
	}), &writed)))
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