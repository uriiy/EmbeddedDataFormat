#ifndef EDF_H
#define EDF_H

#include "BlockWriter.h"

#define BLOCK_SIZE 256
//-----------------------------------------------------------------------------
DataWriter_t MakeBinWriter(const char* file)
{
	FILE* f = fopen(file, "wb");
	return (DataWriter_t)
	{
		.Stream = { .Instance = f, .Write = StreamWrite },
			.Seq = 0,
			.BlockLen = 0, .BufLen = 0,
			.WritePrimitive = BinToBin,
			.Flush = FlushBinBlock,
			.SepBeginStruct = NoWrite,
			.SepEndStruct = NoWrite,
			.SepBeginArray = NoWrite,
			.SepEndArray = NoWrite,
			.SepVar = NoWrite,
			.SepRecBegin = NoWrite,
			.SepRecEnd = NoWrite,
	};
}
//-----------------------------------------------------------------------------
DataWriter_t MakeTextWriter(const char* file)
{
	FILE* f = fopen(file, "wb");
	return (DataWriter_t)
	{
		.Stream = { .Instance = f, .Write = StreamWrite },
			.Seq = 0,
			.BlockLen = 0, .BufLen = 0,
			.WritePrimitive = BinToStr,
			.Flush = FlushTxtBlock,
			.SepBeginStruct = SepBeginStruct,
			.SepEndStruct = SepEndStruct,
			.SepBeginArray = SepBeginArray,
			.SepEndArray = SepEndArray,
			.SepVar = SepVar,
			.SepRecBegin = SepRecBegin,
			.SepRecEnd = SepRecEnd,
	};
}
//-----------------------------------------------------------------------------
DataWriter_t MakeBinReader(const char* file)
{
	FILE* f = fopen(file, "rb");
	return (DataWriter_t)
	{
		.Stream = { .Instance = f, .Read = StreamRead },
			.Seq = 0,
			.BlockLen = 0, .BufLen = 0,
			.Seq = 0,
			.BlockLen = 0, .BufLen = 0,
			.WritePrimitive = BinToStr,
			.Flush = NULL,
			.SepBeginStruct = SepBeginStruct,
			.SepEndStruct = SepEndStruct,
			.SepBeginArray = SepBeginArray,
			.SepEndArray = SepEndArray,
			.SepVar = SepVar,
			.SepRecBegin = SepRecBegin,
			.SepRecEnd = SepRecEnd,
	};
}
//-----------------------------------------------------------------------------
void Close(DataWriter_t* dw)
{
	if (dw->Stream.Instance)
	{
		FlushDataBlock(dw);
		fclose(dw->Stream.Instance);
		dw->Stream.Instance = NULL;
	}
}
//-----------------------------------------------------------------------------
#endif //EDF_H