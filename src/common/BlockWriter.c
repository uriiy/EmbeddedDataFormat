#include "_pch.h"
#include "edf.h"

//-----------------------------------------------------------------------------
static int WriteData(const TypeInfo_t* t,
	uint8_t* src, size_t srcLen,
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
					TypeInfo_t* s = &t->Childs.Item[j];
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
	uint8_t* src, size_t srcLen,
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
	int wr = WriteData(dw->t, src, srcLen, dst, dstLen, skip, &wqty, readed, writed, dw);
	dst += *writed; dstLen -= *writed;
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
int EdfWriteDataBlock(EdfWriter_t* dw, void* vsrc, size_t xsrcLen)
{
	uint8_t* xsrc = (uint8_t*)vsrc;
	uint8_t* src = xsrc;
	size_t srcLen = xsrcLen;

	size_t dstLen = sizeof(dw->Block) - dw->BlockLen;
	uint8_t* dst = dw->Block + dw->BlockLen;

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
				memcpy(dw->Buf, src, dw->BufLen);
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
		dw->BlockLen += w;
		if (0 < wr || 0 == dstLen)
		{
			w = 0;
			EdfFlushDataBlock(dw, &w);
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
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int EdfReadBlock(EdfWriter_t* dw)
{
	int err = 0;
	size_t readed = 0;

	uint8_t blockType;
	uint8_t blockseq;
	uint16_t blockLen;

	do
	{
		if (!(err = StreamRead(&dw->Stream, &readed, &blockType, 1))
			&& IsBlockType(blockType))
		{
			if (!(StreamRead(&dw->Stream, &readed, &blockseq, 1))
				&& blockseq == dw->Seq)
			{
				if (!(err = StreamRead(&dw->Stream, &readed, &blockLen, 2))
					&& 4096 > blockLen && BLOCK_SIZE >= blockLen)
				{
					if (!(StreamRead(&dw->Stream, &readed, &dw->Block, blockLen)))
					{
						dw->BlockType = blockType;
						dw->Seq++;
						dw->BlockLen = blockLen;
						return 0;
					}
				}
			}
		}
	} while (!err);
	dw->BlockType = 0;
	dw->BlockLen = 0;
	return err;
}
