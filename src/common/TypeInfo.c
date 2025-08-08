#include "_pch.h"
#include "TypeInfo.h"

//-----------------------------------------------------------------------------
TypeInfo_t MakeTypeInfo(char* name, PoType type
	, uint8_t dimCount, uint32_t* dims
	, uint8_t childCount, TypeInfo_t* childs)
{
	TypeInfo_t t;
	memset(&t, 0, sizeof(TypeInfo_t));
	t.Type = type;
	t.Name = name;
	t.Dims.Count = dimCount;
	t.Dims.Item = dims;
	t.Childs.Count = childCount;
	t.Childs.Item = childs;
	return t;
}
//-----------------------------------------------------------------------------
uint32_t GetValueSize(const TypeInfo_t* t)
{
	uint32_t sz;
	if (Struct == t->Type && t->Childs.Item && t->Childs.Count)
	{
		sz = 0;
		for (uint32_t i = 0; i < t->Childs.Count; i++)
		{
			sz += GetValueSize(&t->Childs.Item[i]);
		}
	}
	else
	{
		sz = GetSizeOf(t->Type);
	}

	if (t->Dims.Item && t->Dims.Count)
	{
		for (uint32_t i = 0; i < t->Dims.Count; i++)
			sz *= t->Dims.Item[i];
	}

	return sz;
}
//-----------------------------------------------------------------------------
int StreamWriteInfoBin(Stream_t* s, const TypeInfo_t* t, size_t* writed)
{
	int err = 0;
	// TYPE
	if ((err = StreamWrite(s, writed, &t->Type, 1)))
		return err;
	if (t->Dims.Item && t->Dims.Count)
	{
		if ((err = StreamWrite(s, writed, &t->Dims.Count, 1)))
			return err;
		for (uint32_t i = 0; i < t->Dims.Count; i++)
			if ((err = StreamWrite(s, writed, &t->Dims.Item[i], sizeof(uint32_t))))
				return err;
	}
	else
	{
		if ((err = StreamWrite(s, writed, &err, 1)))//val=0
			return err;
	}

	size_t nameSize = t->Name ? strlen(t->Name) : 0;
	nameSize = (255 < nameSize ? 255 : nameSize);

	if ((err = StreamWrite(s, writed, &nameSize, 1)) ||
		(err = StreamWrite(s, writed, t->Name, nameSize)))
		return err;

	if (Struct == t->Type && t->Childs.Item && t->Childs.Count)
	{
		if ((err = StreamWrite(s, writed, &t->Childs.Count, 1)))
			return err;
		for (uint8_t i = 0; i < t->Childs.Count; i++)
			if ((err = StreamWriteInfoBin(s, &t->Childs.Item[i], writed)))
				return err;
	}
	return err;
}
//-----------------------------------------------------------------------------
static int StreamPrintOffset(Stream_t* s, int noffset, size_t* writed)
{
	int err = 0;
	for (int i = 0; i < noffset && 0 == err; i++)
		err = StreamWrite(s, writed, SepOffset, sizeof(SepOffset) - 1);
	return err;
}
//-----------------------------------------------------------------------------
static int StreamPrintType(Stream_t* s, PoType po, size_t* writed)
{
#define POT_PRINT_S(s, t) return StreamWrite(s, writed, t, (sizeof t) - 1)
	switch (po)
	{
	default: break;
	case Struct: POT_PRINT_S(s, PoTypeStruct);
	case Int8: POT_PRINT_S(s, PoTypeInt8);
	case UInt8: POT_PRINT_S(s, PoTypeUInt8);
	case Char: POT_PRINT_S(s, PoTypeChar);
	case String: POT_PRINT_S(s, PoTypeString);
	case UInt16: POT_PRINT_S(s, PoTypeUInt16);
	case Int16: POT_PRINT_S(s, PoTypeUInt16);
	case Half: POT_PRINT_S(s, PoTypeHalf);
	case UInt32: POT_PRINT_S(s, PoTypeInt32);
	case Int32: POT_PRINT_S(s, PoTypeUInt32);
	case Single: POT_PRINT_S(s, PoTypeSingle);
	case Int64: POT_PRINT_S(s, PoTypeInt64);
	case UInt64: POT_PRINT_S(s, PoTypeUInt64);
	case Double: POT_PRINT_S(s, PoTypeDouble);
	}
	return 0;
}
//-----------------------------------------------------------------------------
int StreamWriteInfoTxt(Stream_t* s, const TypeInfo_t* t, int noffset, size_t* writed)
{
	int err = 0;
	// TYPE
	if ((err = StreamPrintOffset(s, noffset, writed)) ||
		(err = StreamPrintType(s, t->Type, writed)))
		return err;
	// DIMS
	if (t->Dims.Count && t->Dims.Item)
	{
		for (size_t i = 0; i < t->Dims.Count; i++)
			if ((err = StreamWriteFmt(s, writed, "[%lu]", t->Dims.Item[i])))
				return err;
	}
	// NAME
	size_t nameSize = t->Name ? strlen(t->Name) : 0;
	nameSize = (255 < nameSize ? 255 : nameSize);
	if ((err = StreamWriteFmt(s, writed, " \'%s\'", t->Name)))
		return err;
	// CHILDS
	if (Struct == t->Type && t->Childs.Item && t->Childs.Count)
	{
		if ((err = StreamWrite(s, writed, "\n", 1)) ||
			(err = StreamPrintOffset(s, noffset, writed)) ||
			(err = StreamWrite(s, writed, "{", 1)))
			return err;
		for (size_t i = 0; i < t->Childs.Count; i++)
		{
			if ((err = StreamWrite(s, writed, "\n", 1)) ||
				(err = StreamWriteInfoTxt(s, &t->Childs.Item[i], noffset + 1, writed)))
				return err;
		}
		if ((err = StreamWrite(s, writed, "\n", 1)) ||
			(err = StreamPrintOffset(s, noffset, writed)) ||
			(err = StreamWrite(s, writed, "}", 1)))
			return err;
	}
	return StreamWrite(s, writed, ";", 1);
}
//-----------------------------------------------------------------------------
static int FromBytes(uint8_t** src, TypeInfo_t* t, uint8_t** mem, size_t memLen)
{
	uint8_t* psrc = *src;
	uint8_t* pdst = *mem;
	if (!IsPoType(psrc[0]))
		return (size_t)-1;
	memset(t, 0, sizeof(TypeInfo_t));
	t->Type = *psrc++;
	t->Dims.Count = *psrc++;

	if (t->Dims.Count)
	{
		// allocate array
		const size_t dimsSize = sizeof(uint32_t) * t->Dims.Count;
		if (dimsSize > memLen)
			return (size_t)-1;
		memLen -= dimsSize;

		t->Dims.Item = (uint32_t*)pdst;
		pdst += dimsSize;
		for (uint8_t i = 0; i < t->Dims.Count; i++)
		{
			t->Dims.Item[i] = *(uint32_t*)psrc;
			psrc += sizeof(uint32_t);
		}
	}
	const size_t nameSize = *psrc++;
	if (nameSize)
	{
		if (nameSize > memLen)
			return (size_t)-1;
		memLen -= nameSize;

		t->Name = (char*)pdst;
		memcpy(t->Name, psrc, nameSize);
		pdst += nameSize;
		*pdst++ = '\0'; memLen--;
		psrc += nameSize;
	}

	if (Struct == t->Type)
	{
		t->Childs.Count = *psrc++;
		if (t->Childs.Count)
		{
			// allocate array
			const size_t childsSize = sizeof(TypeInfo_t) * t->Childs.Count;
			if (childsSize > memLen)
				return (size_t)-1;
			memLen -= childsSize;

			t->Childs.Item = (TypeInfo_t*)pdst;
			pdst += childsSize;
			int err = 0;
			uint8_t* w = NULL;
			for (uint8_t i = 0; i < t->Childs.Count; i++)
			{
				w = pdst;
				if ((err = FromBytes(&psrc, &t->Childs.Item[i], &pdst, memLen)))
					return err;
				memLen -= pdst - w;
			}
		}
	}
	*src = (uint8_t*)(psrc);
	*mem = (uint8_t*)(pdst);
	return 0;
}
//-----------------------------------------------------------------------------
int InfoFromBytes(uint8_t** src, TypeInfo_t* t, uint8_t** mem, size_t memLen)
{
	(*mem) += sizeof(TypeInfo_t);
	memLen -= sizeof(TypeInfo_t);
	return FromBytes(src, t, mem, memLen);
}