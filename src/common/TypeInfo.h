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

TypeInfo_t MakeTypeInfo(char* name, PoType type
	, uint8_t dimCount, uint32_t* dims
	, uint8_t childCount, TypeInfo_t* childs);

uint32_t GetTypeCSize(const TypeInfo_t* t);
int StreamWriteInfoBin(Stream_t* s, const TypeInfo_t* t, size_t* writed);
int StreamWriteInfoTxt(Stream_t* buf, const TypeInfo_t* t, int noffset, size_t* writed);
int StreamWriteBinToCBin(uint8_t* src, size_t srcLen, size_t* readed,
	uint8_t* dst, size_t dstLen, size_t* writed,
	TypeRec_t** t);

#endif