#ifndef EDF_H
#define EDF_H
//-----------------------------------------------------------------------------
#include "EdfWriter.h"
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//-----------------------------------------------------------------------------
int OpenBinWriter(DataWriter_t* w, const char* file);
int OpenTextWriter(DataWriter_t* w, const char* file);
int OpenBinReader(DataWriter_t* w, const char* file);
int OpenTextReader(DataWriter_t* w, const char* file);
void EdfClose(DataWriter_t* dw);

int EdfWriteHeader(DataWriter_t* dw, const EdfHeader_t* h, size_t* writed);
int EdfWriteInfo(DataWriter_t* dw, const TypeInfo_t* t, size_t* writed);

size_t EdfWriteDataBlock(uint8_t* src, size_t srcLen, DataWriter_t* dw);
size_t EdfFlushDataBlock(DataWriter_t* dw);

size_t EdfReadBlock(DataWriter_t* dr);
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif //EDF_H