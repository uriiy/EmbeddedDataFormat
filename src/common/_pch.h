//#pragma once
//#include <cstdio>
//#include <cstdint>
//#include "windows.h"
#include "stdint.h"
#include "stdio.h"
//#include "uchar.h"
#include "memory.h"
#include "stdio.h"
#include "string.h"
//#include "assert.h"
#include <stdarg.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define LOG_ERR() printf("err: %d %s %s ", __LINE__, __FILE__, __FUNCTION__)