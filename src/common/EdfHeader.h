#ifndef EDFHEADER_H
#define EDFHEADER_H

#include "_pch.h"

typedef enum Options
{
	Default = 0,
	UseCrc = 1,
} Options_t;

typedef struct EdfHeader
{
	uint8_t VersMajor;
	uint8_t VersMinor;
	uint8_t VersPatch;
	uint16_t Encoding;
	uint16_t Blocksize;
	Options_t Flags;
} EdfHeader_t;

EdfHeader_t MakeHeaderDefault(void);
int MakeHeaderFromBytes(const uint8_t* b, size_t srcSize, EdfHeader_t* h);
size_t HeaderToBytes(const EdfHeader_t* h, uint8_t* b);


#endif