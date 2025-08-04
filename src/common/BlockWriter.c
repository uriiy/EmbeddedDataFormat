#include "_pch.h"
#include "EdfWriter.h"

//-----------------------------------------------------------------------------
size_t WriteTxtHeaderBlock(const EdfHeader_t* h, Stream_t* stream)
{
	size_t len = (*stream->FWrite)(stream->Instance, "~ version=%d.%d.%d bs=%d encoding=%d flags=%d \n"
		, h->VersMajor, h->VersMinor, h->VersPatch
		, h->Blocksize, h->Encoding, h->Flags);
	return len;
}
//-----------------------------------------------------------------------------
size_t WriteHeaderBlock(const EdfHeader_t* h, DataWriter_t* dw)
{
	dw->Block[0] = (uint8_t)btHeader;
	dw->Block[1] = (uint8_t)dw->Seq++;
	size_t data_len = HeaderToBytes(h, &dw->Block[4]);
	*((uint16_t*)&dw->Block[2]) = (uint16_t)data_len;
	dw->BlockLen = 1 + 1 + 2 + data_len;
	if (dw->BlockLen != (*dw->Stream.Write)(dw->Stream.Instance, dw->Block, dw->BlockLen))
		LOG_ERR();
	data_len = dw->BlockLen;
	dw->BlockLen = 0;
	dw->h = *h;
	return data_len;
}
//-----------------------------------------------------------------------------
size_t WriteTxtVarInfoBlock(const TypeInfo_t* t, DataWriter_t* tw)
{
	if (4 != (*tw->Stream.Write)(tw->Stream.Instance, "\n\n? ", 4))
		LOG_ERR();
	size_t len = ToString(t, tw->Block, 0);
	tw->BlockLen = len;
	if (tw->BlockLen != (*tw->Stream.Write)(tw->Stream.Instance, tw->Block, tw->BlockLen))
		LOG_ERR();
	tw->BlockLen = 0;
	return len;
}
//-----------------------------------------------------------------------------
size_t WriteVarInfoBlock(const TypeInfo_t* t, DataWriter_t* dw)
{
	dw->Block[0] = (uint8_t)btVarInfo;
	dw->Block[1] = (uint8_t)dw->Seq++;
	size_t data_len = ToBytes(t, &dw->Block[4]);
	*((uint16_t*)&dw->Block[2]) = (uint16_t)data_len;
	dw->BlockLen = 1 + 1 + 2 + data_len;
	if (dw->BlockLen != (*dw->Stream.Write)(dw->Stream.Instance, dw->Block, dw->BlockLen))
		LOG_ERR();
	data_len = dw->BlockLen;
	dw->BlockLen = 0;
	dw->t = t;
	return data_len;
}
//-----------------------------------------------------------------------------
size_t FlushBinBlock(Stream_t* s, BlockType t, uint8_t seq, uint8_t* src, size_t len)
{
	if (0 == len)
		return 0;
	uint8_t h[4] = { t, seq };
	*((uint16_t*)&h[2]) = (uint16_t)len;
	if (sizeof(h) != (*s->Write)(s->Instance, h, sizeof(h)))
		LOG_ERR();
	if (len != (*s->Write)(s->Instance, src, len))
		LOG_ERR();
	return 1 + 1 + 2 + len;
}
//-----------------------------------------------------------------------------
size_t FlushTxtBlock(Stream_t* s, BlockType t, uint8_t seq, uint8_t* src, size_t len)
{
	if (0 == len)
		return 0;
	if (len != (*s->Write)(s->Instance, src, len))
		LOG_ERR();
	return len;
}
//-----------------------------------------------------------------------------
size_t FlushDataBlock(DataWriter_t* dw)
{
	if (NULL == dw->FlushBlock || 0 == dw->BlockLen)
		return 0;
	size_t ret = (*dw->FlushBlock)(&dw->Stream, btVarData, dw->Seq, dw->Block, dw->BlockLen);
	dw->Seq++;
	dw->BlockLen = 0;
	return ret;
}
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
			FlushDataBlock(dw);
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
size_t WriteDataBlock(//const TypeInfo_t* t,
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
size_t ReadBlock(DataWriter_t* dw)
{
	size_t len = 0;
	do
	{
		len = (*dw->Stream.Read)(dw->Stream.Instance, dw->Block, 1);
		if (1 == len && IsBlockType(dw->Block[0]))
		{
			len = (*dw->Stream.Read)(dw->Stream.Instance, &dw->Block[1], 3);
			if (3 == len && dw->Block[1] == dw->Seq)
			{
				dw->BlockLen = *(uint16_t*)&dw->Block[2];
				len = (*dw->Stream.Read)(dw->Stream.Instance, &dw->Block[4], dw->BlockLen);
				if (len == dw->BlockLen)
				{
					dw->Seq++;
					dw->BlockLen += 4;
					return dw->BlockLen;
				}
			}
		}
	} while (1 == len);
	return (size_t) - 1;
}
//-----------------------------------------------------------------------------
size_t ReadHeaderBlock(DataWriter_t* dr, EdfHeader_t* h)
{
	if (0 < ReadBlock(dr) && btHeader == dr->Block[0] && 4 + 16 == dr->BlockLen)
	{
		*h = MakeHeaderFromBytes(&dr->Block[4]);
		return 0;
	}
	return (size_t) - 1;
}
//-----------------------------------------------------------------------------