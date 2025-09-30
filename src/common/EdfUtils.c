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
int minStack = INTPTR_MAX;
int maxStack = INTPTR_MIN;

int CallStackSize() 
{
	int var = 0;
	int esp = (int)(&var);
	var++;

	minStack = MIN(minStack, (int)esp);
	maxStack = MAX(maxStack, (int)esp);

	printf("CURR=%d MIN=%d MAX=%d DIFF=%d\n", esp, minStack, maxStack, maxStack - minStack);
	return 0;
}