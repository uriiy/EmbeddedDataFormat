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
void EdfClose(DataWriter_t* dw);
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif //EDF_H