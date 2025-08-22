#include "_pch.h"
//-----------------------------------------------------------------------------
#ifdef LOG_ERRF
static char errBuf[128];
void Log_ErrF(const char* const fmt, ...)
{
	va_list arglist;
	va_start(arglist, fmt);
	vsnprintf(errBuf, sizeof(errBuf), fmt, arglist);
	perror(errBuf);
	va_end(arglist);
}
#endif 
//-----------------------------------------------------------------------------
size_t strnlength(const char* s, size_t n)
{
	const char* found = memchr(s, '\0', n);
	return found ? (size_t)(found - s) : n;
}
//-----------------------------------------------------------------------------