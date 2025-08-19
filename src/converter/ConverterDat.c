#include "_pch.h"
#include "converter.h"
#include "SiamFileFormat.h"
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

	char cbuf[256] = { 0 };
	snprintf(cbuf, sizeof(cbuf), "%u.%02u.%02uT%02u:%02u:%02u",
		(uint32_t)2000 + dat.Year, dat.Month, dat.Day,
		(uint8_t)0, (uint8_t)0, (uint8_t)0);
	EdfWriteInfData(&dw, CString, "DateTime", &((char*) { cbuf }));
	//EdfWriteInfData(&dw, UInt8, "Year", &dat.Year);
	//EdfWriteInfData(&dw, UInt8, "Month", &dat.Month);
	//EdfWriteInfData(&dw, UInt8, "Day", &dat.Day);

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

	EdfClose(&dw);
	return 0;
}

//-----------------------------------------------------------------------------
int EdfToDat(const char* src, const char* edf)
{

	return 0;
}