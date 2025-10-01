#include "_pch.h"
#include "EdfHeader.h"

//-----------------------------------------------------------------------------
EdfHeader_t MakeHeaderDefault(void)
{
	EdfHeader_t h = { 1,0, 65001,BLOCK_SIZE, Default | UseCrc };
	return h;
}
//-----------------------------------------------------------------------------
int MakeHeaderFromBytes(const uint8_t* b, size_t srcSize, EdfHeader_t* h)
{
	if (16 > srcSize)
		return ERR_SRC_SHORT;

	*h = (EdfHeader_t)
	{
		.VersMajor = b[0],
		.VersMinor = b[1],
		.Encoding = *((uint16_t*)&b[2]),
		.Blocksize = *((uint16_t*)&b[4]),
		.Flags = (Options_t)(*((uint32_t*)&b[6])),
	};
	return ERR_NO;
}
//-----------------------------------------------------------------------------
size_t HeaderToBytes(const EdfHeader_t* h, uint8_t* b)
{
	memset(&b, 0, 16);
	b[0] = h->VersMajor;
	b[1] = h->VersMinor;
	*((uint16_t*)&b[2]) = h->Encoding;
	*((uint16_t*)&b[4]) = h->Blocksize;
	*((uint32_t*)&b[6]) = h->Flags;
	return 16;
}