#include "_pch.h"
#include "EdfHeader.h"

//-----------------------------------------------------------------------------
EdfHeader_t MakeHeaderDefault()
{
	EdfHeader_t h = { 1,2,3, 65001,BLOCK_SIZE,Default };
	return h;
}
//-----------------------------------------------------------------------------
EdfHeader_t MakeHeaderFromBytes(const uint8_t* b)
{
	EdfHeader_t h = 
	{
		.VersMajor = b[0],
		.VersMinor = b[1],
		.VersPatch = b[2],
		.Encoding = *((uint16_t*)&b[3]),
		.Blocksize = *((uint16_t*)&b[5]),
		.Flags = (Options_t)(*((uint32_t*)&b[7])),
	};
	return h;
}
//-----------------------------------------------------------------------------
size_t HeaderToBytes(const EdfHeader_t* h, uint8_t* b)
{
	//var b = new byte[16];
	b[0] = h->VersMajor;
	b[1] = h->VersMinor;
	b[2] = h->VersPatch;
	*((uint16_t*)&b[3]) = h->Encoding;
	*((uint16_t*)&b[5]) = h->Blocksize;
	*((uint32_t*)&b[7]) = h->Flags;
	memset(&b[11], 0, 5);
	return 16;
}