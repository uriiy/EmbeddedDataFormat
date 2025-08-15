#include "_pch.h"
#include "converter.h"

//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	if (2 < argc && NULL != argv[1])
	{
		char dst[4096];
		size_t len = strlen(argv[1]);
		if (sizeof(dst) < len)
			return -1;
		snprintf(dst, sizeof(dst), "%s", argv[1]);
		if ('t' == *argv[2])
		{
			if (IsExt(dst, "bdf") && 0 == ChangeExt(dst, "tdf"))
				return BinToText(argv[1], dst);
			if (IsExt(dst, "dat") && 0 == ChangeExt(dst, "tdf"))
				return DatToEdf(argv[1], dst, 't');
			if (IsExt(dst, "e") && 0 == ChangeExt(dst, "tdf"))
				return EchoToEdf(argv[1], dst, 't');
			if (IsExt(dst, "d") && 0 == ChangeExt(dst, "tdf"))
				return DynToEdf(argv[1], dst, 't');
		}
		if ('b' == *argv[2])
		{
			//if (IsExt(dst, "tdf") && 0 == ChangeExt(dst, "tdf", "bdf"))
			//	return TextToBin(argv[1], dst);
			if (IsExt(dst, "dat") && 0 == ChangeExt(dst, "bdf"))
				return DatToEdf(argv[1], dst, 'b');
			if (IsExt(dst, "e") && 0 == ChangeExt(dst, "bdf"))
				return EchoToEdf(argv[1], dst, 'b');
			if (IsExt(dst, "d") && 0 == ChangeExt(dst, "bdf"))
				return DynToEdf(argv[1], dst, 'b');
		}
	}
	return 0;
}

