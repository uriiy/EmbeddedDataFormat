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
			{ String, "Name" },
			{ UInt16, "Value" },
			{ String, "Unit" },
			{ String, "Description" },
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

typedef void (*DoOnItemUInt16)(UInt16Value_t* s, void* state);

int UnpackUInt16KeyVal(MemStream_t* src, MemStream_t* dst,
	int* skip, DoOnItemUInt16 DoOnItem, void* state);
//-----------------------------------------------------------------------------
static const TypeInfo_t UInt32ValueInf =
{
	Struct, "UInt32Value", { 0, NULL },
	.Childs =
	{
		.Count = 4,
		.Item = (TypeInfo_t[])
		{
			{ String, "Name" },
			{ UInt32, "Value" },
			{ String, "Unit" },
			{ String, "Description" },
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

typedef void (*DoOnItemUInt32Fn)(UInt32Value_t* s, void* state);

int UnpackUInt32KeyVal(MemStream_t* src, MemStream_t* dst,
	int* skip, DoOnItemUInt32Fn DoOnItem, void* state);
//-----------------------------------------------------------------------------
static const TypeInfo_t DoubleValueInf =
{
	Struct, "DoubleValue", { 0, NULL },
	.Childs =
	{
		.Count = 4,
		.Item = (TypeInfo_t[])
		{
			{ String, "Name" },
			{ Double, "Value" },
			{ String, "Unit" },
			{ String, "Description" },
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

typedef void (*DoOnItemDoubleFn)(DoubleValue_t* s, void* state);
int UnpackDoubleKeyVal(MemStream_t* src, MemStream_t* dst,
	int* skip, DoOnItemDoubleFn DoOnItem, void* state);
//-----------------------------------------------------------------------------
#pragma pack(pop)
//-----------------------------------------------------------------------------
#endif