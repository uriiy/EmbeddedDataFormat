#ifndef EDF_H
#define EDF_H
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//-----------------------------------------------------------------------------
#pragma pack(push,1)
#include "EdfWriter.h"
#pragma pack(pop)
//-----------------------------------------------------------------------------
// mode 
// "wb" - Write Binary file
// "wt" - Write Text file
// "ab" - Append existing Binary file
// "at" - Append existing Text file
// "rb" - Read Binary file
// "rt" - Read Text file
int EdfOpen(EdfWriter_t* w, const char* file, const char* mode);
void EdfClose(EdfWriter_t* dw);

int EdfWriteHeader(EdfWriter_t* dw, const EdfHeader_t* h, size_t* writed);
int EdfWriteInfo(EdfWriter_t* dw, const TypeInfo_t* t, size_t* writed);
int EdfWriteDataBlock(EdfWriter_t* dw, void* src, size_t srcLen);
int EdfFlushDataBlock(EdfWriter_t* dw, size_t* writed);

int EdfReadBlock(EdfWriter_t* dr);

//shortcut
int EdfWriteInfData(EdfWriter_t* dw, PoType, char* name, void* data);
int EdfWriteStringBytes(EdfWriter_t* dw, char* name, void* data, size_t maxLen);
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif //EDF_H