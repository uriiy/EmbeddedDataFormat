#ifndef TYPEINFO_H
#define TYPEINFO_H

#include "_pch.h"
#include "PoType.h"
#include "EdfStream.h"

typedef struct TypeInfo
{
	uint8_t Type; /// PoType 
	char* Name;
	struct
	{
		uint8_t Count;
		uint32_t* Item;
	} Dims;
	struct
	{
		uint8_t Count;
		struct TypeInfo* Item;
	} Childs;
} TypeInfo_t;

typedef struct
{
	uint32_t Id;
	TypeInfo_t Inf;
} TypeRec_t;


uint32_t GetTypeCSize(const TypeInfo_t* t);
int8_t HasDynamicFields(const TypeInfo_t* t);
int StreamWriteInfBin(Stream_t* st, const TypeRec_t* t, size_t* writed);
int StreamWriteInfTxt(Stream_t* st, const TypeRec_t* t, size_t* writed);
int StreamWriteBinToCBin(uint8_t* src, size_t srcLen, size_t* readed,
	uint8_t* dst, size_t dstLen, size_t* writed,
	TypeRec_t** t);

#endif