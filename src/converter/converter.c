#include "_pch.h"
#include "assert.h"
#include "converter.h"
#include "edf_cfg.h"
#include "math.h"
//-----------------------------------------------------------------------------
static char* GetFileExt(const char* filename) {
	char* dot = strrchr(filename, '.');
	if (!dot || dot == filename) return "";
	return dot + 1;
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
int ChangeExt(char* file, const char* ext)
{
	// remove ext
	char* fileExt = GetFileExt(file);
	if (fileExt && 0 != strlen(fileExt))
	{
		*fileExt = '\0';
	}
	// add ext
	size_t fileLen = strlen(file);
	memcpy(file + fileLen, ext, strlen(ext) + 1);
	return 0;
}
//-----------------------------------------------------------------------------
int BinToText(const char* src, const char* dst)
{
	EdfWriter_t br = { 0 };
	EdfWriter_t tw = { 0 };
	if (EdfOpen(&br, src, "rb"))
		LOG_ERR();
	if (EdfOpen(&tw, dst, "wt"))
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
			uint8_t* srcData = br.Block;
			uint8_t* mem = (uint8_t*)&br.Buf;
			size_t memLen = sizeof(br.Buf);
			err = InfoFromBytes(&srcData, (TypeInfo_t*)&br.Buf, &mem, memLen);
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
/// SPSK
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
		.Type = Struct, .Name = "OMEGA_DATA_FILE_V1_1", .Dims = {0, NULL},
		.Childs =
		{
			.Count = 4,
			.Item = (TypeInfo_t[])
			{
				{ UInt32, "время измерения от начала дня (мс)" },
				{ Int32, "давление, 0.001 атм" },
				{ Int32, "температура, 0.001 °С" },
				{ UInt16, "сопротивление изоляции (кОм)" },
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

//-----------------------------------------------------------------------------
/// ECHO

static const double Discrete3000 = 0.00585938;
static const double Discrete6000 = 0.01171875;

static double ExtractDiscrete(uint16_t level)
{
	return (level & 0x4000) ? Discrete6000 : Discrete3000;
}
static double ExtractLevel(uint16_t level)
{
	return level & 0xBFFF;
}
static uint16_t ExtractReflections(uint16_t val)
{
	// ахтунг! параметр передаётся в двоично десятичном виде :-(
	const uint16_t mask = 0x000F;
	uint16_t dec = (uint16_t)((((val >> 4)) & mask) * 10);
	uint16_t sig = (uint16_t)(val & mask);
	int refect = dec + sig;
	return (refect > 99) ? (uint16_t)99 : (uint16_t)refect;
}
static double UnPow(double v, double p)
{
	if (0 > v)
		return pow(fabs(v), p) * (-1.0f);
	return pow(v, p);
}

int EchoToEdf(const char* src, const char* edf, char mode)
{
	FILE* f = NULL;
	int err = fopen_s(&f, src, "rb");
	if (err)
		return err;

	ECHO_FILE_V2_0 dat;
	if (1 != fread(&dat, sizeof(ECHO_FILE_V2_0), 1, f))
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

	double discrete = ExtractDiscrete(dat.Level);
	int maxDepthMult = 1;
	if (Discrete3000 < discrete)
		maxDepthMult = 2;
	float speed = (float)dat.Speed / 10;
	float xDiscrete = speed / 341;

	EdfHeader_t h = MakeHeaderDefault();
	if ((err = EdfWriteHeader(&dw, &h, &writed)))
		return err;

	EdfWriteInfData(&dw, UInt32, "FileType", &dat.FileType);
	EdfWriteStringBytes(&dw, "FileDescription", &dat.FileDescription, FIELD_SIZEOF(ECHO_FILE_V2_0, FileDescription));

	EdfWriteInfData(&dw, UInt16, "ResearchType", &dat.Id.ResearchType);
	EdfWriteInfData(&dw, UInt16, "DeviceType", &dat.Id.DeviceType);
	EdfWriteInfData(&dw, UInt16, "DeviceNum", &dat.Id.DeviceNum);
	EdfWriteInfData(&dw, UInt16, "Shop", &dat.Id.Shop);
	EdfWriteInfData(&dw, UInt16, "Oper", &dat.Id.Oper);
	EdfWriteInfData(&dw, UInt16, "Field", &dat.Id.Field);
	EdfWriteStringBytes(&dw, "Cluster", &dat.Id.Cluster, FIELD_SIZEOF(RESEARCH_ID_V2_0, Cluster));
	EdfWriteStringBytes(&dw, "Well", &dat.Id.Well, FIELD_SIZEOF(RESEARCH_ID_V2_0, Well));

	char cbuf[256] = { 0 };
	snprintf(cbuf, sizeof(cbuf), "%u.%02u.%02uT%02u:%02u:%02u",
		(uint32_t)2000 + dat.Id.Time.Year, dat.Id.Time.Month, dat.Id.Time.Day,
		dat.Id.Time.Hour, dat.Id.Time.Min, dat.Id.Time.Sec);
	EdfWriteInfData(&dw, CString, "DateTime", &((char*) { cbuf }));
	//EdfWriteInfData(&dw, UInt8, "Year", &dat.Id.Time.Year);
	//EdfWriteInfData(&dw, UInt8, "Month", &dat.Id.Time.Month);
	//EdfWriteInfData(&dw, UInt8, "Day", &dat.Id.Time.Day);
	//EdfWriteInfData(&dw, UInt8, "Hour", &dat.Id.Time.Hour);
	//EdfWriteInfData(&dw, UInt8, "Min", &dat.Id.Time.Min);
	//EdfWriteInfData(&dw, UInt8, "Sec", &dat.Id.Time.Sec);

	EdfWriteInfData(&dw, UInt16, "RegType", &dat.Id.RegType);
	EdfWriteInfData(&dw, UInt32, "RegNum", &dat.Id.RegNum);

	EdfWriteInfData(&dw, UInt16, "Reflections", &((uint16_t) { ExtractReflections(dat.Reflections) }));
	EdfWriteInfData(&dw, Double, "Level", &((double) { ExtractLevel(dat.Level) }));
	EdfWriteInfData(&dw, Double, "Discrete", &discrete);//!!

	EdfWriteInfData(&dw, Double, "Pressure", &((double) { dat.Pressure / 10.0f }));
	EdfWriteInfData(&dw, UInt16, "Table", &dat.Table);
	EdfWriteInfData(&dw, Single, "Speed", &speed); //!!
	EdfWriteInfData(&dw, Double, "BufPressure", &((double) { dat.BufPressure / 10.0f }));
	EdfWriteInfData(&dw, Double, "LinePressure", &((double) { dat.LinePressure / 10.0f }));
	EdfWriteInfData(&dw, UInt16, "Current", &dat.Current);
	EdfWriteInfData(&dw, UInt8, "IdleHour", &dat.IdleHour);
	EdfWriteInfData(&dw, UInt8, "IdleMin", &dat.IdleMin);

	EdfWriteInfData(&dw, UInt16, "Mode", &dat.Mode);

	EdfWriteInfData(&dw, Single, "Acc", &((float) { dat.Acc / 10.0f }));
	EdfWriteInfData(&dw, Single, "Temp", &((float) { dat.Temp / 10.0f }));

	//EdfWriteInfData(&dw, Double, "EchoChart", &dat.Data);
	//for (size_t i = 0; i < 3000; i++)
	//	EdfWriteDataBlock(&dw, &dat.Data[i], sizeof(uint8_t));

#pragma pack(push,1)
	struct Point
	{
		float x;
		float y;
	};
#pragma pack(pop)
	TypeInfo_t pointType =
	{
		Struct, "EchoChart", { 0, NULL },
		.Childs =
		{
			.Count = 2,
			.Item = (TypeInfo_t[])
			{
				{ Single, "x" },
				{ Single, "y" },
			}
		}
	};
	EdfWriteInfo(&dw, &pointType, &writed);

	struct Point p = { 0,0 };
	for (size_t i = 0; i < 3000; i++)
	{
		if (dat.Data[i] > 127)
			p.y = (float)UnPow(-1 * (dat.Data[i] - 127), 1.0 / 0.35) / 1000;
		else
			p.y = (float)UnPow(dat.Data[i], 1.0 / 0.35) / 1000;

		p.x = xDiscrete * i * maxDepthMult;

		EdfWriteDataBlock(&dw, &p, sizeof(struct Point));
	}

	EdfClose(&dw);
	return 0;
}

//-----------------------------------------------------------------------------
// helper
TypeInfo_t DoubleValueInf =
{
	Struct, "DoubleValue", { 0, NULL },
	.Childs =
	{
		.Count = 4,
		.Item = (TypeInfo_t[])
		{
			{ CString, "Name" },
			{ Double, "Value" },
			{ CString, "Unit" },
			{ CString, "Description" },
		}
	}
};
#pragma pack(push,1)
typedef struct DoubleValue
{
	char* Name;
	double Value;
	char* Unit;
	char* Description;
} DoubleValue_t;
#pragma pack(pop)
//-----------------------------------------------------------------------------
/// DYN

static int8_t ExtractTravel(uint16_t number) // 6bit integer
{
	int8_t result = ((number & 0xFC00) >> 10);
	return (result > 31) ? (result - 64) : result;
}

int DynToEdf(const char* src, const char* edf, char mode)
{
	assert(8 == strlen("тест"));

	FILE* f = NULL;
	int err = fopen_s(&f, src, "rb");
	if (err)
		return err;

	DYN_FILE_V2_0 dat;
	if (1 != fread(&dat, sizeof(DYN_FILE_V2_0), 1, f))
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

	TypeInfo_t commentType = { .Type = CString, .Name = "Comments" };
	EdfWriteInfo(&dw, &commentType, &writed);
	EdfWriteDataBlock(&dw, &((char*) { "Rod - диаметр штока" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Aperture - номер отверстия" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "MaxWeight - максимальная нагрузка (кг)" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "MinWeight - минимальная нагрузка (кг)" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "TopWeight - вес штанг вверху (кг)" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "BotWeight - вес штанг внизу (кг)" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Travel - ход штока (мм)" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "BeginPos - положение штока перед первым измерением (мм)" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Period - период качаний (мс)" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Cycles - пропущено циклов" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Pressure - затрубное давление (атм)" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "BufPressure - буферное давление (атм)" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "LinePressure - линейное давление (атм)" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "PumpType - тип привода станка-качалки {}" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Acc - напряжение аккумулятора датчика, (В)" }), sizeof(char*));
	EdfWriteDataBlock(&dw, &((char*) { "Temp - температура датчика, (°С)" }), sizeof(char*));

	EdfWriteInfData(&dw, UInt32, "FileType", &dat.FileType);
	EdfWriteStringBytes(&dw, "FileDescription", &dat.FileDescription, FIELD_SIZEOF(DYN_FILE_V2_0, FileDescription));
	// RESEARCH_ID_V2_0
	EdfWriteInfData(&dw, UInt16, "ResearchType", &dat.Id.ResearchType);
	EdfWriteInfData(&dw, UInt16, "DeviceType", &dat.Id.DeviceType);
	EdfWriteInfData(&dw, UInt16, "DeviceNum", &dat.Id.DeviceNum);
	EdfWriteInfData(&dw, UInt16, "Shop", &dat.Id.Shop);
	EdfWriteInfData(&dw, UInt16, "Oper", &dat.Id.Oper);
	EdfWriteInfData(&dw, UInt16, "Field", &dat.Id.Field);
	EdfWriteStringBytes(&dw, "Cluster", &dat.Id.Cluster, FIELD_SIZEOF(RESEARCH_ID_V2_0, Cluster));
	EdfWriteStringBytes(&dw, "Well", &dat.Id.Well, FIELD_SIZEOF(RESEARCH_ID_V2_0, Well));

	char cbuf[256] = { 0 };
	snprintf(cbuf, sizeof(cbuf), "%u.%02u.%02uT%02u:%02u:%02u",
		(uint32_t)2000 + dat.Id.Time.Year, dat.Id.Time.Month, dat.Id.Time.Day,
		dat.Id.Time.Hour, dat.Id.Time.Min, dat.Id.Time.Sec);
	EdfWriteInfData(&dw, CString, "DateTime", &((char*) { cbuf }));
	EdfWriteInfData(&dw, UInt16, "RegType", &dat.Id.RegType);
	EdfWriteInfData(&dw, UInt32, "RegNum", &dat.Id.RegNum);
	// - end RESEARCH_ID_V2_0

	EdfWriteInfData(&dw, Single, "Rod", &((float) { dat.Rod / 10.0f }));
	EdfWriteInfData(&dw, UInt16, "Aperture", &dat.Aperture);
	EdfWriteInfData(&dw, UInt32, "MaxWeight", &((uint32_t) { dat.MaxWeight* dat.LoadStep }));
	EdfWriteInfData(&dw, UInt32, "MinWeight", &((uint32_t) { dat.MinWeight* dat.LoadStep }));
	EdfWriteInfData(&dw, UInt32, "TopWeight", &((uint32_t) { dat.TopWeight* dat.LoadStep }));
	EdfWriteInfData(&dw, UInt32, "BotWeight", &((uint32_t) { dat.BotWeight* dat.LoadStep }));
	EdfWriteInfData(&dw, Double, "Travel", &((double) { dat.Travel* dat.TravelStep / 10.0f }));
	EdfWriteInfData(&dw, Double, "BeginPos", &((double) { dat.BeginPos* dat.TravelStep / 10.0f }));
	EdfWriteInfData(&dw, UInt32, "Period", &((uint32_t) { dat.Period* dat.TimeStep }));
	EdfWriteInfData(&dw, UInt16, "Cycles", &((uint16_t) { dat.Cycles }));
	EdfWriteInfData(&dw, Double, "Pressure", &((double) { dat.Pressure / 10.0f }));
	EdfWriteInfData(&dw, Double, "BufPressure", &((double) { dat.BufPressure / 10.0f }));
	EdfWriteInfData(&dw, Double, "LinePressure", &((double) { dat.LinePressure / 10.0f }));
	EdfWriteInfData(&dw, UInt16, "PumpType", &dat.PumpType);
	EdfWriteInfData(&dw, Single, "Acc", &((float) { dat.Acc / 10.0f }));
	EdfWriteInfData(&dw, Single, "Temp", &((float) { dat.Temp / 10.0f }));
	/*
	{
		EdfWriteInfo(&dw, &DoubleValueInf, &writed);
		EdfWriteDataBlock(&dw, &(DoubleValue_t)
		{
			.Name = "Rod", .Value = dat.Rod / 10.0f, .Unit = "mm",
				.Description = "диаметр штока"
		}, sizeof(DoubleValue_t));

		EdfWriteDataBlock(&dw, &(DoubleValue_t)
		{ "Cycles", dat.Cycles, "", "пропущено циклов" }, sizeof(DoubleValue_t));
		EdfWriteDataBlock(&dw, &(DoubleValue_t)
		{ "Pressure", dat.Pressure / 10.0f, "атм", "затрубное давление" }, sizeof(DoubleValue_t));
		EdfWriteDataBlock(&dw, &(DoubleValue_t)
		{ "BufPressure", dat.BufPressure / 10.0f, "атм", "буферное давление" }, sizeof(DoubleValue_t));
		EdfWriteDataBlock(&dw, &(DoubleValue_t)
		{ "LinePressure", dat.LinePressure / 10.0f, "атм", "линейное давление" }, sizeof(DoubleValue_t));
		EdfWriteDataBlock(&dw, &(DoubleValue_t)
		{ "Acc", dat.Acc / 10.0f, "V", .Description = "напряжение аккумулятора"}, sizeof(DoubleValue_t));
		EdfWriteDataBlock(&dw, &(DoubleValue_t)
		{ "Temp", dat.Temp / 10.0f, "℃", "температура датчика" }, sizeof(DoubleValue_t));
	}
	*/
#pragma pack(push,1)
	struct Point
	{
		float pos;
		float w;
	};
#pragma pack(pop)
	TypeInfo_t pointType =
	{
		Struct, "DynChart", { 0, NULL },
		.Childs =
		{
			.Count = 2,
			.Item = (TypeInfo_t[])
			{
				{ Single, "Position(m)" },
				{ Single, "Weight(t)" }, //вес в тоннах
			}
		}
	};
	EdfWriteInfo(&dw, &pointType, &writed);
	struct Point p = { 0,0 };
	for (size_t i = 0; i < 1000; i++)
	{
		p.w = (float)((dat.Data[i] & 1023) * dat.LoadStep * 1.0E-3);
		p.pos += (float)(ExtractTravel(dat.Data[i]) * dat.TravelStep / 1.E4);
		EdfWriteDataBlock(&dw, &p, sizeof(struct Point));
	}
	EdfClose(&dw);
	return 0;
}