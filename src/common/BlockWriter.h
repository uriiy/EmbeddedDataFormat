#ifndef BLOCKWRITER_H
#define BLOCKWRITER_H

#include "_pch.h"
#include "DfHeader.h"
#include "IStream.h"
#include "Primitives.h"
#include "TypeInfo.h"

typedef enum BlockType
{
	btHeader = 126, //0xB0, ~
	btVarInfo = 63, //0x3f, ?
	btVarData = 61, //0x3d, =
} BlockType;

static uint8_t IsBlockType(BlockType t)
{
	return btHeader == t || btVarInfo == t || btVarData == t;
}

typedef int (*WriteSep)(uint8_t** dst, size_t* dstLen, size_t* w);


//-----------------------------------------------------------------------------
static int WriteSp(const char* sep, uint8_t* dst, size_t* dstLen)
{
	if (!sep)
		return 0;
	size_t sepLen = strlen(sep);
	if (sepLen > *dstLen)
		return -1;
	memcpy(dst, sep, sepLen);
	*dstLen = sepLen;
	return 0;
}
//-----------------------------------------------------------------------------
static int NoWrite(uint8_t** dst, size_t* dstLen, size_t* w)
{
	return 0;
}
//-----------------------------------------------------------------------------
static int SepBeginStruct(uint8_t** dst, size_t* dstLen, size_t* w)
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
static int SepEndStruct(uint8_t** dst, size_t* dstLen, size_t* w)
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
static int SepBeginArray(uint8_t** dst, size_t* dstLen, size_t* w)
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
static int SepEndArray(uint8_t** dst, size_t* dstLen, size_t* w)
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
static int SepVar(uint8_t** dst, size_t* dstLen, size_t* w)
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
static int SepRecBegin(uint8_t** dst, size_t* dstLen, size_t* w)
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
static int SepRecEnd(uint8_t** dst, size_t* dstLen, size_t* w)
{
	return 0;
}

//-----------------------------------------------------------------------------
typedef size_t(*FlushBlock)(Stream_t* s, BlockType t, uint8_t seq, uint8_t* src, size_t len);
//-----------------------------------------------------------------------------

typedef struct DataWriter
{
	DfHeader_t h;
	TypeInfo_t* t;
	Stream_t Stream;
	uint8_t Seq;
	size_t Skip;
	size_t BlockLen;
	uint8_t Block[BLOCK_SIZE + 4];
	size_t BufLen;
	uint8_t Buf[BLOCK_SIZE];
	WritePrimitivesFn WritePrimitive;
	FlushBlock Flush;
	WriteSep SepBeginStruct;
	WriteSep SepEndStruct;
	WriteSep SepBeginArray;
	WriteSep SepEndArray;
	WriteSep SepVar;
	WriteSep SepRecBegin;
	WriteSep SepRecEnd;
} DataWriter_t;
//-----------------------------------------------------------------------------
size_t WriteTxtHeaderBlock(const DfHeader_t* h, DataWriter_t* tw);
size_t WriteHeaderBlock(const DfHeader_t* h, DataWriter_t* dw);

size_t WriteTxtVarInfoBlock(const TypeInfo_t* t, DataWriter_t* tw);
size_t WriteVarInfoBlock(const TypeInfo_t* t, DataWriter_t* dw);

size_t WriteDataBlock(uint8_t* src, size_t srcLen, DataWriter_t* dw);
size_t FlushBinBlock(Stream_t* s, BlockType t, uint8_t seq, uint8_t* src, size_t len);
size_t FlushTxtBlock(Stream_t* s, BlockType t, uint8_t seq, uint8_t* src, size_t len);
size_t FlushDataBlock(DataWriter_t* dw);
//-----------------------------------------------------------------------------
size_t ReadBlock(DataWriter_t* dr);
size_t ReadHeaderBlock(DataWriter_t* dr, DfHeader_t* h);
size_t ReadVarInfoBlock(DataWriter_t* dr, TypeInfo_t* t);
size_t ReadDataBlock(DataWriter_t* dr, uint8_t** dst);

//-----------------------------------------------------------------------------
#endif