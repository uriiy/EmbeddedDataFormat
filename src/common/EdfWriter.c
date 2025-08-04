#include "_pch.h"
#include "EdfWriter.h"

//-----------------------------------------------------------------------------
int NoWrite(uint8_t** dst, size_t* dstLen, size_t* w)
{
	return 0;
}
//-----------------------------------------------------------------------------
int SepBeginStruct(uint8_t** dst, size_t* dstLen, size_t* w)
{
	if (1 > *dstLen)
		return 1;
	--(*dstLen);
	++(*w);
	(*dst)[0] = '{';
	++(*dst);
	return 0;
}
//-----------------------------------------------------------------------------
int SepEndStruct(uint8_t** dst, size_t* dstLen, size_t* w)
{
	if (1 > *dstLen)
		return 1;
	--(*dstLen);
	++(*w);
	(*dst)[0] = '}';
	++(*dst);
	return 0;
}
//-----------------------------------------------------------------------------
int SepBeginArray(uint8_t** dst, size_t* dstLen, size_t* w)
{
	if (1 > *dstLen)
		return 1;
	--(*dstLen);
	++(*w);
	(*dst)[0] = '[';
	++(*dst);
	return 0;
}
//-----------------------------------------------------------------------------
int SepEndArray(uint8_t** dst, size_t* dstLen, size_t* w)
{
	if (1 > *dstLen)
		return 1;
	--(*dstLen);
	++(*w);
	(*dst)[0] = ']';
	++(*dst);
	return 0;
}
//-----------------------------------------------------------------------------
int SepVar(uint8_t** dst, size_t* dstLen, size_t* w)
{
	if (1 > *dstLen)
		return 1;
	--(*dstLen);
	++(*w);
	(*dst)[0] = ';';
	++(*dst);
	return 0;
}
//-----------------------------------------------------------------------------
int SepRecBegin(uint8_t** dst, size_t* dstLen, size_t* w)
{
	if (1 > *dstLen)
		return 1;
	(*dstLen) -= 3;
	(*w) += 3;
	(*dst)[0] = '\n';
	(*dst)[1] = '=';
	(*dst)[2] = ' ';
	(*dst) += 3;
	return 0;
}
//-----------------------------------------------------------------------------
int SepRecEnd(uint8_t** dst, size_t* dstLen, size_t* w)
{
	return 0;
}