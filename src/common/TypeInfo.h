#ifndef TYPEINFO_H
#define TYPEINFO_H

#include "_pch.h"
#include "PoType.h"

typedef struct TypeInfo TypeInfo_t;
/*
typedef struct Dims
{
	uint8_t Count;
	uint32_t* Item;
} Dims_t;

typedef struct Childs
{
	uint8_t Count;
	TypeInfo_t* Item;
} Childs_t;
*/
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
int ToBytes(const TypeInfo_t* t, uint8_t* buf);
int ToString(const TypeInfo_t* t, uint8_t* buf, int noffset);
size_t FromBytes(uint8_t** src, TypeInfo_t* t, uint8_t** mem);


#endif