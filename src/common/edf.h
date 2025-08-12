#ifndef EDF_H
#define EDF_H
//-----------------------------------------------------------------------------
#pragma pack(push,1)
#include "EdfWriter.h"
#pragma pack(pop)
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
int EdfWriteDataBlock(EdfWriter_t* dw, void* src, size_t srcLen);
int EdfFlushDataBlock(EdfWriter_t* dw, size_t* writed);

int EdfReadBlock(EdfWriter_t* dr);

//shortcut
int EdfWriteInfData(EdfWriter_t* dw, PoType, char* name, void* data);

//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif //EDF_H