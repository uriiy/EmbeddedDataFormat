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
int StreamWriteInfoBin(Stream_t* s, const TypeInfo_t* t, size_t* writed)
{
	int err = 0;
	// TYPE
	if ((err = StreamWrite(s, writed, &(uint8_t){ t->Type }, 1)))
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
	size_t nameSize = t->Name ? strnlength(t->Name, 255) : 0;

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
	case Int16: POT_PRINT_S(s, PoTypeInt16);
	case Half: POT_PRINT_S(s, PoTypeHalf);
	case UInt32: POT_PRINT_S(s, PoTypeUInt32);
	case Int32: POT_PRINT_S(s, PoTypeInt32);
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
	if (t->Name && 0 < strnlength(t->Name, 255))
		if ((err = StreamWriteFmt(s, writed, " \'%.255s\'", t->Name)))
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

static int StreamBinToCBin(MemStream_t* src, MemStream_t* mem, TypeInfo_t** t)
{
	int err = 0;
	size_t readed = 0;

	TypeInfo_t* ti = NULL;
	if (*t)
		ti = *t;
	else
		if ((err = MemAlloc(mem, sizeof(TypeInfo_t), &ti)))
			return err;

	if ((err = StreamRead(src, &readed, &ti->Type, 1)))
		return err;
	if (!IsPoType(ti->Type))
		return (size_t)-1;
	if ((err = StreamRead(src, &readed, &ti->Dims.Count, 1)))
		return err;

	if (ti->Dims.Count)
	{
		// allocate array
		const size_t dimsSize = sizeof(uint32_t) * ti->Dims.Count;
		if ((err = MemAlloc(mem, dimsSize, &ti->Dims.Item)))
			return err;
		for (uint8_t i = 0; i < ti->Dims.Count; i++)
		{
			if ((err = StreamRead(src, &readed, &ti->Dims.Item[i], sizeof(uint32_t))))
				return err;
		}
	}

	size_t nameSize = 0;
	if ((err = StreamRead(src, &readed, &nameSize, 1)) ||
		(err = MemAlloc(mem, nameSize + 1, &ti->Name)) ||
		(err = StreamRead(src, &readed, ti->Name, nameSize)))
		return err;
	ti->Name[nameSize] = 0;

	if (Struct == ti->Type)
	{
		if ((err = StreamRead(src, &readed, &ti->Childs.Count, 1)))
			return err;
		if (ti->Childs.Count)
		{
			// allocate array
			const size_t childsSize = sizeof(TypeInfo_t) * ti->Childs.Count;
			if ((err = MemAlloc(mem, childsSize, &ti->Childs.Item)))
				return err;
			for (uint8_t i = 0; i < ti->Childs.Count; i++)
			{
				TypeInfo_t* ch = &ti->Childs.Item[i];
				if ((err = StreamBinToCBin(src, mem, &ch)))
					return err;
			}
		}
	}
	*t = ti;
	return 0;
}
//-----------------------------------------------------------------------------
int StreamWriteBinToCBin(uint8_t* src, size_t srcLen, size_t* readed,
	uint8_t* dst, size_t dstLen, size_t* writed,
	TypeRec_t** t)
{
	int err = 0;
	MemStream_t mssrc = { 0 };
	if ((err = MemStreamInOpen(&mssrc, src, srcLen)))
		return err;
	MemStream_t msdst = { 0 };
	if ((err = MemStreamOutOpen(&msdst, dst, dstLen)))
		return err;

	TypeRec_t* tr = NULL;
	if (*t)
		tr = *t;
	else
		if ((err = MemAlloc(&msdst, sizeof(TypeRec_t), &tr)))
			return err;

	if ((err = StreamRead(&mssrc, readed, &tr->Id, FIELD_SIZEOF(TypeRec_t, Id))))
		return err;
	if ((err = StreamBinToCBin(&mssrc, &msdst, &(TypeInfo_t*){&tr->Inf})))
		return err;
	if (readed)
		*readed = mssrc.RPos;
	if (writed)
		*writed = msdst.WPos;

	*t = tr;
	return err;
}
//-----------------------------------------------------------------------------
uint32_t GetTypeCSize(const TypeInfo_t* t)
{
	uint32_t sz = 0;

	switch (t->Type)
	{
	case Struct:
		if (t->Childs.Item && t->Childs.Count)
		{
			for (uint32_t i = 0; i < t->Childs.Count; i++)
				sz += GetTypeCSize(&t->Childs.Item[i]);
		}
		break;
	default:
		sz = GetSizeOf(t->Type);
		break;
	}//switch
	if (t->Dims.Item && t->Dims.Count)
		for (uint32_t i = 0; i < t->Dims.Count; i++)
			sz *= t->Dims.Item[i];
	return sz;
}