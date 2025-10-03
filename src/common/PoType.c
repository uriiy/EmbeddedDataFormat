#include "_pch.h"
#include "PoType.h"

//-----------------------------------------------------------------------------
uint8_t GetSizeOf(enum PoType p)
{
	switch (p)
	{
	case Struct:
	default: return 0;
	case UInt8:
	case Int8:
	case Char:
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
	case String:
		return sizeof(char*);
	}
}
//-----------------------------------------------------------------------------
uint8_t IsPoType(PoType p)
{
	return Struct <= p && String >= p;
}
//-----------------------------------------------------------------------------
uint8_t IsBlockType(BlockType t)
{
	return btHeader == t || btVarInfo == t || btVarData == t;
}