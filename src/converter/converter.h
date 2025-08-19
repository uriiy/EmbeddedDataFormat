#ifndef CONVERTER_H
#define CONVERTER_H

int IsExt(const char* file, const char* ext);
int ChangeExt(char* file, const char* ext);

int BinToText(const char* src, const char* dst);
int TextToBin(const char* src, const char* dst);

int DatToEdf(const char* src, const char* dst, char mode);
int EdfToDat(const char* src, const char* dst);

int EchoToEdf(const char* src, const char* dst, char mode);
int EdfToEcho(const char* src, const char* dst);

int DynToEdf(const char* src, const char* dst, char mode);
int EdfToDyn(const char* src, const char* dst);

#endif
