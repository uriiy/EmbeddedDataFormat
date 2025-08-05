#include "_pch.h"
#include "edf.h"





//-----------------------------------------------------------------------------
static int WriteData(const TypeInfo_t* t,
	uint8_t* src, size_t srcLen,
	uint8_t* dst, size_t dstLen,
	size_t* skip, size_t* wqty,
	size_t* readed, size_t* writed,
	DataWriter_t* dw)
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
		if (*skip >= totalElement)
		{
			*skip -= (size_t)totalElement;
			return 0;
		}
		size_t r = 0, w = 0;
		int wr = (*dw->WritePrimitive)(t->Type,
			src, totalElement,
			dst, dstLen,
			&r, &w);
		if (0 == wr)
		{
			*wqty += totalElement;
			*readed += r;
			*writed += w;
			src += r; srcLen -= r;
			dst += w; dstLen -= w;
			(*dw->SepVar)(&dst, &dstLen, writed);
		}
		return wr;
	}
	if (0 == *skip && 1 < totalElement)
		(*dw->SepBeginArray)(&dst, &dstLen, writed);
	for (size_t i = 0; i < totalElement; i++)
	{
		if (Struct == t->Type)
		{
			if (t->Childs.Count)
			{
				if (0 == *skip)
					(*dw->SepBeginStruct)(&dst, &dstLen, writed);
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
				if (0 == *skip)
					(*dw->SepEndStruct)(&dst, &dstLen, writed);
			}
		}
		else
		{
			if (0 < *skip)
			{
				(*skip)--;
				(*wqty)++;
				continue;
			}
			size_t r = 0, w = 0;
			int wr = (*dw->WritePrimitive)(t->Type, src, srcLen, dst, dstLen, &r, &w);
			if (0 == wr)
			{
				(*wqty)++;
				*readed += r;
				*writed += w;
				src += r; srcLen -= r;
				dst += w; dstLen -= w;
				(*dw->SepVar)(&dst, &dstLen, writed);
			}
			else
				return wr;
		}
	}
	if (0 == *skip && 1 < totalElement)
		(*dw->SepEndArray)(&dst, &dstLen, writed);
	return 0;
}
//-----------------------------------------------------------------------------
static int WriteSingleValue(
	uint8_t* src, size_t srcLen,
	uint8_t* dst, size_t dstLen,
	size_t* skip,
	size_t* readed, size_t* writed,
	DataWriter_t* dw)
{
	size_t wqty = 0;
	*readed = *writed = 0;
	if (0 == srcLen)
		return -1;
	if (0 == *skip)
		(*dw->SepRecBegin)(&dst, &dstLen, writed);
	int wr = WriteData(dw->t, src, srcLen, dst, dstLen, skip, &wqty, readed, writed, dw);
	dst += *writed; dstLen -= *writed;
	if (0 == wr)
	{
		(*dw->SepRecEnd)(&dst, &dstLen, writed);
		*skip = 0;
	}
	else
	{
		*skip = wqty;
	}
	return wr;
}
//-----------------------------------------------------------------------------
static int WriteMultipleValues(//const TypeInfo_t* t,
	uint8_t* xsrc, size_t xsrcLen,
	uint8_t* xdst, size_t xdstLen,
	size_t* readed, size_t* writed,
	DataWriter_t* dw)
{
	uint8_t* dst = xdst;
	size_t dstLen = xdstLen;
	*readed = *writed = 0;
	int wr;
	do
	{
		// copy xsrc data to buffer
		size_t len = MIN(sizeof(dw->Buf) - dw->BufLen, xsrcLen);
		if (0 < len)
		{
			memcpy(dw->Buf + dw->BufLen, xsrc, len);
			xsrc += len;
			xsrcLen -= len;
			dw->BufLen += len;
			(*readed) += len;
			// add copy
		}

		size_t r = 0, w = 0;
		wr = WriteSingleValue(dw->Buf, dw->BufLen, dst, dstLen, &dw->Skip, &r, &w, dw);
		//readed += r;
		*writed += w;
		dw->BufLen -= r;
		memcpy(dw->Buf, dw->Buf + r, dw->BufLen);
		dst += w; dstLen -= w;
		if (0 < wr)
		{
			dw->BlockLen = w;
			EdfFlushDataBlock(dw);
			wr = 0;
			dst = xdst; dstLen = xdstLen;
		}
		else
		{
			dw->BlockLen += w;
		}
	} while (0 == wr && 0 < dw->BufLen);
	return wr;
}
//-----------------------------------------------------------------------------
size_t EdfWriteDataBlock(//const TypeInfo_t* t,
	uint8_t* src, size_t srcLen,
	DataWriter_t* dw)
{
	int ret = 0;
	size_t r = 0; size_t w = 0;
	do
	{
		size_t dstLen = sizeof(dw->Block) - dw->BlockLen;
		uint8_t* dst = dw->Block + dw->BlockLen;

		ret = WriteMultipleValues(src, srcLen, dst, dstLen, &r, &w, dw);
		src += r; srcLen -= r;
		dst += w; dstLen -= w;

	} while (0 == ret && 0 < srcLen);
	return ret;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
size_t EdfReadBlock(DataWriter_t* dw)
{
	size_t len = 0;
	do
	{
		len = StreamRead(&dw->Stream, dw->Block, 1);
		if (1 == len && IsBlockType(dw->Block[0]))
		{
			len = StreamRead(&dw->Stream, &dw->Block[1], 3);
			if (3 == len && dw->Block[1] == dw->Seq)
			{
				dw->BlockLen = *(uint16_t*)&dw->Block[2];
				len = StreamRead(&dw->Stream, &dw->Block[4], dw->BlockLen);
				if (len == dw->BlockLen)
				{
					dw->Seq++;
					dw->BlockLen += 4;
					return dw->BlockLen;
				}
			}
		}
	} while (1 == len);
	return (size_t)-1;
}
