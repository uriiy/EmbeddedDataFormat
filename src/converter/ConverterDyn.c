#include "_pch.h"
#include "KeyValue.h"
#include "Charts.h"
#include "converter.h"
#include "SiamFileFormat.h"
#include "assert.h"
#include "edf_cfg.h"
#include "math.h"
//-----------------------------------------------------------------------------
/// DYN
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

	EdfWriteInfo(&dw, &DateTimeInf, &writed);
	EdfWriteDataBlock(&dw, &(DateTime_t)
	{
		dat.Id.Time.Year + 2000, dat.Id.Time.Month, dat.Id.Time.Day,
			dat.Id.Time.Hour, dat.Id.Time.Min, dat.Id.Time.Sec,
	}, sizeof(DateTime_t));
	EdfWriteInfData(&dw, UInt16, "RegType", &dat.Id.RegType);
	EdfWriteInfData(&dw, UInt32, "RegNum", &dat.Id.RegNum);
	// - end RESEARCH_ID_V2_0
	/*
	EdfWriteInfo(&dw, &CommentsInf, &writed);
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
	*/
	{
		//EdfWriteInfo(&dw, &CommentsInf, &writed);
		//EdfWriteDataBlock(&dw, &((char*) { "Key-Value-Unit-Description list sample" }), sizeof(char*));

		EdfWriteInfo(&dw, &UInt16ValueInf, &writed);
		EdfWriteDataBlock(&dw, &(UInt16Value_t[])
		{
			{ "Aperture", dat.Aperture, "", "номер отверстия 1" },
			{ "Cycles", dat.Cycles, "", "пропущено циклов" },
			{ "PumpType", dat.PumpType, "", "тип привода станка-качалки {}" },
			{ "TravelStep", dat.TravelStep, "", "величина дискреты перемещения, 0.1 мм" },
		}, sizeof(UInt16Value_t[4]));

		EdfWriteInfo(&dw, &UInt32ValueInf, &writed);
		EdfWriteDataBlock(&dw, &(UInt32Value_t[])
		{
			{ "MaxWeight", dat.MaxWeight* dat.LoadStep, "кг", "максимальная нагрузка" },
			{ "MinWeight", dat.MinWeight * dat.LoadStep, "кг", "инимальная нагрузка" },
			{ "TopWeight", dat.TopWeight * dat.LoadStep, "кг", "вес штанг вверху" },
			{ "BotWeight", dat.BotWeight * dat.LoadStep, "кг", "вес штанг внизу" },
			{ "Period", dat.Period * dat.TimeStep, "мм", "ход штока" },
		}, sizeof(UInt32Value_t[3]));

		EdfWriteInfo(&dw, &DoubleValueInf, &writed);
		EdfWriteDataBlock(&dw, &(DoubleValue_t[])
		{
			{ "Rod", dat.Rod / 10.0f, "мм", "диаметр штока" },
			{ "Travel", dat.Travel * dat.TravelStep / 10.0f, "мм", "ход штока123456789" },
			{ "BeginPos", dat.BeginPos * dat.TravelStep / 10.0f, "мм", "положение штока перед первым измерением" },
			{ "Pressure", dat.Pressure / 10.0f, "атм", "затрубное давление" },
			{ "BufPressure", dat.BufPressure / 10.0f, "атм", "буферное давление" },
			{ "LinePressure", dat.LinePressure / 10.0f, "атм", "линейное давление" },
			{ "Acc",  dat.Acc / 10.0f, "V", .Description = "напряжение аккумулятора" },
			{ "Temp", dat.Temp / 10.0f, "℃", "температура датчика" },
		}, sizeof(DoubleValue_t[8]));
	}

	EdfWriteInfo(&dw, &ChartXYDescriptionInf, &writed);
	EdfWriteDataBlock(&dw, &((struct ChartXYDesct)
	{
		"динамограмма", "перемещение, м", "вес в тоннах"
	}), sizeof(struct ChartXYDesct));

	EdfWriteInfo(&dw, &Point2DInf, &writed);
	struct PointXY p = { 0,0 };
	for (size_t i = 0; i < 1000; i++)
	{
		p.x += (float)(ExtractTravel(dat.Data[i]) * dat.TravelStep / 1.E4);
		p.y = (float)((dat.Data[i] & 1023) * dat.LoadStep * 1.0E-3);
		EdfWriteDataBlock(&dw, &p, sizeof(struct PointXY));
	}
	fclose(f);
	EdfClose(&dw);
	return 0;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static void DoOnDoubleItem(DoubleValue_t s, void* state)
{
	DYN_FILE_V2_0* dat = (DYN_FILE_V2_0*)state;

	if (0 == strcmp("Rod", s.Name))
		dat->Rod = s.Value * 10;
	else if (0 == strcmp("Travel", s.Name))
		dat->Travel = s.Value * 10.0f / dat->TravelStep;
	else if (0 == strcmp("BeginPos", s.Name))
		dat->BeginPos = s.Value * 10.0f / dat->TravelStep;
	else if (0 == strcmp("Pressure", s.Name))
		dat->Pressure = round(s.Value * 10);
	else if (0 == strcmp("BufPressure", s.Name))
		dat->BufPressure = round(s.Value * 10);
	else if (0 == strcmp("LinePressure", s.Name))
		dat->LinePressure = round(s.Value * 10);
	else if (0 == strcmp("Acc", s.Name))
		dat->Acc = round(s.Value * 10.f);
	else if (0 == strcmp("Temp", s.Name))
		dat->Temp = round(s.Value * 10.f);
}
//-----------------------------------------------------------------------------
static void DoOnUInt16Item(UInt16Value_t s, void* state)
{
	DYN_FILE_V2_0* dat = (DYN_FILE_V2_0*)state;

	if (0 == strcmp("Aperture", s.Name))
		dat->Aperture = s.Value;
	else if (0 == strcmp("Cycles", s.Name))
		dat->Cycles = s.Value;
	else if (0 == strcmp("PumpType", s.Name))
		dat->PumpType = s.Value;
	else if (0 == strcmp("TravelStep", s.Name))
		dat->TravelStep = s.Value;
}
//-----------------------------------------------------------------------------

