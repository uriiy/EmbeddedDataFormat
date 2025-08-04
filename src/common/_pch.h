#ifndef PCH_H
#define PCH_H

#ifdef __cplusplus
#include <cstdint>
#include <cstdio>
#include <cstring>
#else
#include "stdint.h"
#include "stdio.h"
#include "string.h"
#endif

//#include "windows.h"
//#include "uchar.h"
//#include "assert.h"
#include "memory.h"
#include "stdarg.h"

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif 

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif 

#define LOG_ERR() printf("err: %d %s %s ", __LINE__, __FILE__, __FUNCTION__)



#endif //PCH_H