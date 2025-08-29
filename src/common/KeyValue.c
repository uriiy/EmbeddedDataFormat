#include "_pch.h"
#include "KeyValue.h"

//-----------------------------------------------------------------------------
int UnpackUInt16KeyVal(MemStream_t* src, MemStream_t* dst,
	int* skip, DoOnItemUInt16 DoOnItem, void* state)
{
	int err = 0;
	UInt16Value_t* s = (*skip) ? (UInt16Value_t*)dst->Buffer : NULL;
	while (!(err = EdfSreamBinToCBin(&UInt16ValueInf, src, dst, &s, skip)))
	{
		(*DoOnItem)(s, state);
		s = NULL;
		*skip = 0;
		dst->WPos = 0;
	}
	*skip = 0 - (*skip);
	return err;
}
//-----------------------------------------------------------------------------
int UnpackUInt32KeyVal(MemStream_t* src, MemStream_t* dst,
	int* skip, DoOnItemUInt32Fn DoOnItem, void* state)
{
	int err = 0;
	UInt32Value_t* s = (*skip) ? (UInt32Value_t*)dst->Buffer : NULL;
	while (!(err = EdfSreamBinToCBin(&UInt32ValueInf, src, dst, &s, skip)))
	{
		(*DoOnItem)(s, state);
		s = NULL;
		*skip = 0;
		dst->WPos = 0;
	}
	*skip = 0 - (*skip);
	return err;
}
//-----------------------------------------------------------------------------
int UnpackDoubleKeyVal(MemStream_t* src, MemStream_t* dst,
	int* skip, DoOnItemDoubleFn DoOnItem, void* state)
{
	int err = 0;
	DoubleValue_t* s = (*skip) ? (DoubleValue_t*)dst->Buffer : NULL;
	while (!(err = EdfSreamBinToCBin(&DoubleValueInf, src, dst, &s, skip)))
	{
		(*DoOnItem)(s, state);
		s = NULL;
		*skip = 0;
		dst->WPos = 0;
	}
	*skip = 0 - (*skip);
	return err;
}
//-----------------------------------------------------------------------------
