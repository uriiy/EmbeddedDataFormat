#include "_pch.h"
#include "converter.h"

//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	if (2 < argc && NULL != argv[1] && 't' == *argv[2])
	{
		char fn[512];
		FilenameToTdf(argv[1], fn);
		BinToText(argv[1], fn);
	}
	return 0;
}

