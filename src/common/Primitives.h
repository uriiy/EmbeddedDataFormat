#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include "_pch.h"
#include "PoType.h"

typedef int (*WritePrimitivesFn)(PoType t,
	const uint8_t* src, size_t srcLen,
	uint8_t* dst, size_t dstLen,
	size_t* r, size_t* w);

int BinToBin(PoType t,
	const uint8_t* src, size_t srcLen,
	uint8_t* dst, size_t dstLen,
	size_t* r, size_t* w);

int CBinToBin(PoType t,
	const uint8_t* src, size_t srcLen,
	uint8_t* dst, size_t dstLen,
	size_t* r, size_t* w);

int CBinToStr(PoType t,
	const uint8_t* src, size_t srcLen,
	uint8_t* dst, size_t dstLen,
	size_t* r, size_t* w);

int BinToStr(PoType t,
	const uint8_t* src, size_t srcLen,
	uint8_t* dst, size_t dstLen,
	size_t* r, size_t* w);

// BinToCBin // required dynamic mem allocator

#endif