#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "_pch.h"
#include "TypeInfo.h"

typedef int (*WritePrimitivesFn)(PoType t,
	uint8_t* src, size_t srcLen,
	uint8_t* dst, size_t dstLen,
	size_t* r, size_t* w);

size_t GetBString(const char* str, uint8_t* dst, size_t dst_len);

int BinToBin(PoType t,
	uint8_t* src, size_t srcLen,
	uint8_t* dst, size_t dstLen,
	size_t* r, size_t* w);

int BinToStr(PoType t,
	uint8_t* src, size_t srcLen,
	uint8_t* dst, size_t dstLen,
	size_t* r, size_t* w);

#endif