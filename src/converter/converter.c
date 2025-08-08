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
				err = MakeHeaderFromBytes(br.Block, br.BlockLen , &h);
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
	uint8_t sbuf[256] = { 0 };
	size_t slen = 0;

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

	EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = UInt32, .Name = "FileType" }), &writed);
	EdfWriteDataBlock(&dw, &dat.FileType, sizeof(uint32_t));
	slen = GetBString(dat.FileDescription, sbuf, sizeof(sbuf));
	EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = String, .Name = "FileDescription" }), &writed);
	EdfWriteDataBlock(&dw, sbuf, slen);

	EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = UInt8, .Name = "Year" }), &writed);
	EdfWriteDataBlock(&dw, &dat.Year, 1);
	EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = UInt8, .Name = "Month" }), &writed);
	EdfWriteDataBlock(&dw, &dat.Month, 1);
	EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = UInt8, .Name = "Day" }), &writed);
	EdfWriteDataBlock(&dw, &dat.Day, 1);

	EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = UInt16, .Name = "Shop" }), &writed);
	EdfWriteDataBlock(&dw, &dat.id.Shop, FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Shop));
	EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = UInt16, .Name = "Field" }), &writed);
	EdfWriteDataBlock(&dw, &dat.id.Field, FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Field));
	slen = GetBString(dat.id.Cluster, sbuf, sizeof(sbuf));
	EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = String, .Name = "Cluster" }), &writed);
	EdfWriteDataBlock(&dw, sbuf, slen);
	slen = GetBString(dat.id.Well, sbuf, sizeof(sbuf));
	EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = String, .Name = "Well" }), &writed);
	EdfWriteDataBlock(&dw, sbuf, slen);
	EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = UInt16, .Name = "PlaceId" }), &writed);
	EdfWriteDataBlock(&dw, &dat.id.PlaceId, FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, PlaceId));
	EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = Int32, .Name = "Depth" }), &writed);
	EdfWriteDataBlock(&dw, &dat.id.Depth, FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Depth));

	EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = UInt16, .Name = "RegType" }), &writed);
	EdfWriteDataBlock(&dw, &dat.RegType, FIELD_SIZEOF(SPSK_FILE_V1_1, RegType));
	EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = UInt16, .Name = "RegNum" }), &writed);
	EdfWriteDataBlock(&dw, &dat.RegNum, FIELD_SIZEOF(SPSK_FILE_V1_1, RegNum));
	EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = UInt16, .Name = "RegVer" }), &writed);
	EdfWriteDataBlock(&dw, &dat.RegVer, FIELD_SIZEOF(SPSK_FILE_V1_1, RegVer));
	EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = UInt16, .Name = "SensType" }), &writed);
	EdfWriteDataBlock(&dw, &dat.SensType, FIELD_SIZEOF(SPSK_FILE_V1_1, SensType));
	EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = UInt32, .Name = "SensNum" }), &writed);
	EdfWriteDataBlock(&dw, &dat.SensNum, FIELD_SIZEOF(SPSK_FILE_V1_1, SensNum));
	EdfWriteInfo(&dw, &((TypeInfo_t) { .Type = UInt16, .Name = "SensVer" }), &writed);
	EdfWriteDataBlock(&dw, &dat.SensVer, FIELD_SIZEOF(SPSK_FILE_V1_1, SensVer));

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

