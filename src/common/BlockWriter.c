#include "_pch.h"
#include "edf.h"

//-----------------------------------------------------------------------------
static int WriteData(const TypeInfo_t* t,
	const uint8_t* src, size_t srcLen,
	uint8_t* dst, size_t dstLen,
	size_t* skip, size_t* wqty,
	size_t* readed, size_t* writed,
	EdfWriter_t* dw)
{
	size_t totalElement = 1;
	for (size_t i = 0; i < t->Dims.Count; i++)
		totalElement *= t->Dims.Item[i];
	if (Char == t->Type)
	{
		if (srcLen < totalElement)
			return -1;
		if (dstLen < totalElement)
			return 1;
		if (totalElement <= *skip)
			*skip -= totalElement;
		{
			size_t r = 0, w = 0;
			int wr = (*dw->WritePrimitive)(t->Type, src, totalElement, dst, dstLen, &r, &w);
			if (wr)
				return wr;
			*wqty += totalElement;
			*readed += r;
			*writed += w;
			src += r; srcLen -= r;
			dst += w; dstLen -= w;
		}
		if ((EdfWriteSep(dw->SepVarEnd, &dst, &dstLen, skip, wqty, writed)))
			return 1;
		return 0;
	}
	if (1 < totalElement)
	{
		if ((EdfWriteSep(dw->BeginArray, &dst, &dstLen, skip, wqty, writed)))
			return 1;
	}
	for (size_t i = 0; i < totalElement; i++)
	{
		if (Struct == t->Type)
		{
			if (t->Childs.Count)
			{
				if ((EdfWriteSep(dw->BeginStruct, &dst, &dstLen, skip, wqty, writed)))
					return 1;
				for (size_t j = 0; j < t->Childs.Count; j++)
				{
					const TypeInfo_t* s = &t->Childs.Item[j];
					size_t r = 0, w = 0;
					int wr = WriteData(s, src, srcLen, dst, dstLen, skip, wqty, &r, &w, dw);
					*readed += r;
					*writed += w;
					if (0 != wr)
						return wr;
					src += r; srcLen -= r;
					dst += w; dstLen -= w;
				}
				if ((EdfWriteSep(dw->EndStruct, &dst, &dstLen, skip, wqty, writed)))
					return 1;
			}
		}
		else
		{
			if (0 < (*skip))
				(*skip)--;
			else
			{
				size_t r = 0, w = 0;
				int wr = (*dw->WritePrimitive)(t->Type, src, srcLen, dst, dstLen, &r, &w);
				if (wr)
					return wr;
				(*wqty)++;
				*readed += r;
				*writed += w;
				src += r; srcLen -= r;
				dst += w; dstLen -= w;
			}
			if ((EdfWriteSep(dw->SepVarEnd, &dst, &dstLen, skip, wqty, writed)))
				return 1;
		}
	}
	if (1 < totalElement)
	{
		if ((EdfWriteSep(dw->EndArray, &dst, &dstLen, skip, wqty, writed)))
			return 1;
	}
	return 0;
}
//-----------------------------------------------------------------------------
static int WriteSingleValue(
	const uint8_t* src, size_t srcLen,
	uint8_t* dst, size_t dstLen,
	size_t* skip,
	size_t* readed, size_t* writed,
	EdfWriter_t* dw)
{
	size_t prevskip = *skip;
	size_t wqty = 0;
	*readed = *writed = 0;
	if (0 == srcLen)
		return -1;
	if ((EdfWriteSep(dw->RecBegin, &dst, &dstLen, skip, &wqty, writed)))
		return 1;
	size_t w = 0;
	int wr = WriteData(&dw->t->Inf, src, srcLen, dst, dstLen, skip, &wqty, readed, &w, dw);
	dst += w; dstLen -= w; *writed += w;
	if (0 > wr)
	{
		*skip = prevskip + wqty;
	}
	else if (0 < wr)
	{
		*skip = prevskip + wqty;
	}
	else
	{
		if ((EdfWriteSep(dw->RecEnd, &dst, &dstLen, skip, &wqty, writed)))
		{
			wr = 1;
			*skip = prevskip + wqty;
		}
		else
			*skip = 0;
	}
	return wr;
}
//-----------------------------------------------------------------------------
int EdfWriteDataBlock(EdfWriter_t* dw, const void* vsrc, size_t xsrcLen)
{
	const uint8_t* xsrc = (const uint8_t*)vsrc;
	const uint8_t* src = xsrc;
	size_t srcLen = xsrcLen;

	size_t dstLen = sizeof(dw->Block) - dw->DatLen;
	uint8_t* dst = dw->Block + dw->DatLen;

	size_t r = 0, w = 0;
	int wr;
	do
	{
		if (dw->BufLen)
		{
			// copy xsrc data to buffer
			size_t len = MIN(sizeof(dw->Buf) - dw->BufLen, xsrcLen);
			if (0 < len)
			{
				memcpy(dw->Buf + dw->BufLen, xsrc, len);
				xsrc += len;
				xsrcLen -= len;
				dw->BufLen += len;

				src = dw->Buf;
				srcLen = dw->BufLen;
			}
		}
		else
		{
			src = xsrc;
			srcLen = xsrcLen;
		}

		wr = WriteSingleValue(src, srcLen, dst, dstLen, &dw->Skip, &r, &w, dw);

		if (dw->BufLen)
		{
			dw->BufLen -= r;
			if (dw->BufLen)
			{
				memcpy(dw->Buf, src + r, dw->BufLen);
				src = dw->Buf;
				srcLen = dw->BufLen;
			}
			else
			{
				src = xsrc;
				srcLen = xsrcLen;
			}
			if (0 > wr && 0 < xsrcLen)
				wr = 0;
		}
		else
		{
			xsrc += r;
			xsrcLen -= r;
			src = xsrc;
			srcLen = xsrcLen;
		}


		dst += w; dstLen -= w;
		dw->DatLen += (uint16_t)w;
		if (0 < wr || 0 == dstLen)
		{
			w = 0;
			int err = EdfFlushDataBlock(dw, &w);
			if (err)
				return err;
			wr = 0;
			dst -= w;
			dstLen += w;
		}
	} while (0 == wr && 0 < srcLen);

	if (0 > wr && 0 < srcLen)
	{
		dw->BufLen = srcLen;
		memcpy(dw->Buf, src, dw->BufLen);
	}


	return wr;
}
//-----------------------------------------------------------------------------
int EdfReadBin(const TypeInfo_t* t, MemStream_t* src, MemStream_t* mem, void** presult,
	int* skip)
{
	if (!IsPoType(t->Type))
		return -2;

	size_t itemCLen = GetTypeCSize(t);
	int err = 0;
	uint8_t* ti = NULL;
	if (*presult)
		ti = *presult;
	else
	{
		if ((err = MemAlloc(mem, itemCLen, (void**)&ti)))
			return 1;
		*presult = ti;
	}

	switch (t->Type)
	{
	case Struct:
		if (t->Childs.Count)
		{
			for (size_t j = 0; j < t->Childs.Count; j++)
			{
				const TypeInfo_t* s = &t->Childs.Item[j];
				size_t childCLen = GetTypeCSize(s);
				if ((err = EdfReadBin(s, src, mem, (void**)&ti, skip)))
					return err;
				ti += childCLen;
			}
		}
		break;
	case String:
	{
		if (0 <= ++(*skip))
			if ((err = StreamReadString(src, mem, (char**)ti)))
				return err;
		ti += itemCLen;
	}
	break;
	default:
		if (0 <= ++(*skip))
			if ((err = StreamRead(src, NULL, ti, itemCLen)))
				return -1;
		ti += itemCLen;
		break;
	}//switch
	return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int EdfReadBlock(EdfWriter_t* dw)
{
	int err = 0;
	size_t readed = 0;

	dw->BlkType = 0;
	dw->DatLen = 0;

	if ((err = StreamRead(&dw->Stream, &readed, &dw->BlkType, 1)))
		return err;
	if (!IsBlockType(dw->BlkType))
		return ERR_BLK_WRONG_TYPE;

	uint8_t blockseq;
	if ((err = StreamRead(&dw->Stream, &readed, &blockseq, 1)))
		return err;
	if (blockseq != dw->BlkSeq)
		return ERR_BLK_WRONG_SEQ;

	if ((err = StreamRead(&dw->Stream, &readed, &dw->DatLen, 2)))
		return err;
	if (4096 < dw->DatLen || BLOCK_SIZE < dw->DatLen)
		return ERR_BLK_WRONG_SIZE;

	if ((err = StreamRead(&dw->Stream, &readed, &dw->Block, dw->DatLen)))
		return err;

	if (btHeader == dw->BlkType)
		memcpy(&dw->h, &dw->Block, sizeof(EdfHeader_t));

	if (dw->h.Flags & UseCrc)
	{
		uint16_t crcData = MbCrc16(&dw->BlkType, 4 + dw->DatLen);
		uint16_t crcFile = 0;
		if ((err = StreamRead(&dw->Stream, &readed, &crcFile, sizeof(uint16_t))))
			return err;
		if (crcData != crcFile)
			return ERR_BLK_WRONG_CRC;
	}
	dw->BlkSeq++;
	return 0;

}
