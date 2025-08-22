#ifndef BLOCKWRITER_H
#define BLOCKWRITER_H

#include "_pch.h"
#include "EdfHeader.h"
#include "EdfStream.h"
#include "Primitives.h"
#include "TypeInfo.h"

typedef struct EdfWriter EdfWriter_t;

typedef int (*FlushDataFn)(EdfWriter_t* w, size_t* writed);
typedef int (*WriteHeaderFn)(EdfWriter_t* w, const EdfHeader_t* h, size_t* writed);
typedef int (*WriteInfoFn)(EdfWriter_t* w, const TypeInfo_t* t, size_t* writed);

int EdfWriteSep(const char* const src,
	uint8_t** dst, size_t* dstSize,
	size_t* skip, size_t* wqty,
	size_t* writed);
//-----------------------------------------------------------------------------

typedef struct EdfWriter
{
	EdfHeader_t h;
	const TypeInfo_t* t;
	Stream_t Stream;
	size_t Skip;

	uint8_t BlockType;
	uint8_t Seq;
	size_t BlockLen;
	uint8_t Block[BLOCK_SIZE];

	size_t BufLen;
	uint8_t Buf[BLOCK_SIZE];

	WritePrimitivesFn WritePrimitive;
	WriteHeaderFn FlushHeader;
	WriteInfoFn FlushInfo;
	FlushDataFn FlushData;

	const char* BeginStruct;
	const char* EndStruct;
	const char* BeginArray;
	const char* EndArray;
	const char* SepVarEnd;
	const char* RecBegin;
	const char* RecEnd;
} EdfWriter_t;

int StreamWriteBlockDataTxt(EdfWriter_t* dw, size_t* writed);
int StreamWriteBlockDataBin(EdfWriter_t* dw, size_t* writed);

int EdfWriteHeaderBin(EdfWriter_t* dw, const EdfHeader_t* h, size_t* writed);
int EdfWriteHeaderTxt(EdfWriter_t* dw, const EdfHeader_t* h, size_t* writed);

int EdfWriteInfoBin(EdfWriter_t* w, const TypeInfo_t* t, size_t* writed);
int EdfWriteInfoTxt(EdfWriter_t* w, const TypeInfo_t* t, size_t* writed);


//-----------------------------------------------------------------------------
#endif