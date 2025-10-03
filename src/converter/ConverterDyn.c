#include "_pch.h"
#include "assert.h"
#include "Charts.h"
#include "converter.h"
#include "edf_cfg.h"
#include "KeyValue.h"
#include "math.h"
#include "SiamFileFormat.h"
#include "stdlib.h"
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

	EdfWriteInfData(&dw, 0, String, "Comment", "ResearchTypeId={ECHOGRAM-5, DYNAMOGRAM-6, SAMT-11}");

	EdfWriteInfData(&dw, FILETYPEID, UInt32, "FileTypeId", &((uint32_t) { dat.FileType }));
	EdfWriteInfData(&dw, LAYOUTVERSION, UInt32, "LayoutVersion", &((uint32_t) { 1 }));

	EdfWriteInfo(&dw, &(const TypeRec_t){ DateTimeType, BEGINDATETIME, "BeginDateTime" }, & writed);
	EdfWriteDataBlock(&dw, &(DateTime_t)
	{
		dat.Id.Time.Year + 2000, dat.Id.Time.Month, dat.Id.Time.Day,
			dat.Id.Time.Hour, dat.Id.Time.Min, dat.Id.Time.Sec,
	}, sizeof(DateTime_t));

	char field[256] = { 0 };
	char cluster[256] = { 0 };
	char well[256] = { 0 };
	char shop[256] = { 0 };
	snprintf(field, sizeof(field) - 1, "%d", dat.Id.Field);
	memcpy(cluster, dat.Id.Cluster, strnlength(dat.Id.Cluster, FIELD_SIZEOF(RESEARCH_ID_V2_0, Cluster)));
	memcpy(well, dat.Id.Well, strnlength(dat.Id.Well, FIELD_SIZEOF(RESEARCH_ID_V2_0, Well)));
	snprintf(shop, sizeof(shop) - 1, "%d", dat.Id.Shop);

	EdfWriteInfo(&dw, &(const TypeRec_t){ PositionType, POSITION, "Position" }, & writed);
	EdfWriteDataBlock(&dw, &(Position_t)
	{.Field = field, .Cluster = cluster, .Well = well, .Shop = shop, },
		sizeof(Position_t));


	EdfWriteInfo(&dw, &(const TypeRec_t){ DeviceInfoType, DEVICEINFO, "DevInfo" }, & writed);
	EdfWriteDataBlock(&dw, &(DeviceInfo_t)
	{.HwId = 0, .HwModel = 0, .SwId = dat.Id.DeviceType, .SwModel = 0, .SwRevision = 0, .HwNumber = dat.Id.DeviceNum},
		sizeof(DeviceInfo_t));

	EdfWriteInfo(&dw, &(const TypeRec_t){ DeviceInfoType, REGINFO, "RegInfo" }, & writed);
	EdfWriteDataBlock(&dw, &(DeviceInfo_t)
	{.HwId = 0, .HwModel = 0, .SwId = dat.Id.RegType, .SwModel = 0, .SwRevision = 0, .HwNumber = dat.Id.RegNum},
		sizeof(DeviceInfo_t));

	EdfWriteInfData(&dw, 0, UInt16, "Oper", &dat.Id.Oper);

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

	EdfWriteInfRecData(&dw, 0, Single, "Rod", &((float) { dat.Rod / 10.0f }));
	EdfWriteInfRecData(&dw, 0, UInt16, "Aperture", &dat.Aperture);
	EdfWriteInfRecData(&dw, 0, UInt32, "MaxWeight", &((uint32_t) { dat.MaxWeight* dat.LoadStep }));
	EdfWriteInfRecData(&dw, 0, UInt32, "MinWeight", &((uint32_t) { dat.MinWeight* dat.LoadStep }));
	EdfWriteInfRecData(&dw, 0, UInt32, "TopWeight", &((uint32_t) { dat.TopWeight* dat.LoadStep }));
	EdfWriteInfRecData(&dw, 0, UInt32, "BotWeight", &((uint32_t) { dat.BotWeight* dat.LoadStep }));
	EdfWriteInfRecData(&dw, 0, Double, "Travel", &((double) { dat.Travel* dat.TravelStep / 10.0f }));
	EdfWriteInfRecData(&dw, 0, Double, "BeginPos", &((double) { dat.BeginPos* dat.TravelStep / 10.0f }));
	EdfWriteInfRecData(&dw, 0, UInt32, "Period", &((uint32_t) { dat.Period* dat.TimeStep }));
	EdfWriteInfRecData(&dw, 0, UInt16, "Cycles", &((uint16_t) { dat.Cycles }));
	EdfWriteInfRecData(&dw, 0, Double, "Pressure", &((double) { dat.Pressure / 10.0f }));
	EdfWriteInfRecData(&dw, 0, Double, "BufPressure", &((double) { dat.BufPressure / 10.0f }));
	EdfWriteInfRecData(&dw, 0, Double, "LinePressure", &((double) { dat.LinePressure / 10.0f }));
	EdfWriteInfRecData(&dw, 0, UInt16, "PumpType", &dat.PumpType);
	EdfWriteInfRecData(&dw, 0, Single, "Acc", &((float) { dat.Acc / 10.0f }));
	EdfWriteInfRecData(&dw, 0, Single, "Temp", &((float) { dat.Temp / 10.0f }));
	*/
	{
		//EdfWriteInfo(&dw, &CommentsInf, &writed);
		//EdfWriteDataBlock(&dw, &((char*) { "Key-Value-Unit-Description list sample" }), sizeof(char*));

		EdfWriteInfo(&dw, &(const TypeRec_t){ UInt16ValueInf, 0, "UInt16Value"}, & writed);
		EdfWriteDataBlock(&dw, &(UInt16Value_t[])
		{
			{ "Aperture", dat.Aperture, "", "номер отверстия 1" },
			{ "Cycles", dat.Cycles, "", "пропущено циклов" },
			{ "PumpType", dat.PumpType, "", "тип привода станка-качалки {}" },
			{ "TravelStep", dat.TravelStep, "0.1мм/1", "величина дискреты перемещения" },
			{ "LoadStep", dat.LoadStep, "кг/1", "величина дискреты нагрузки" },
			{ "TimeStep", dat.TimeStep, "мс/1", "величина дискреты времени" },
		}, sizeof(UInt16Value_t[6]));

		EdfWriteInfo(&dw, &(const TypeRec_t){ UInt32ValueInf, 0, "UInt32Value"}, & writed);
		EdfWriteDataBlock(&dw, &(UInt32Value_t[])
		{
			{ "MaxWeight", dat.MaxWeight* dat.LoadStep, "кг", "максимальная нагрузка" },
			{ "MinWeight", dat.MinWeight * dat.LoadStep, "кг", "инимальная нагрузка" },
			{ "TopWeight", dat.TopWeight * dat.LoadStep, "кг", "вес штанг вверху" },
			{ "BotWeight", dat.BotWeight * dat.LoadStep, "кг", "вес штанг внизу" },
			{ "Period", dat.Period * dat.TimeStep, "мм", "ход штока" },
		}, sizeof(UInt32Value_t[5]));

		EdfWriteInfo(&dw, &(const TypeRec_t){ DoubleValueInf, 0, "DoubleValue"}, & writed);
		EdfWriteDataBlock(&dw, &(DoubleValue_t[])
		{
			{ "Rod", dat.Rod / 10.0f, "мм", "диаметр штока" },
			{ "Travel", dat.Travel * dat.TravelStep / 10.0f, "мм", "ход штока123456789" },
			{ "BeginPos", dat.BeginPos * dat.TravelStep / 10.0f, "мм", "положение штока перед первым измерением" },
			{ "Pressure", dat.Pressure / 10.0f, "атм", "затрубное давление" },
			{ "BufPressure", dat.BufPressure / 10.0f, "атм", "буферное давление" },
			{ "LinePressure", dat.LinePressure / 10.0f, "атм", "линейное давление" },
			{ "Acc",  dat.Acc / 10.0f, "V", .Description = "напряжение аккумулятора" },
			{ "Temp", dat.Temp / 10.0f, "°С", "температура датчика" },
		}, sizeof(DoubleValue_t[8]));
	}

	EdfWriteInfo(&dw, &(const TypeRec_t){ ChartNInf, 0, "DynamogrammInfo" }, & writed);
	EdfWriteDataBlock(&dw, &((ChartN_t[])
	{
		{ "Position", "m", "", "перемещение" },
		{ "Weight", "T", "", "вес" },
	}), sizeof(ChartN_t) * 2);

	EdfWriteInfo(&dw, &(const TypeRec_t){ Point2DInf, 0, "DynChart"}, & writed);
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
static void DoOnDoubleItem(DoubleValue_t* s, void* state)
{
	DYN_FILE_V2_0* dat = (DYN_FILE_V2_0*)state;
	if (0 == strcmp("Rod", s->Name))
		dat->Rod = (uint16_t)round(s->Value * 10);
	else if (0 == strcmp("Travel", s->Name))
		dat->Travel = (uint16_t)round(s->Value * 10.0f / dat->TravelStep);
	else if (0 == strcmp("BeginPos", s->Name))
		dat->BeginPos = (uint16_t)round(s->Value * 10.0f / dat->TravelStep);
	else if (0 == strcmp("Pressure", s->Name))
		dat->Pressure = (int16_t)round(s->Value * 10);
	else if (0 == strcmp("BufPressure", s->Name))
		dat->BufPressure = (int16_t)round(s->Value * 10);
	else if (0 == strcmp("LinePressure", s->Name))
		dat->LinePressure = (int16_t)round(s->Value * 10);
	else if (0 == strcmp("Acc", s->Name))
		dat->Acc = (uint16_t)round(s->Value * 10.f);
	else if (0 == strcmp("Temp", s->Name))
		dat->Temp = (int16_t)round(s->Value * 10.f);
}
//-----------------------------------------------------------------------------
static void DoOnUInt16Item(UInt16Value_t* s, void* state)
{
	DYN_FILE_V2_0* dat = (DYN_FILE_V2_0*)state;
	if (0 == strcmp("Aperture", s->Name))
		dat->Aperture = s->Value;
	else if (0 == strcmp("Cycles", s->Name))
		dat->Cycles = s->Value;
	else if (0 == strcmp("PumpType", s->Name))
		dat->PumpType = s->Value;
	else if (0 == strcmp("TravelStep", s->Name))
		dat->TravelStep = s->Value;
	else if (0 == strcmp("LoadStep", s->Name))
		dat->LoadStep = s->Value;
	else if (0 == strcmp("TimeStep", s->Name))
		dat->TimeStep = s->Value;
}
//-----------------------------------------------------------------------------
static void DoOnUInt32Item(UInt32Value_t* s, void* state)
{
	DYN_FILE_V2_0* dat = (DYN_FILE_V2_0*)state;
	if (0 == strcmp("MaxWeight", s->Name))
		dat->MaxWeight = (uint16_t)(s->Value / dat->LoadStep);
	else if (0 == strcmp("MinWeight", s->Name))
		dat->MinWeight = (uint16_t)(s->Value / dat->LoadStep);
	else if (0 == strcmp("TopWeight", s->Name))
		dat->TopWeight = (uint16_t)(s->Value / dat->LoadStep);
	else if (0 == strcmp("BotWeight", s->Name))
		dat->BotWeight = (uint16_t)(s->Value / dat->LoadStep);
	else if (0 == strcmp("Period", s->Name))
		dat->Period = (uint16_t)(s->Value / dat->TimeStep);
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

	DYN_FILE_V2_0 dat = { 0 };
	dat.FileType = 6;
	dat.Id.ResearchType = 1;
	memcpy(dat.FileDescription, FileDescDyn, sizeof(FileDescDyn));
	size_t recN = 0;
	PointXY_t record = { 0 };

	int skip = 0;
	uint8_t bDst[3 * 256 + 8] = { 0 };
	MemStream_t msDst = { 0 };
	if ((err = MemStreamOpen(&msDst, bDst, sizeof(bDst), 0, "w")))
		return err;

	while (!(err = EdfReadBlock(&br)))
	{
		MemStream_t src = { 0 };
		if ((err = MemStreamInOpen(&src, br.Block, br.DatLen)))
			return err;

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
			skip = 0;
			msDst.WPos = 0;
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
				case FILETYPEID: dat.FileType = *((uint32_t*)br.Block); break;//case FILETYPE:
				case FILEDESCRIPTION:
					memcpy(dat.FileDescription, &br.Block[1],
						MIN(*((uint8_t*)br.Block), FIELD_SIZEOF(SPSK_FILE_V1_1, FileDescription)));
					break;//case FILEDESCRIPTION:
				case BEGINDATETIME:
				{
					DateTime_t* t = (DateTime_t*)br.Block;
					dat.Id.Time.Year = (uint8_t)(t->Year - 2000);
					dat.Id.Time.Month = t->Month;
					dat.Id.Time.Day = t->Day;
					dat.Id.Time.Hour = t->Hour;
					dat.Id.Time.Min = t->Min;
					dat.Id.Time.Sec = t->Sec;
				}
				break;
				case POSITION:
				{
					Position_t* p = NULL;
					if ((err = EdfReadBin(&PositionType, &src, &msDst, &p, &skip)))
						return err;

					unsigned long ulVal = strtoul(p->Field, NULL, 10);
					if (ERANGE == errno)
					{
						errno = 0;
						ulVal = 0;
					}
					dat.Id.Field = (uint16_t)ulVal;

					uint8_t len = MIN(*((uint8_t*)p->Cluster), FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Cluster));
					memcpy(dat.Id.Cluster, p->Cluster, len);

					len = MIN(*((uint8_t*)p->Well), FIELD_SIZEOF(FILES_RESEARCH_ID_V1_0, Well));
					memcpy(dat.Id.Well, p->Well, len);

					ulVal = strtoul(p->Shop, NULL, 10);
					if (ERANGE == errno)
					{
						errno = 0;
						ulVal = 0;
					}
					dat.Id.Shop = (uint16_t)ulVal;
				}
				break;
				case DEVICEINFO:
				{
					DeviceInfo_t* dvc = NULL;
					if ((err = EdfReadBin(&DeviceInfoType, &src, &msDst, &dvc, &skip)))
						return err;
					dat.Id.DeviceType = (uint16_t)dvc->SwId;
					dat.Id.DeviceNum = (uint32_t)dvc->HwNumber;
				}
				break;
				case REGINFO:
				{
					DeviceInfo_t* dvc = NULL;
					if ((err = EdfReadBin(&DeviceInfoType, &src, &msDst, &dvc, &skip)))
						return err;
					dat.Id.RegType = (uint16_t)dvc->SwId;
					dat.Id.RegNum = (uint32_t)dvc->HwNumber;
				}
				break;

				}//switch

			}
			else if (IsVarName(br.t, "Oper"))
				dat.Id.Oper = *((uint16_t*)br.Block);
			else if (IsVarName(br.t, "UInt16Value"))
			{
				UnpackUInt16KeyVal(&src, &msDst, &skip, DoOnUInt16Item, &dat);
			}
			else if (IsVarName(br.t, "UInt32Value"))
			{
				UnpackUInt32KeyVal(&src, &msDst, &skip, DoOnUInt32Item, &dat);
			}
			else if (IsVarName(br.t, "DoubleValue"))
			{
				UnpackDoubleKeyVal(&src, &msDst, &skip, DoOnDoubleItem, &dat);
			}

			else if (IsVarName(br.t, "DynChart"))
			{
				PointXY_t* s = NULL;
				while (!(err = EdfReadBin(&Point2DInf, &src, &msDst, &s, &skip))
					&& recN <= FIELD_SIZEOF(DYN_FILE_V2_0, Data))
				{
					double posDif = recN ? s->x - record.x : s->x;
					//double posDif = s->x;
					uint16_t tr = (((uint16_t)round(posDif * 1.0E4 / dat.TravelStep) & 0x003f)) << 10;
					uint16_t w = ((uint16_t)(round(s->y * 1.0E3 / dat.LoadStep)) & 0x003f);
					dat.Data[recN++] = tr | w;
					record = *s;
					s = NULL;
					skip = 0;
					msDst.WPos = 0;
				}
				skip = -skip;
				err = 0;
			}//else
		}//case btVarData:
		break;
		}//switch (br.BlkType)
		if (0 != err)
		{
			LOG_ERR();
			break;
		}
	}//while (!(err = EdfReadBlock(&br)))

	dat.crc = MbCrc16(&dat, sizeof(DYN_FILE_V2_0) - 2);

	if (1 != fwrite(&dat, sizeof(DYN_FILE_V2_0), 1, f))
		return -1;

	fclose(f);
	EdfClose(&br);
	return 0;
}


//-----------------------------------------------------------------------------
