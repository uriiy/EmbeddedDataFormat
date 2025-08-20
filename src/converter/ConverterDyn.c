#include "_pch.h"
#include "converter.h"
#include "SiamFileFormat.h"
#include "assert.h"
#include "edf_cfg.h"
#include "math.h"
//-----------------------------------------------------------------------------
/// DYN
//-----------------------------------------------------------------------------
// helper
/*
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
*/
//-----------------------------------------------------------------------------
static int8_t ExtractTravel(uint16_t number) // 6bit integer
{
	int8_t result = ((number & 0xFC00) >> 10);
	return (result > 31) ? (result - 64) : result;
}
//-----------------------------------------------------------------------------
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
		err = EdfOpen(&dw, edf, "wt");
	else if ('b' == mode)
		err = EdfOpen(&dw, edf, "wb");
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

	EdfWriteInfo(&dw, &ResearchTimeInf, &writed);
	EdfWriteDataBlock(&dw, &(TIME)
	{
		dat.Id.Time.Hour, dat.Id.Time.Min, dat.Id.Time.Sec,
			dat.Id.Time.Day, dat.Id.Time.Month, dat.Id.Time.Year
	}, sizeof(TIME));
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
	fclose(f);
	EdfClose(&dw);
	return 0;
}
//-----------------------------------------------------------------------------
int EdfToDyn(const char* src, const char* dst)
{
	return 0;
}
//-----------------------------------------------------------------------------
