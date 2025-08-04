#ifndef EDFHEADER_H
#define EDFHEADER_H

#include "_pch.h"

#define BLOCK_SIZE 1024

typedef enum Options
{
	Default = 0,
	UseCrc = 1,
	MaskUseCrc = 0xFE,
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
EdfHeader_t MakeHeaderFromBytes(const uint8_t* b);
size_t HeaderToBytes(const EdfHeader_t* h, uint8_t* b);


#endif