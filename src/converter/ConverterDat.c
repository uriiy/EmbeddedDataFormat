#include "_pch.h"
#include "converter.h"
#include "SiamFileFormat.h"
#include "assert.h"
#include "edf_cfg.h"
#include "math.h"
//-----------------------------------------------------------------------------
/// SPSK
//-----------------------------------------------------------------------------
TypeInfo_t recordInf =
{
	.Type = Struct, .Name = "OMEGA_DATA_V1_1", .Dims = {0, NULL},
	.Childs =
	{
		.Count = 4,
		.Item = (TypeInfo_t[])
		{
			{ UInt32, "время от начала дня(мс)" },
			{ Int32, "давление, 0.001(атм)" },
			{ Int32, "температура, 0.001(°С)" },
			{ UInt16, "сопр.изоляции(кОм)" },
		}
	}
};
#pragma pack(push,1)
typedef struct
{
	uint32_t Time;			// время измерения от начала дня, мс
	int32_t Press;			// давление, 0.001 атм
	int32_t Temp;			// температура, 0.001 °С
	uint16_t Vbat;			// напряжение батареи,
} Dat_t;
#pragma pack(pop)

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

	char* edfMode = NULL;
	if ('t' == mode)
		edfMode = "wt";
	else if ('b' == mode)
		edfMode = "wb";
	else
		return -1;

	EdfWriter_t dw;
	size_t writed = 0;
	if ((err = EdfOpen(&dw, edf, edfMode)))
		return err;

	EdfHeader_t h = MakeHeaderDefault();
	if ((err = EdfWriteHeader(&dw, &h, &writed)))
		return err;

	EdfWriteInfData(&dw, UInt32, "FileType", &dat.FileType);
	EdfWriteStringBytes(&dw, "FileDescription", &dat.FileDescription, FIELD_SIZEOF(SPSK_FILE_V1_1, FileDescription));

	EdfWriteInfData(&dw, UInt8, "Year", &dat.Year);
	EdfWriteInfData(&dw, UInt8, "Month", &dat.Month);
	EdfWriteInfData(&dw, UInt8, "Day", &dat.Day);

	EdfWriteInfData(&dw, UInt16, "Shop", &dat.Id.Shop);
	EdfWriteInfData(&dw, UInt16, "Field", &dat.Id.Field);
	EdfWriteStringBytes(&dw, "Cluster", &dat.Id.Cluster, FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Cluster));
	EdfWriteStringBytes(&dw, "Well", &dat.Id.Well, FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Well));
	EdfWriteInfData(&dw, UInt16, "PlaceId", &dat.Id.PlaceId);
	EdfWriteInfData(&dw, Int32, "Depth", &dat.Id.Depth);

	EdfWriteInfData(&dw, UInt16, "RegType", &dat.RegType);
	EdfWriteInfData(&dw, UInt16, "RegNum", &dat.RegNum);
	EdfWriteInfData(&dw, UInt16, "RegVer", &dat.RegVer);
	EdfWriteInfData(&dw, UInt16, "SensType", &dat.SensType);
	EdfWriteInfData(&dw, UInt32, "SensNum", &dat.SensNum);
	EdfWriteInfData(&dw, UInt16, "SensVer", &dat.SensVer);

	if ((err = EdfWriteInfo(&dw, &recordInf, &writed)))
		return err;

	OMEGA_DATA_V1_1 record;
	do
	{
		if (1 == fread(&record, sizeof(OMEGA_DATA_V1_1), 1, f))
		{
			if ((err = EdfWriteDataBlock(&dw, &record, sizeof(OMEGA_DATA_V1_1) - 2)))
				return err;
			//EdfFlushDataBlock(&dw, &writed);
		}
	} while (!feof(f));

	fclose(f);
	EdfClose(&dw);
	return 0;
}
//-----------------------------------------------------------------------------
int EdfToDat(const char* edfFile, const char* datFile)
{
	int err = 0;

	EdfWriter_t br;
	size_t writed = 0;
	if ((err = EdfOpen(&br, edfFile, "rb")))
		return err;

	FILE* f = NULL;
	if ((err = fopen_s(&f, datFile, "wb")))
		return err;

	SPSK_FILE_V1_1 dat = { 0 };
	OMEGA_DATA_V1_1 record = { 0 };
	size_t recN = 0;

	while (!(err = EdfReadBlock(&br)))
	{
		switch (br.BlockType)
		{
		default: break;
		case btHeader:
			if (16 == br.BlockLen)
			{
				//EdfHeader_t h = { 0 };
				//err = MakeHeaderFromBytes(br.Block, br.BlockLen, &h);
				//if (!err)
				//	err = EdfWriteHeader(&tw, &h, &writed);
			}
			break;
		case btVarInfo:
		{
			br.t = NULL;
			uint8_t* srcData = br.Block;
			uint8_t* mem = (uint8_t*)&br.Buf;
			size_t memLen = sizeof(br.Buf);
			err = InfoFromBytes(&srcData, (TypeInfo_t*)&br.Buf, &mem, memLen);
			writed = mem - br.Buf;
			if (!err)
			{
				br.t = (TypeInfo_t*)&br.Buf;
				writed = 0;
			}
			else
			{
				err = 0;
				//return err;// ignore wrong or too big info block
			}
		}
		break;
		case btVarData:
		{
			//EdfWriteDataBlock(&tw, &br.Block, br.BlockLen);
			//EdfFlushDataBlock(&tw, &writed);
			if (0 == _strnicmp(br.t->Name, "FileType", 10))
			{
				dat.FileType = *((uint32_t*)br.Block);
			}
			else if (0 == _strnicmp(br.t->Name, "FileDescription", 50))
			{
				uint8_t len = *((uint8_t*)br.Block);
				len = MIN(len, FIELD_SIZEOF(SPSK_FILE_V1_1, FileDescription));
				memcpy(dat.FileDescription, &br.Block[1], len);
			}
			else if (0 == _strnicmp(br.t->Name, "Year", 10))
			{
				dat.Year = *((uint8_t*)br.Block);
			}
			else if (0 == _strnicmp(br.t->Name, "Month", 10))
			{
				dat.Month = *((uint8_t*)br.Block);
			}
			else if (0 == _strnicmp(br.t->Name, "Day", 10))
			{
				dat.Day = *((uint8_t*)br.Block);
			}
			else if (0 == _strnicmp(br.t->Name, "Shop", 10))
			{
				dat.Id.Shop = *((uint16_t*)br.Block);
			}
			else if (0 == _strnicmp(br.t->Name, "Field", 10))
			{
				dat.Id.Field = *((uint16_t*)br.Block);
			}
			else if (0 == _strnicmp(br.t->Name, "Cluster", 10))
			{
				uint8_t len = *((uint8_t*)br.Block);
				len = MIN(*((uint8_t*)br.Block), FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Cluster));
				memcpy(dat.Id.Cluster, &br.Block[1], len);
			}
			else if (0 == _strnicmp(br.t->Name, "Well", 10))
			{
				uint8_t len = *((uint8_t*)br.Block);
				len = MIN(*((uint8_t*)br.Block), FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Well));
				memcpy(dat.Id.Well, &br.Block[1], len);
			}
			else if (0 == _strnicmp(br.t->Name, "PlaceId", 10))
			{
				dat.Id.PlaceId = *((uint16_t*)br.Block);
			}
			else if (0 == _strnicmp(br.t->Name, "Depth", 10))
			{
				dat.Id.Depth = *((int32_t*)br.Block);
			}
			else if (0 == _strnicmp(br.t->Name, "RegType", 50))
				dat.RegType = *((uint16_t*)br.Block);
			else if (0 == _strnicmp(br.t->Name, "RegNum", 50))
				dat.RegNum = *((uint16_t*)br.Block);
			else if (0 == _strnicmp(br.t->Name, "RegVer", 50))
				dat.RegVer = *((uint16_t*)br.Block);
			else if (0 == _strnicmp(br.t->Name, "SensType", 50))
				dat.SensType = *((uint16_t*)br.Block);
			else if (0 == _strnicmp(br.t->Name, "SensNum", 50))
				dat.SensNum = *((uint32_t*)br.Block);
			else if (0 == _strnicmp(br.t->Name, "SensVer", 50))
				dat.SensVer = *((uint16_t*)br.Block);
			else if (0 == _strnicmp(br.t->Name, "OMEGA_DATA_V1_1", 50))
			{
				if (0 == recN++)
				{
					if (1 != fwrite(&dat, sizeof(SPSK_FILE_V1_1), 1, f))
						return -1;
				}
				uint8_t* pblock = br.Block;
				const size_t data_len = sizeof(OMEGA_DATA_V1_1) - 2;// skip crc

				while(br.BlockLen)
				{
					memcpy(&record, pblock, data_len);
					pblock += data_len;
					br.BlockLen -= data_len;
					if (data_len > br.BlockLen)
						break;
					if (1 != fwrite(&record, sizeof(OMEGA_DATA_V1_1), 1, f))
						return -1;
				}
			}
		}
		break;
		}
		if (0 != err)
		{
			LOG_ERR();
			break;
		}
	}















	fclose(f);
	EdfClose(&br);
	return 0;
}
//-----------------------------------------------------------------------------