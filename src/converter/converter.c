#include "_pch.h"
#include "converter.h"
#include "edf_cfg.h"

//-----------------------------------------------------------------------------
void BinToText(const char* src, const char* dst)
{
	EdfWriter_t br = { 0 };
	EdfWriter_t tw = { 0 };
	if (OpenBinReader(&br, src))
		LOG_ERR();
	if (OpenTextWriter(&tw, dst))
		LOG_ERR();

	size_t writed = 0;
	int err = 0;

	while (-1 != EdfReadBlock(&br))
	{
		switch (br.Block[0])
		{
		default: break;
		case btHeader:
			if (4 + 16 == br.BlockLen)
			{
				EdfHeader_t h = { 0 };
				err = MakeHeaderFromBytes(&br.Block[4], br.BlockLen - 4, &h);
				if (!err)
					err = EdfWriteHeader(&tw, &h, &writed);
			}
			break;
		case btVarInfo:
		{
			uint8_t* src = &br.Block[4];
			TypeInfo_t* t = (TypeInfo_t*)&br.Buf;
			tw.t = t;
			uint8_t* mem = (uint8_t*)&br.Buf + sizeof(TypeInfo_t);
			err = FromBytes(&src, t, &mem);
			if (!err)
			{
				writed = 0;
				err = EdfWriteInfo(&tw, t, &writed);
			}
		}
		break;
		case btVarData:
		{
			err = EdfWriteDataBlock(&br.Block[4], br.BlockLen - 4, &tw);
			if (!err)
				EdfFlushDataBlock(&tw);
		}
		break;
		}
		if (0 != err)
		{
			LOG_ERR();
			break;
		}
	}
	EdfClose(&br);
	EdfClose(&tw);
}
//-----------------------------------------------------------------------------
void FilenameToTdf(const char* input, char* output)
{
	size_t len = strlen(input);
	if (4 < len && 0 == strcmp(input + len - 4, ".bdf"))
	{
		memcpy(output, input, len);
		*(output + len) = '\0';
		output += len - 3;
		*output = 't';
	}
}