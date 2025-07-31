#ifndef POTYPE_H
#define POTYPE_H
#include "_pch.h"

static const char PoTypeStruct[] = "Struct";
static const char PoTypeInt8[] = "Int8";
static const char PoTypeUInt8[] = "UInt8";
static const char PoTypeChar[] = "Char";
static const char PoTypeString[] = "String";

static const char PoTypeUInt16[] = "UInt16";
static const char PoTypeInt16[] = "Int16";
static const char PoTypeHalf[] = "Half";

static const char PoTypeUInt32[] = "UInt32";
static const char PoTypeInt32[] = "Int32";
static const char PoTypeSingle[] = "Single";

static const char PoTypeUInt64[] = "UInt64";
static const char PoTypeInt64[] = "Int64";
static const char PoTypeDouble[] = "Double";


//typedef uint8_t PoType;

//#pragma pack(1)
typedef enum PoType
{
	Struct = 0,
	// integres
	Int8,
	UInt8,
	Int16,
	UInt16,
	Int32,
	UInt32,
	Int64,
	UInt64,
	// float
	Half,
	Single,
	Double,
	// strings
	Char,
	String,
} PoType;

static uint8_t GetSizeOf(enum PoType p)
{
	switch (p)
	{
	default: return 0;
	case UInt8:
	case Int8:
	case Char:
	case String:
		return 1;
	case UInt16:
	case Int16:
	case Half:
		return 2;
	case UInt32:
	case Int32:
	case Single:
		return 4;
	case UInt64:
	case Int64:
	case Double:
		return 8;
	}
}

static uint8_t IsPoType(PoType p)
{
	if (Struct > p)
		return 0;
	if (String < p)
		return 0;
	return 1;
}


#endif