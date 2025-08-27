#include "_pch.h"
#include "converter.h"
#include "SiamFileFormat.h"
#include "KeyValue.h"
#include "Charts.h"
#include "assert.h"
#include "edf_cfg.h"
#include "math.h"
//-----------------------------------------------------------------------------
/// SPSK
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

	EdfWriteInfo(&dw, &DateTimeInf, &writed);
	EdfWriteDataBlock(&dw, &(DateTime_t) { dat.Year + 2000, dat.Month, dat.Day, }, sizeof(DateTime_t));

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

	/*
	TypeInfo_t commentType = { .Type = CString, .Name = "Comments" };
	EdfWriteInfo(&dw, &commentType, &writed);
	EdfWriteDataBlock(&dw, &((char*) { "описание структуры OMEGA_DATA_V1_1" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Time - время измерения от начала дня, мс" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Press - давление, 0.001 атм" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Temp - температура, 0.001 °С" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Vbat - напряжение батареи V" }), sizeof(char*));
	*/
	EdfWriteInfo(&dw, &ChartXYDescriptionInf, &writed);
	EdfWriteDataBlock(&dw, &((ChartXYDesct_t[])
	{
		{ "давление, 0.001 атм", "Press", "Time" },
		{ "температура, 0.001 °С", "Temp", "Time" },
		{ "напряжение батареи V", "Vbat", "Time" },
	}), sizeof(ChartXYDesct_t) * 3);

	if ((err = EdfWriteInfo(&dw, &OmegaDataInf, &writed)))
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

	// hint
	//int const* ptr; // ptr is a pointer to constant int 
	//int* const ptr;  // ptr is a constant pointer to int

	SPSK_FILE_V1_1 dat = { 0 };
	OMEGA_DATA_V1_1 record = { 0 };
	size_t recN = 0;
	const size_t data_len = sizeof(OMEGA_DATA_V1_1) - 2;// skip crc, not used yet
	uint8_t* precord = (void*)&record;
	uint8_t* const recordBegin = precord;
	uint8_t* const recordEnd = recordBegin + data_len;

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
			err = StreamWriteBinToCBin(br.Block, br.BlockLen, NULL, br.Buf, sizeof(br.Buf), NULL, &br.t);
			if (!err)
			{
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
			else if (0 == _strnicmp(br.t->Name, DateTimeInf.Name, 100))
			{
				DateTime_t t = *((DateTime_t*)br.Block);
				dat.Year = (uint8_t)(t.Year - 2000);
				dat.Month = t.Month;
				dat.Day = t.Day;
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
				uint8_t len = MIN(*((uint8_t*)br.Block), FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Cluster));
				memcpy(dat.Id.Cluster, &br.Block[1], len);
			}
			else if (0 == _strnicmp(br.t->Name, "Well", 10))
			{
				uint8_t len = MIN(*((uint8_t*)br.Block), FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Well));
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
					dat.crc = MbCrc16(&dat, sizeof(SPSK_FILE_V1_1));
					if (1 != fwrite(&dat, sizeof(SPSK_FILE_V1_1), 1, f))
						return -1;
				}
				uint8_t* pblock = br.Block;

				while (0 < br.BlockLen)
				{
					size_t len = (size_t)MIN(br.BlockLen, (size_t)(recordEnd - precord));
					memcpy(precord, pblock, len);
					precord += len;
					pblock += len;
					br.BlockLen -= len;
					if (recordEnd == precord)
					{
						precord = recordBegin;
						if (1 != fwrite(&record, sizeof(OMEGA_DATA_V1_1), 1, f))
							return -1;
					}
				}//while (0 < br.BlockLen)
			}//else
		}//case btVarData:
		break;
		}//switch (br.BlockType)
		if (0 != err)
		{
			LOG_ERR();
			break;
		}
	}//while (!(err = EdfReadBlock(&br)))

	fclose(f);
	EdfClose(&br);
	return 0;
}
//-----------------------------------------------------------------------------