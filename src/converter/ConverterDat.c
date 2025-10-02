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

	EdfWriteInfData(&dw, FILETYPE, UInt32, "FileType", &dat.FileType);
	EdfWriteInfDataString(&dw, FILEDESCRIPTION, "FileDescription",
		&dat.FileDescription, FIELD_SIZEOF(SPSK_FILE_V1_1, FileDescription));

	EdfWriteInfo(&dw, &(const TypeRec_t){ DateTimeType, BEGINDATETIME, "BeginDateTime" }, & writed);
	EdfWriteDataBlock(&dw, &(DateTime_t) { dat.Year + 2000, dat.Month, dat.Day, }, sizeof(DateTime_t));

	EdfWriteInfData(&dw, 0, UInt16, "Shop", &dat.Id.Shop);
	EdfWriteInfData(&dw, 0, UInt16, "Field", &dat.Id.Field);
	EdfWriteInfDataString(&dw, 0, "Cluster",
		&dat.Id.Cluster, FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Cluster));
	EdfWriteInfDataString(&dw, 0, "Well",
		&dat.Id.Well, FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Well));
	EdfWriteInfData(&dw, 0, UInt16, "PlaceId", &dat.Id.PlaceId);
	EdfWriteInfData(&dw, 0, Int32, "Depth", &dat.Id.Depth);

	EdfWriteInfData(&dw, 0, UInt16, "RegType", &dat.RegType);
	EdfWriteInfData(&dw, 0, UInt16, "RegNum", &dat.RegNum);
	EdfWriteInfData(&dw, 0, UInt16, "RegVer", &dat.RegVer);
	EdfWriteInfData(&dw, 0, UInt16, "SensType", &dat.SensType);
	EdfWriteInfData(&dw, 0, UInt32, "SensNum", &dat.SensNum);
	EdfWriteInfData(&dw, 0, UInt16, "SensVer", &dat.SensVer);

	/*
	TypeInfo_t commentType = { .Type = String, .Name = "Comments" };
	EdfWriteInfo(&dw, &commentType, &writed);
	EdfWriteDataBlock(&dw, &((char*) { "описание структуры OMEGA_DATA_V1_1" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Time - время измерения от начала дня, мс" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Press - давление, 0.001 атм" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Temp - температура, 0.001 °С" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Vbat - напряжение батареи V" }), sizeof(char*));
	*/
	EdfWriteInfo(&dw, &(const TypeRec_t){ ChartNInf, 0 }, & writed);
	EdfWriteDataBlock(&dw, &((ChartN_t[])
	{
		{ "Time", "мс", "", "время измерения от начала дня" },
		{ "Press", "0.001 атм","", "давление" },
		{ "Temp", "0.001 °С","", "температура" },
		{ "Vbat", "0.001 V","", "напряжение батареи" },
	}), sizeof(ChartN_t) * 4);

	if ((err = EdfWriteInfo(&dw, &(TypeRec_t){OmegaDataType, OMEGADATA}, & writed)))
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
		switch (br.BlkType)
		{
		default: break;
		case btHeader:
			if (16 == br.DatLen)
			{
				//EdfHeader_t h = { 0 };
				//err = MakeHeaderFromBytes(br.Block, br.DatLen, &h);
				//if (!err)
				//	err = EdfWriteHeader(&tw, &h, &writed);
			}
			break;
		case btVarInfo:
		{
			br.t = NULL;
			err = StreamWriteBinToCBin(br.Block, br.DatLen, NULL, br.Buf, sizeof(br.Buf), NULL, &br.t);
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
			if (br.t->Id)
			{
				switch (br.t->Id)
				{
				default: break;
				case FILETYPE: dat.FileType = *((uint32_t*)br.Block); break;//case FILETYPE:
				case FILEDESCRIPTION:
					memcpy(dat.FileDescription, &br.Block[1],
						MIN(*((uint8_t*)br.Block), FIELD_SIZEOF(SPSK_FILE_V1_1, FileDescription)));
					break;//case FILEDESCRIPTION:
				case BEGINDATETIME:
				{
					DateTime_t t = *((DateTime_t*)br.Block);
					dat.Year = (uint8_t)(t.Year - 2000);
					dat.Month = t.Month;
					dat.Day = t.Day;
				}
				break;
				case OMEGADATA:
				{
					if (0 == recN++)
					{
						dat.crc = MbCrc16(&dat, sizeof(SPSK_FILE_V1_1));
						if (1 != fwrite(&dat, sizeof(SPSK_FILE_V1_1), 1, f))
							return -1;
					}
					uint8_t* pblock = br.Block;

					while (0 < br.DatLen)
					{
						size_t len = (size_t)MIN(br.DatLen, (size_t)(recordEnd - precord));
						memcpy(precord, pblock, len);
						precord += len;
						pblock += len;
						br.DatLen -= (uint16_t)len;
						if (recordEnd == precord)
						{
							precord = recordBegin;
							if (1 != fwrite(&record, sizeof(OMEGA_DATA_V1_1), 1, f))
								return -1;
						}
					}//while (0 < br.DatLen)
				}
				break;//OMEGADATAREC

				}//switch

			}
			else if (IsVarName(br.t, "Shop"))
			{
				dat.Id.Shop = *((uint16_t*)br.Block);
			}
			else if (IsVarName(br.t, "Field"))
			{
				dat.Id.Field = *((uint16_t*)br.Block);
			}
			else if (IsVarName(br.t, "Cluster"))
			{
				uint8_t len = MIN(*((uint8_t*)br.Block), FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Cluster));
				memcpy(dat.Id.Cluster, &br.Block[1], len);
			}
			else if (IsVarName(br.t, "Well"))
			{
				uint8_t len = MIN(*((uint8_t*)br.Block), FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Well));
				memcpy(dat.Id.Well, &br.Block[1], len);
			}
			else if (IsVarName(br.t, "PlaceId"))
			{
				dat.Id.PlaceId = *((uint16_t*)br.Block);
			}
			else if (IsVarName(br.t, "Depth"))
			{
				dat.Id.Depth = *((int32_t*)br.Block);
			}
			else if (IsVarName(br.t, "RegType"))
				dat.RegType = *((uint16_t*)br.Block);
			else if (IsVarName(br.t, "RegNum"))
				dat.RegNum = *((uint16_t*)br.Block);
			else if (IsVarName(br.t, "RegVer"))
				dat.RegVer = *((uint16_t*)br.Block);
			else if (IsVarName(br.t, "SensType"))
				dat.SensType = *((uint16_t*)br.Block);
			else if (IsVarName(br.t, "SensNum"))
				dat.SensNum = *((uint32_t*)br.Block);
			else if (IsVarName(br.t, "SensVer"))
				dat.SensVer = *((uint16_t*)br.Block);

		}//case btVarData:
		break;
		}//switch (br.BlkType)
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