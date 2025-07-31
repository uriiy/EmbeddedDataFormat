#ifndef DFHEADER_H
#define DFHEADER_H

#include "_pch.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BLOCK_SIZE 1024

typedef enum Options
{
	Default = 0,
	UseCrc = 1,
	MaskUseCrc = 0xFE,
} Options_t;

typedef struct DfHeader
{
	uint8_t VersMajor;
	uint8_t VersMinor;
	uint8_t VersPatch;
	uint16_t Encoding;
	uint16_t Blocksize;
	Options_t Flags;
} DfHeader_t;

DfHeader_t MakeHeaderDefault();
DfHeader_t MakeHeaderFromBytes(const uint8_t* b);
size_t HeaderToBytes(const DfHeader_t* h, uint8_t* b);

#ifdef __cplusplus
}
#endif

#endif