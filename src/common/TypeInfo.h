#ifndef TYPEINFO_H
#define TYPEINFO_H

#include "_pch.h"
#include "PoType.h"
#include "EdfStream.h"

typedef struct TypeInfo
{
	PoType Type; /// PoType 
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

TypeInfo_t MakeTypeInfo(char* name, PoType type
	, uint8_t dimCount, uint32_t* dims
	, uint8_t childCount, TypeInfo_t* childs);

uint32_t GetValueSize(const TypeInfo_t* t);
size_t ToBytes(const TypeInfo_t* t, uint8_t* buf);
int StreamWriteInfoTxt(Stream_t* buf, const TypeInfo_t* t, int noffset, size_t* writed);
int FromBytes(uint8_t** src, TypeInfo_t* t, uint8_t** mem);

#endif