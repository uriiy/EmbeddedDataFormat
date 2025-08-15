#ifndef CONVERTER_H
#define CONVERTER_H

#include "_pch.h"
#include "SiamFileFormat.h"

int IsExt(const char* file, const char* ext);
int ChangeExt(char* file, const char* ext);

int BinToText(const char* src, const char* dst);
int DatToEdf(const char* src, const char* dst, char mode);
int EchoToEdf(const char* src, const char* dst, char mode);
int DynToEdf(const char* src, const char* dst, char mode);

#endif
