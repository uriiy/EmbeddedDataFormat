#ifndef BLOCKWRITER_H
#define BLOCKWRITER_H

#include "_pch.h"
#include "EdfHeader.h"
#include "EdfStream.h"
#include "Primitives.h"
#include "TypeInfo.h"

typedef struct EdfWriter EdfWriter_t;

typedef int (*WriteSepFn)(uint8_t** dst, size_t* dstLen, size_t* w);
typedef size_t(*FlushBlockFn)(Stream_t* s, BlockType t, uint8_t seq, uint8_t* src, size_t len);
typedef int (*WriteHeaderFn)(EdfWriter_t* w, const EdfHeader_t* h, size_t* writed);
typedef int (*WriteInfoFn)(EdfWriter_t* w, const TypeInfo_t* t, size_t* writed);

//-----------------------------------------------------------------------------
//#pragma pack(1)
typedef struct EdfWriter
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
	WriteHeaderFn FlushHeader;
	WriteInfoFn FlushInfo;
	FlushBlockFn FlushData;

	WriteSepFn BeginStruct;
	WriteSepFn EndStruct;
	WriteSepFn BeginArray;
	WriteSepFn EndArray;
	WriteSepFn SepVarEnd;
	WriteSepFn RecBegin;
	WriteSepFn RecEnd;
} EdfWriter_t;

//-----------------------------------------------------------------------------
#endif