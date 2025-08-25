#include "_pch.h"
#include "KeyValue.h"

//-----------------------------------------------------------------------------
static int DeSerializeUInt16KeyValItem(const uint8_t* src, size_t srcLen,
	UInt16Value_t* s, size_t* dstLen)
{
	size_t offset = 0;
	uint8_t len = 0;

	if (srcLen < 1 + offset)
		return -1;
	len = MIN(src[offset], (uint8_t)0xFE);
	if (srcLen < 1 + len + offset)
		return -1;
	s->Name = (char*)&src[1];
	offset += 1 + len;

	len = sizeof(uint16_t);
	if (srcLen < len + offset)
		return -1;
	s->Value = *((uint16_t*)&src[offset]);
	offset += len;

	len = MIN(src[offset], (uint8_t)0xFE);
	if (srcLen < 1 + len + offset)
		return -1;
	s->Unit = (char*)&src[1 + offset];
	offset += 1 + len;

	len = MIN(src[offset], (uint8_t)0xFE);
	if (srcLen < 1 + len + offset)
		return -1;
	s->Description = (char*)&src[1 + offset];
	offset += 1 + len;

	*dstLen = offset;
	return 0;
}
//-----------------------------------------------------------------------------
int DeSerializeUInt16KeyVal(uint8_t* psrc, const uint8_t* const psrcEnd,
	uint8_t** ppbuf, uint8_t* const pbufBegin, uint8_t* const pbufEnd,
	DoOnItemUInt16 DoOnItem, void* state)
{
	uint8_t* pbuf = *ppbuf;
	uint8_t* rBegin = NULL;
	size_t itemlen = 0;
	UInt16Value_t s;
	do
	{
		size_t len = MIN(pbufEnd - pbuf, psrcEnd - psrc);
		if (0 < len)
		{
			memcpy(pbuf, psrc, len);
			psrc += len;
			pbuf += len;
		}
		rBegin = pbufBegin;
		while (0 == DeSerializeUInt16KeyValItem(rBegin, pbuf - rBegin, &s, &itemlen))
		{
			rBegin += itemlen;
			(*DoOnItem)(s, state);
		}
		memcpy(pbufBegin, rBegin, pbuf - rBegin);// move to buffer begin
		pbuf = pbufBegin + (pbuf - rBegin);// set pointer to begin data
	} while (psrcEnd > psrc);

	*ppbuf = pbuf;
	return 0;
}



//-----------------------------------------------------------------------------
static int DeSerializeDoubleKeyValItem(const uint8_t* src, size_t srcLen,
	DoubleValue_t* s, size_t* dstLen)
{
	size_t offset = 0;
	uint8_t len = 0;

	if (srcLen < 1 + offset)
		return -1;
	len = MIN(src[offset], (uint8_t)0xFE);
	if (srcLen < 1 + len + offset)
		return -1;
	s->Name = (char*)&src[1];
	offset += 1 + len;

	len = sizeof(double);
	if (srcLen < len + offset)
		return -1;
	s->Value = *((double*)&src[offset]);
	offset += len;

	len = MIN(src[offset], (uint8_t)0xFE);
	if (srcLen < 1 + len + offset)
		return -1;
	s->Unit = (char*)&src[1 + offset];
	offset += 1 + len;

	len = MIN(src[offset], (uint8_t)0xFE);
	if (srcLen < 1 + len + offset)
		return -1;
	s->Description = (char*)&src[1 + offset];
	offset += 1 + len;

	*dstLen = offset;
	return 0;
}
//-----------------------------------------------------------------------------
int DeSerializeDoubleKeyVal(uint8_t* psrc, const uint8_t* const psrcEnd,
	uint8_t** ppbuf, uint8_t* const pbufBegin, uint8_t* const pbufEnd,
	DoOnItemDoubleFn DoOnItem, void* state)
{
	uint8_t* pbuf = *ppbuf;
	uint8_t* rBegin = NULL;
	size_t itemlen = 0;
	DoubleValue_t s;
	do
	{
		size_t len = MIN(pbufEnd - pbuf, psrcEnd - psrc);
		if (0 < len)
		{
			memcpy(pbuf, psrc, len);
			psrc += len;
			pbuf += len;
		}
		rBegin = pbufBegin;
		while (0 == DeSerializeDoubleKeyValItem(rBegin, pbuf - rBegin, &s, &itemlen))
		{
			rBegin += itemlen;
			(*DoOnItem)(s, state);
		}
		memcpy(pbufBegin, rBegin, pbuf - rBegin);// move to buffer begin
		pbuf = pbufBegin + (pbuf - rBegin);// set pointer to begin data
	} while (psrcEnd > psrc);
	*ppbuf = pbuf;
	return 0;
}