int EdfToDyn(const char* edfFile, const char* dynFile)
{
	int err = 0;

	EdfWriter_t br;
	size_t writed = 0;
	if ((err = EdfOpen(&br, edfFile, "rb")))
		return err;

	FILE* f = NULL;
	if ((err = fopen_s(&f, dynFile, "wb")))
		return err;

	// hint
	//int const* ptr; // ptr is a pointer to constant int 
	//int* const ptr;  // ptr is a constant pointer to int

	DYN_FILE_V2_0 dat = { 0 };
	PointXY_t record = { 0 };
	size_t recN = 0;
	const size_t data_len = sizeof(PointXY_t);
	uint8_t* precord = (void*)&record;
	uint8_t* const recordBegin = precord;
	uint8_t* const recordEnd = recordBegin + data_len;

	uint8_t buf[3 * 256 + 8] = { 0 };
	uint8_t* pbuf = buf;
	uint8_t* const pbufEnd = buf + sizeof(buf);

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
			pbuf = buf;
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
			if (0 == _stricmp(br.t->Name, "FileType"))
				dat.FileType = *((uint32_t*)br.Block);
			else if (0 == _stricmp(br.t->Name, "FileDescription"))
			{
				uint8_t len = MIN(MIN(0xFD, *((uint8_t*)br.Block)), FIELD_SIZEOF(DYN_FILE_V2_0, FileDescription));
				memcpy(dat.FileDescription, &br.Block[1], len);
			}
			else if (0 == _stricmp(br.t->Name, "ResearchType"))
				dat.Id.ResearchType = *((uint16_t*)br.Block);
			else if (0 == _stricmp(br.t->Name, "DeviceType"))
				dat.Id.DeviceType = *((uint16_t*)br.Block);
			else if (0 == _stricmp(br.t->Name, "DeviceNum"))
				dat.Id.DeviceNum = *((uint32_t*)br.Block);
			else if (0 == _stricmp(br.t->Name, "Oper"))
				dat.Id.Oper = *((uint16_t*)br.Block);
			else if (0 == _stricmp(br.t->Name, "Shop"))
				dat.Id.Shop = *((uint16_t*)br.Block);
			else if (0 == _stricmp(br.t->Name, "Field"))
				dat.Id.Field = *((uint16_t*)br.Block);
			else if (0 == _stricmp(br.t->Name, "Cluster"))
			{
				uint8_t len = MIN(*((uint8_t*)br.Block), FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Cluster));
				memcpy(dat.Id.Cluster, &br.Block[1], len);
			}
			else if (0 == _stricmp(br.t->Name, "Well"))
			{
				uint8_t len = MIN(*((uint8_t*)br.Block), FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Well));
				memcpy(dat.Id.Well, &br.Block[1], len);
			}
			else if (0 == _stricmp(br.t->Name, DateTimeInf.Name))
			{
				DateTime_t t = *((DateTime_t*)br.Block);
				dat.Id.Time.Year = (uint8_t)(t.Year - 2000);
				dat.Id.Time.Month = t.Month;
				dat.Id.Time.Day = t.Day;
				dat.Id.Time.Hour = t.Hour;
				dat.Id.Time.Min = t.Min;
				dat.Id.Time.Sec = t.Sec;
			}
			else if (0 == _stricmp(br.t->Name, "RegType"))
				dat.Id.RegType = *((uint16_t*)br.Block);
			else if (0 == _stricmp(br.t->Name, "Cycles"))
				dat.Id.RegNum = *((uint32_t*)br.Block);

			else if (0 == _stricmp(br.t->Name, "UInt16Value"))
				DeSerializeUInt16KeyVal(br.Block, br.Block + br.BlockLen,
					&pbuf, buf, pbufEnd, DoOnUInt16Item, &dat);
			else if (0 == _stricmp(br.t->Name, "DoubleValue"))
				DeSerializeDoubleKeyVal(br.Block, br.Block + br.BlockLen,
					&pbuf, buf, pbufEnd, DoOnDoubleItem, &dat);

			else if (0 == _stricmp(br.t->Name, "Chart2D"))
			{
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
						//dat.Data[recN] = TODO
						recN++;
					}
					if (1000 <= recN)
					{
						if (1 != fwrite(&dat, sizeof(DYN_FILE_V2_0), 1, f))
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
