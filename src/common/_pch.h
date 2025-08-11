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

#include "edf_cfg.h"

#define FIELD_SIZEOF(t, f) (sizeof(((t*)0)->f))

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif 

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif 

#ifndef UNUSED
//#define UNUSED(x) ((x)=(x))
#define UNUSED(x) (void)(x);
#endif 

#define LOG_ERR() printf("\n err: %d %s %s ", __LINE__, __FILE__, __FUNCTION__)

#define ERR_NO 0;
#define ERR_SRC_SHORT -1;
#define ERR_DST_SHORT 1;

#define ERR_FN_NOT_EXIST -2;



#endif //PCH_H