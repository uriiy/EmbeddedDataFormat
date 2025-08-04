#ifndef TYPEINFO_H
#define TYPEINFO_H

#include "_pch.h"
#include "PoType.h"
#include "EdfStream.h"

#ifdef __cplusplus
extern "C" {
#endif

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

//typedef struct TypeInfo_tx TypeInfo;


TypeInfo_t MakeTypeInfo(char* name, PoType type
	, uint8_t dimCount, uint32_t* dims
	, uint8_t childCount, TypeInfo_t* childs);

uint32_t GetValueSize(const TypeInfo_t* t);
size_t ToBytes(const TypeInfo_t* t, uint8_t* buf);
size_t ToString(const TypeInfo_t* t, uint8_t* buf, int noffset);
size_t InfToString(const TypeInfo_t* t, Stream_t* buf, int noffset);
size_t FromBytes(uint8_t** src, TypeInfo_t* t, uint8_t** mem);

#ifdef __cplusplus
}
#endif

#endif