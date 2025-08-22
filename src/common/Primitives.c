#include "_pch.h"
#include "Primitives.h"

//-----------------------------------------------------------------------------
size_t GetBString(const char* str, uint8_t* dst, size_t dst_len)
{
	if (NULL == str)
		return 0;
	size_t len = strnlength(str, 0xFE);
	if (len > dst_len)
		return 0;
	dst[0] = (uint8_t)len;
	memcpy(dst + 1, str, len);
	return len + 1;
}
//-----------------------------------------------------------------------------
size_t GetCString(const char* str, uint32_t arr_len, uint8_t* dst, size_t dst_len)
{
	if (NULL == str)
		return 0;
	size_t len = strnlength(str, 0xFE - 1) + 1;
	if (0 == len || len > dst_len)
		return 0;
	memcpy(dst, str, len);
	memset(dst + len, 0, arr_len - len);
	return arr_len;
}
//-----------------------------------------------------------------------------
int BinToBin(PoType t,
	uint8_t* src, size_t srcLen,
	uint8_t* dst, size_t dstLen,
	size_t* r, size_t* w)
{
	*r = *w = GetSizeOf(t);
	if (srcLen < *r)
		return -1;
	if (dstLen < *w)
		return 1;
	switch (t)
	{
	case Struct:
	default: *r = *w = 0; return -2;
	case Int8:
	case UInt8:
	case UInt16:
	case UInt32:
	case UInt64:
	case Int16:
	case Int32:
	case Int64:
	case Half:
	case Single:
	case Double:
		memcpy(dst, src, *r);
		break;
	case Char:
		if (dstLen < srcLen)
		{
			*r = *w = 0;
			return 1;
		}
		*r = *w = srcLen;
		memcpy(dst, src, srcLen);
		return 0;
	case String:
	{
		size_t sLen = src[0];
		size_t blength = 1;
		*r = *w = 0;
		blength += sLen;
		if (srcLen < blength)
			return -1;
		if (dstLen < blength)
			return 1;
		memcpy(dst, src, blength);
		*r = *w = blength;
	}
	return 0;
	case CString:
	{
		char* str = *(char**)src;
		size_t len = ((NULL == str) ? 0 : strnlength(str, 0xFE));
		if (dstLen < len + 1)
			return 1;
		(*dst) = (uint8_t)len;
		dst++;
		memcpy(dst, str, len);
		*w = len + 1;
	}
	return 0;
	}//switch
	return 0;
}
//-----------------------------------------------------------------------------
static size_t xprint(const uint8_t* buf, size_t bufLen, char* format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	int writed = vsnprintf((char*)buf, bufLen, format, arglist);
	va_end(arglist);
	if (writed == (int)bufLen)
		return writed + 1;
	return writed;
}
//-----------------------------------------------------------------------------
int BinToStr(PoType t,
	uint8_t* src, size_t srcLen,
	uint8_t* dst, size_t dstLen,
	size_t* r, size_t* w)
{
	*r = GetSizeOf(t);
	if (srcLen < *r)
	{
		*w = 0;
		return -1;
	}
	if (dstLen < 1)
	{
		*w = 0;
		return 1;
	}
	switch (t)
	{
	case Struct:
	default: *r = *w = 0; return -2;
	case Int8:
		*w = xprint(dst, dstLen, "%d", (int8_t)src[0]);
		return (dstLen < *w);
	case UInt8:
		*w = xprint(dst, dstLen, "%u", (uint8_t)src[0]);
		return (dstLen < *w);
	case Int16:
		*w = xprint(dst, dstLen, "%d", *((int16_t*)src));
		return (dstLen < *w);
	case UInt16:
		*w = xprint(dst, dstLen, "%u", *((uint16_t*)src));
		return (dstLen < *w);
	case Int32:
		*w = xprint(dst, dstLen, "%d", *((int32_t*)src));
		return (dstLen < *w);
	case UInt32:
		*w = xprint(dst, dstLen, "%lu", *((uint32_t*)src));
		return (dstLen < *w);
	case Int64:
		*w = xprint(dst, dstLen, "%lld", *((int64_t*)src));
		return (dstLen < *w);
	case UInt64:
		*w = xprint(dst, dstLen, "%llu", *((uint64_t*)src));
		return (dstLen < *w);
	case Half:
		//*w = sprintf_s(dst, dstLen, "%g", *((uint16_t*)src));
		return 0;
	case Single:
		*w = xprint(dst, dstLen, "%g", *((float*)src));
		return (dstLen < *w);
	case Double:
		*w = xprint(dst, dstLen, "%g", *((double*)src));
		return (dstLen < *w);
	case Char:
		if (dstLen < (*w) + 2)
			return 1;
		*r = srcLen;
		*w = srcLen + 2;
		dst[0] = '"';
		memcpy(dst + 1, src, srcLen);
		dst[srcLen + 2 - 1] = '"';
		return 0;
	case String:
		if ((size_t)2 + src[0] > dstLen)
			return 1;
		*r = 1 + src[0];
		*w = 2 + src[0];
		*dst++ = '"';
		memcpy(dst, src + 1, src[0]);
		dst += src[0];
		*dst++ = '"';
		return 0;
	case CString:
	{
		// print text without buf
		char* str = *(char**)src;
		size_t len = ((NULL == str) ? 0 : strnlength(str, 0xFE));
		if (dstLen < len + 2)
			return 1;
		*w = 2 + len;
		*dst++ = '"';
		memcpy(dst, str, len);
		dst += len;
		*dst++ = '"';
		return 0;
	}
	break;//case CString:
	}//switch (t)
}