#include "_pch.h"
#include "converter.h"
#include "edf_cfg.h"

//-----------------------------------------------------------------------------
int BinToText(const char* src, const char* dst)
{
	EdfWriter_t br = { 0 };
	EdfWriter_t tw = { 0 };
	if (OpenBinReader(&br, src))
		LOG_ERR();
	if (OpenTextWriter(&tw, dst))
		LOG_ERR();

	size_t writed = 0;
	int err = 0;

	while (!(err = EdfReadBlock(&br)))
	{
		switch (br.BlockType)
		{
		default: break;
		case btHeader:
			if (16 == br.BlockLen)
			{
				EdfHeader_t h = { 0 };
				err = MakeHeaderFromBytes(br.Block, br.BlockLen, &h);
				if (!err)
					err = EdfWriteHeader(&tw, &h, &writed);
			}
			break;
		case btVarInfo:
		{
			tw.t = NULL;
			uint8_t* src = br.Block;
			uint8_t* mem = (uint8_t*)&br.Buf;
			size_t memLen = sizeof(br.Buf);
			err = InfoFromBytes(&src, (TypeInfo_t*)&br.Buf, &mem, memLen);
			writed = mem - br.Buf;
			if (!err)
			{
				tw.t = (TypeInfo_t*)&br.Buf;
				writed = 0;
				err = EdfWriteInfo(&tw, tw.t, &writed);
			}
		}
		break;
		case btVarData:
		{
			EdfWriteDataBlock(&tw, &br.Block, br.BlockLen);
			//EdfFlushDataBlock(&tw, &writed);
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
	return 0;
}
//-----------------------------------------------------------------------------
int ChangeExt(char* file, const char* input, const char* output)
{
	size_t extLen = strlen(input);
	size_t fileLen = strlen(file);
	if (4 < fileLen && 0 == _strcmpi(file + fileLen - extLen, input))
	{
		memcpy(file + fileLen - extLen, output, extLen);
		return 0;
	}
	return -1;
}
//-----------------------------------------------------------------------------
int IsExt(const char* file, const char* ext)
{
	size_t extLen = strlen(ext);
	size_t fileLen = strlen(file);
	const char* fileExt = file + fileLen - extLen;
	return 0 == _strcmpi(fileExt, ext);
}
//-----------------------------------------------------------------------------
int DatToEdf(const char* src, const char* edf, char mode)
{
	FILE* f = NULL;
	int err = fopen_s(&f, src, "rb");
	if (err)
		return err;

	SPSK_FILE_V1_1 dat;
	if (1 != fread(&dat, sizeof(SPSK_FILE_V1_1), 1, f))
		return -1;

	EdfWriter_t dw;
	size_t writed = 0;

	if ('t' == mode)
		err = OpenTextWriter(&dw, edf);
	else if ('b' == mode)
		err = OpenBinWriter(&dw, edf);
	else
		err = -1;
	if (err)
		return err;

	EdfHeader_t h = MakeHeaderDefault();
	if ((err = EdfWriteHeader(&dw, &h, &writed)))
		return err;

	EdfWriteInfData(&dw, UInt32, "FileType", &dat.FileType);
	EdfWriteStringBytes(&dw, "FileDescription", &dat.FileDescription, FIELD_SIZEOF(SPSK_FILE_V1_1, FileDescription));

	EdfWriteInfData(&dw, UInt8, "Year", &dat.Year);
	EdfWriteInfData(&dw, UInt8, "Month", &dat.Month);
	EdfWriteInfData(&dw, UInt8, "Day", &dat.Day);

	EdfWriteInfData(&dw, UInt16, "Shop", &dat.id.Shop);
	EdfWriteInfData(&dw, UInt16, "Field", &dat.id.Field);
	EdfWriteStringBytes(&dw, "Cluster", &dat.id.Cluster, FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Cluster));
	EdfWriteStringBytes(&dw, "Well", &dat.id.Well, FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Well));
	EdfWriteInfData(&dw, UInt16, "PlaceId", &dat.id.PlaceId);
	EdfWriteInfData(&dw, Int32, "Depth", &dat.id.Depth);

	EdfWriteInfData(&dw, UInt16, "RegType", &dat.RegType);
	EdfWriteInfData(&dw, UInt16, "RegNum", &dat.RegNum);
	EdfWriteInfData(&dw, UInt16, "RegVer", &dat.RegVer);
	EdfWriteInfData(&dw, UInt16, "SensType", &dat.SensType);
	EdfWriteInfData(&dw, UInt32, "SensNum", &dat.SensNum);
	EdfWriteInfData(&dw, UInt16, "SensVer", &dat.SensVer);

	TypeInfo_t recordInf =
	{
		.Type = Struct, .Name = "OMEGA_DATA_FILE_V1_1", .Dims = {0, NULL},
		.Childs =
		{
			.Count = 4,
			.Item = (TypeInfo_t[])
			{
				{ UInt32, "Time" },
				{ Int32, "Press" },
				{ Int32, "Temp" },
				{ UInt16, "Vbat" },
			}
		}
	};
	EdfWriteInfo(&dw, &recordInf, &writed);


	OMEGA_DATA_FILE_V1_1 record;
	do
	{
		if (1 == fread(&record, sizeof(OMEGA_DATA_FILE_V1_1), 1, f))
		{
			EdfWriteDataBlock(&dw, &record, sizeof(OMEGA_DATA_FILE_V1_1) - 2);
			//EdfFlushDataBlock(&dw, &writed);
		}
	} while (!feof(f));




	EdfClose(&dw);
	return 0;
}

