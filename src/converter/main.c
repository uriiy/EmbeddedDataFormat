#include "_pch.h"
#include "converter.h"

//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	if (2 < argc && NULL != argv[1])
	{
		char dst[1024];
		size_t len = strlen(argv[1]);
		if (sizeof(dst) < len)
			return -1;
		snprintf(dst, sizeof(dst), "%s", argv[1]);
		if ('t' == *argv[2])
		{
			if (IsExt(dst, "bdf") && 0 == ChangeExt(dst, "bdf", "tdf"))
				return BinToText(argv[1], dst);
			if (IsExt(dst, "dat") && 0 == ChangeExt(dst, "dat", "tdf"))
				return DatToTdf(argv[1], dst);
		}
	}
	return 0;
}

