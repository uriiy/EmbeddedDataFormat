#ifndef KEYVALUE_H
#define KEYVALUE_H

#include "_pch.h"
//-----------------------------------------------------------------------------
#pragma pack(push,1)
//-----------------------------------------------------------------------------
static const TypeInfo_t UInt16ValueInf =
{
	Struct, "UInt16Value", { 0, NULL },
	.Childs =
	{
		.Count = 4,
		.Item = (TypeInfo_t[])
		{
			{ CString, "Name" },
			{ UInt16, "Value" },
			{ CString, "Unit" },
			{ CString, "Description" },
		}
	}
};
typedef struct UInt16Value
{
	char* Name;
	uint16_t Value;
	char* Unit;
	char* Description;
} UInt16Value_t;

typedef void (*DoOnItemUInt16)(UInt16Value_t s, void* state);

int DeSerializeUInt16KeyVal(uint8_t* psrc, const uint8_t* const psrcEnd,
	uint8_t** ppbuf, uint8_t* const pbufBegin, uint8_t* const pbufEnd,
	DoOnItemUInt16 DoOnItem, void* state);
//-----------------------------------------------------------------------------
static const TypeInfo_t UInt32ValueInf =
{
	Struct, "UInt32Value", { 0, NULL },
	.Childs =
	{
		.Count = 4,
		.Item = (TypeInfo_t[])
		{
			{ CString, "Name" },
			{ UInt32, "Value" },
			{ CString, "Unit" },
			{ CString, "Description" },
		}
	}
};
typedef struct UInt32Value
{
	char* Name;
	uint32_t Value;
	char* Unit;
	char* Description;
} UInt32Value_t;
//-----------------------------------------------------------------------------
static const TypeInfo_t DoubleValueInf =
{
	Struct, "DoubleValue", { 0, NULL },
	.Childs =
	{
		.Count = 4,
		.Item = (TypeInfo_t[])
		{
			{ CString, "Name" },
			{ Double, "Value" },
			{ CString, "Unit" },
			{ CString, "Description" },
		}
	}
};
typedef struct DoubleValue
{
	char* Name;
	double Value;
	char* Unit;
	char* Description;
} DoubleValue_t;

typedef void (*DoOnItemDoubleFn)(DoubleValue_t s, void* state);

int DeSerializeDoubleKeyVal(uint8_t* psrc, const uint8_t* const psrcEnd,
	uint8_t** ppbuf, uint8_t* const pbufBegin, uint8_t* const pbufEnd,
	DoOnItemDoubleFn DoOnItem, void* state);

//-----------------------------------------------------------------------------
#pragma pack(pop)
//-----------------------------------------------------------------------------
#endif