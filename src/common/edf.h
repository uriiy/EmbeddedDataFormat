#ifndef EDF_H
#define EDF_H
//-----------------------------------------------------------------------------
#include "EdfWriter.h"
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//-----------------------------------------------------------------------------
int OpenBinWriter(EdfWriter_t* w, const char* file);
int OpenTextWriter(EdfWriter_t* w, const char* file);
int OpenBinReader(EdfWriter_t* w, const char* file);
int OpenTextReader(EdfWriter_t* w, const char* file);
void EdfClose(EdfWriter_t* dw);

int EdfWriteHeader(EdfWriter_t* dw, const EdfHeader_t* h, size_t* writed);
int EdfWriteInfo(EdfWriter_t* dw, const TypeInfo_t* t, size_t* writed);

size_t EdfWriteDataBlock(uint8_t* src, size_t srcLen, EdfWriter_t* dw);
size_t EdfFlushDataBlock(EdfWriter_t* dw);

size_t EdfReadBlock(EdfWriter_t* dr);
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif //EDF_H