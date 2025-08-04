#ifndef BLOCKWRITER_H
#define BLOCKWRITER_H

#include "_pch.h"
#include "EdfHeader.h"
#include "EdfStream.h"
#include "Primitives.h"
#include "TypeInfo.h"

typedef int (*WriteSep)(uint8_t** dst, size_t* dstLen, size_t* w);

//-----------------------------------------------------------------------------
int NoWrite(uint8_t** dst, size_t* dstLen, size_t* w);
int SepBeginStruct(uint8_t** dst, size_t* dstLen, size_t* w);
int SepEndStruct(uint8_t** dst, size_t* dstLen, size_t* w);
int SepBeginArray(uint8_t** dst, size_t* dstLen, size_t* w);
int SepEndArray(uint8_t** dst, size_t* dstLen, size_t* w);
int SepVar(uint8_t** dst, size_t* dstLen, size_t* w);
int SepRecBegin(uint8_t** dst, size_t* dstLen, size_t* w);
int SepRecEnd(uint8_t** dst, size_t* dstLen, size_t* w);

//-----------------------------------------------------------------------------
typedef size_t(*FlushBlockFn)(Stream_t* s, BlockType t, uint8_t seq, uint8_t* src, size_t len);
//-----------------------------------------------------------------------------

typedef struct DataWriter
{
	EdfHeader_t h;
	const TypeInfo_t* t;
	Stream_t Stream;
	uint8_t Seq;
	size_t Skip;
	size_t BlockLen;
	uint8_t Block[BLOCK_SIZE + 4];
	size_t BufLen;
	uint8_t Buf[BLOCK_SIZE];
	WritePrimitivesFn WritePrimitive;
	FlushBlockFn FlushBlock;
	WriteSep SepBeginStruct;
	WriteSep SepEndStruct;
	WriteSep SepBeginArray;
	WriteSep SepEndArray;
	WriteSep SepVar;
	WriteSep SepRecBegin;
	WriteSep SepRecEnd;
} DataWriter_t;
//-----------------------------------------------------------------------------
size_t WriteTxtHeaderBlock(const EdfHeader_t* h, Stream_t* stream);
size_t WriteHeaderBlock(const EdfHeader_t* h, DataWriter_t* dw);

size_t WriteTxtVarInfoBlock(const TypeInfo_t* t, DataWriter_t* tw);
size_t WriteVarInfoBlock(const TypeInfo_t* t, DataWriter_t* dw);

size_t WriteDataBlock(uint8_t* src, size_t srcLen, DataWriter_t* dw);
size_t FlushBinBlock(Stream_t* s, BlockType t, uint8_t seq, uint8_t* src, size_t len);
size_t FlushTxtBlock(Stream_t* s, BlockType t, uint8_t seq, uint8_t* src, size_t len);
size_t FlushDataBlock(DataWriter_t* dw);
//-----------------------------------------------------------------------------
size_t ReadBlock(DataWriter_t* dr);
size_t ReadHeaderBlock(DataWriter_t* dr, EdfHeader_t* h);

//-----------------------------------------------------------------------------
#endif